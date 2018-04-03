/**
* @file
* @brief QF/C port to POSIX/P-threads, GNU-C compiler
* @ingroup ports
* @cond
******************************************************************************
* Last Updated for Version: 5.9.7
* Date of the Last Update:  2017-08-25
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* https://state-machine.com
* mailto:info@state-machine.com
******************************************************************************
* @endcond
*/
#define QP_IMPL           /* this is QP implementation */
#include "qf_port.h"      /* QF port */
#include "qf_pkg.h"
#include "qassert.h"
#ifdef Q_SPY              /* QS software tracing enabled? */
    #include "qs_port.h"  /* include QS port */
#else
    #include "qs_dummy.h" /* disable the QS software tracing */
#endif /* Q_SPY */

#include <limits.h>       /* for PTHREAD_STACK_MIN */
#include <glib.h>
/* #include <sys/mman.h>      for mlockall() (uncomment when supported) */

#if !defined PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN	(0x10000u)
#endif /* PTHREAD_STACK_MIN */

Q_DEFINE_THIS_MODULE("qf_port")

/* Global objects ----------------------------------------------------------*/
GMutex QF_pThreadMutex_;

/* Local objects -----------------------------------------------------------*/
static GMutex l_startupMutex;
static bool l_isRunning;
static struct timespec l_tick;
enum { NANOSLEEP_NSEC_PER_SEC = 1000000000 }; /* see NOTE05 */

/*..........................................................................*/
void QF_init(void) {
    extern uint_fast8_t QF_maxPool_;
    extern QTimeEvt QF_timeEvtHead_[QF_MAX_TICK_RATE];

    /* lock memory so we're never swapped out to disk */
    /*mlockall(MCL_CURRENT | MCL_FUTURE);  uncomment when supported */

    /* lock the startup mutex to block any active objects started before
    * calling QF_run()
    */
    g_mutex_lock(&l_startupMutex);

    /* clear the internal QF variables, so that the framework can (re)start
    * correctly even if the startup code is not called to clear the
    * uninitialized data (as is required by the C Standard).
    */
    QF_maxPool_ = (uint_fast8_t)0;
    QF_bzero(&QF_timeEvtHead_[0], (uint_fast16_t)sizeof(QF_timeEvtHead_));
    QF_bzero(&QF_active_[0],      (uint_fast16_t)sizeof(QF_active_));

    l_tick.tv_sec = 0;
    l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC/100L; /* default clock tick */
}
/*..........................................................................*/
int_t QF_run(void) {

    QF_onStartup();  /* invoke startup callback */

    /* unlock the startup mutex to unblock any active objects started before
    * calling QF_run()
    */
    g_mutex_unlock(&l_startupMutex);

    l_isRunning = true;
    while (l_isRunning) { /* the clock tick loop... */
        QF_onClockTick(); /* clock tick callback (must call QF_TICK_X()) */

        nanosleep(&l_tick, NULL); /* sleep for the number of ticks, NOTE05 */
    }
    QF_onCleanup(); /* invoke cleanup callback */

    return (int_t)0; /* return success */
}
/*..........................................................................*/
void QF_setTickRate(uint32_t ticksPerSec) {
    l_tick.tv_nsec = NANOSLEEP_NSEC_PER_SEC / ticksPerSec;
}
/*..........................................................................*/
void QF_stop(void) {
    l_isRunning = false; /* stop the loop in QF_run() */
}
/*..........................................................................*/
static void *thread_routine(void *arg) { /* the expected POSIX signature */
    QActive *act = (QActive *)arg;

    /* block this thread until the startup mutex is unlocked from QF_run() */
    g_mutex_lock(&l_startupMutex);
    g_mutex_unlock(&l_startupMutex);

    /* loop until m_thread is cleared in QActive_stop() */
    do {
        QEvt const *e = QActive_get_(act); /* wait for the event */
        QHSM_DISPATCH(&act->super, e);     /* dispatch to the HSM */
        QF_gc(e);    /* check if the event is garbage, and collect it if so */
    } while (act->thread != (uint8_t)0);
    QF_remove_(act); /* remove this object from the framework */
    g_cond_clear(&act->osObject); /* cleanup the condition variable */
    return (void *)0; /* return success */
}
/*..........................................................................*/
void QActive_start_(QActive * const me, uint_fast8_t prio,
                    QEvt const *qSto[], uint_fast16_t qLen,
                    void *stkSto, uint_fast16_t stkSize,
                    QEvt const *ie)
{
    (void)stkSize;

    /* p-threads allocate stack internally */
    Q_REQUIRE_ID(600, stkSto == (void *)0);

    QEQueue_init(&me->eQueue, qSto, qLen);
    g_cond_init(&me->osObject);

    me->prio = (uint8_t)prio;
    QF_add_(me); /* make QF aware of this active object */

    QHSM_INIT(&me->super, ie); /* take the top-most initial tran. */
    QS_FLUSH(); /* flush the QS trace buffer to the host */

    g_thread_new(NULL, &thread_routine, me);
    me->thread = (uint8_t)1;
}
/*..........................................................................*/
void QActive_stop(QActive * const me) {
    me->thread = (uint8_t)0; /* stop the QActive thread loop */
}

/*****************************************************************************
* NOTE01:
* In Linux, the scheduler policy closest to real-time is the SCHED_FIFO
* policy, available only with superuser privileges. QF_run() attempts to set
* this policy as well as to maximize its priority, so that the ticking
* occurrs in the most timely manner (as close to an interrupt as possible).
* However, setting the SCHED_FIFO policy might fail, most probably due to
* insufficient privileges.
*
* NOTE02:
* On some Linux systems nanosleep() might actually not deliver the finest
* time granularity. For example, on some Linux implementations, nanosleep()
* could not block for shorter intervals than 20ms, while the underlying
* clock tick period was only 10ms. Sometimes, the select() system call can
* provide a finer granularity.
*
* NOTE03:
* Any blocking system call, such as nanosleep() or select() system call can
* be interrupted by a signal, such as ^C from the keyboard. In this case this
* QF port breaks out of the event-loop and returns to main() that exits and
* terminates all spawned p-threads.
*
* NOTE04:
* According to the man pages (for pthread_attr_setschedpolicy) the only value
* supported in the Linux p-threads implementation is PTHREAD_SCOPE_SYSTEM,
* meaning that the threads contend for CPU time with all processes running on
* the machine. In particular, thread priorities are interpreted relative to
* the priorities of all other processes on the machine.
*
* This is good, because it seems that if we set the priorities high enough,
* no other process (or thread running within) can gain control over the CPU.
*
* However, QF limits the number of priority levels to QF_MAX_ACTIVE.
* Assuming that a QF application will be real-time, this port reserves the
* three highest Linux priorities for the ISR-like threads (e.g., the ticker,
* I/O), and the rest highest-priorities for the active objects.
*
* NOTE05:
* In some (older) Linux kernels, the POSIX nanosleep() system call might
* deliver only 2*actual-system-tick granularity. To compensate for this,
* you would need to reduce (by 2) the constant NANOSLEEP_NSEC_PER_SEC.
*/

