 /*****************************************************************************
* Product: DPP example, Windows (console)
* Last updated for version 5.9.7
* Last updated on  2017-08-25
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
*****************************************************************************/
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef Q_SPY
    #include "qspy.h"
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>  /* Win32 API for multithreading */
    #include <winsock2.h> /* for Windows network facilities */
	#include <time.h>
#endif

Q_DEFINE_THIS_FILE

/* local variables ---------------------------------------------------------*/
#ifdef Q_SPY
    enum {
        TL_STAT = QS_USER,
        COMMAND_STAT
    };
    // static SOCKET l_sock = INVALID_SOCKET;
    static uint8_t const l_clock_tick = 0x00U;
    static uint8_t const l_button     = 0x01U;
    static uint8_t l_running = 0x00U;
#endif


// static uint8_t keyPressed = 0;

/*..........................................................................*/
void QF_onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC); /* set the desired tick rate */
}
/*..........................................................................*/
void QF_onCleanup(void) {
    printf("\nBye! Bye!\n");
}
/*..........................................................................*/
void QF_onClockTick(void) {
    // static QEvt const tickEvt = { TIME_TICK_SIG, 0U, 0U };

    // QF_TICK_X(0U, &l_clock_tick); /* perform the QF clock tick processing */
    QACTIVE_POST(the_Ticker0, 0, &l_clock_tick); /* post to Ticker0 */
    // QF_PUBLISH(&tickEvt, &l_clock_tick); /* publish to all subscribers */

    if (_kbhit()) { /* any key pressed? */
        int ch = _getch();

        switch (ch)
        {
            case '\33':        // ESC key pressed
            case 'q':		   // q pressec
            case 'Q':		   // Q pressed
                BSP_terminate(0);
                break;
            case 'p':
            case 'P':
                // keyPressed = 1;
				printf("Pedestrian button pressed...\n");
				BSP_publishBtnEvt(); /* publish to all subscribers */
                break;
            case 'e':
            case 'E':
                // keyPressed = 1;
				printf("EMERGENCY button pressed...\n");
				BSP_publishEmergencyEvt(); /* publish to all subscribers */
                break;
            default:
                break;
        }
    }
}
/*..........................................................................*/
void Q_onAssert(char const * const module, int_t loc) {
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */
    fprintf(stderr, "Assertion failed in %s, line %d", module, loc);
    exit(-1);
}
/*..........................................................................*/
void BSP_init(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("Traffic Light example"
           "\nQP %s\n"
           "Press 'p' for pedestrian signal\n"
           "Press ESC to quit...\n",
           QP_versionStr);

    Q_ALLEGE(QS_INIT((argc > 1) ? argv[1] : ""));
    QS_OBJ_DICTIONARY(&l_clock_tick); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_button); /* must be called *after* QF_init() */
    QS_USR_DICTIONARY(TL_STAT);
}
/*..........................................................................*/
void BSP_terminate(int16_t result) {
    (void)result;
#ifdef Q_SPY
    l_running = (uint8_t)0; /* stop the QS output thread */
#endif
    QF_stop(); /* stop the main "ticker thread" */
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, eTLlight_t light)
{
    static char * const strIdentity[MaxIdentity] = {
        "Traffic Light A", "Traffic Light B", "Pedestrian Light"
    };
    static char * const strColor[NO_LIGHT] = {
        "RED", "YELLOW", "GREEN"
    };
    printf("%s is %s\n", strIdentity[id], strColor[light]);

#if 0    
    QS_BEGIN(T, AO_Philo[n]) /* application-specific record begin */
        QS_U8(1, n);  /* Philosopher number */
        QS_STR(stat); /* Philosopher status */
    QS_END()
#endif
}
/*..........................................................................*/
void BSP_setPedLed(uint16_t status)
{
    (void)status;
}
/*..........................................................................*/
// called when button PEDESTRIAN is clicked
void BSP_publishBtnEvt(void)
{
	static QEvt const buttonEvt = { BUTTON_SIG, 0U, 0U };

    QF_PUBLISH(&buttonEvt, &l_button); /* publish to all subscribers */
}

void BSP_publishEmergencyEvt(void)
{
	static QEvt buttonEvt = { EM_RELEASE_SIG, 0U, 0U };

    buttonEvt.sig = ((buttonEvt.sig == EMERGENCY_SIG) ? EM_RELEASE_SIG : EMERGENCY_SIG);
    QF_PUBLISH(&buttonEvt, &l_button); /* publish to all subscribers */
}

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY /* define QS callbacks */
/*..........................................................................*/
static DWORD WINAPI idleThread(LPVOID par) {/* signature for CreateThread() */
    (void)par;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
    l_running = (uint8_t)1;
    while (l_running) {
        uint16_t nBytes = 256;
        uint8_t const *block;
        QF_CRIT_ENTRY(dummy);
        block = QS_getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);
        if (block != (uint8_t *)0) {
            QSPY_parse(block, nBytes);
        }
        Sleep(10U);  /* wait for a clock tick */
    }
    return 0; /* return success */
}
/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[2*1024];  /* 4K buffer for Quantum Spy */
    QS_initBuf(qsBuf, sizeof(qsBuf));

    /* here 'arg' is ignored, but this command-line parameter can be used
    * to setup the QSP_config(), to set up the QS filters, or for any
    * other purpose.
    */
    (void)arg;

    QSPY_config(QP_VERSION,         // version
                QS_OBJ_PTR_SIZE,    // objPtrSize
                QS_FUN_PTR_SIZE,    // funPtrSize
                QS_TIME_SIZE,       // tstampSize
                Q_SIGNAL_SIZE,      // sigSize,
                QF_EVENT_SIZ_SIZE,  // evtSize
                QF_EQUEUE_CTR_SIZE, // queueCtrSize
                QF_MPOOL_CTR_SIZE,  // poolCtrSize
                QF_MPOOL_SIZ_SIZE,  // poolBlkSize
                QF_TIMEEVT_CTR_SIZE,// tevtCtrSize
                (void *)0,          // matFile,
                (void *)0,          // mscFile
                (QSPY_CustParseFun)0); // no customized parser function

    /* setup the QS filters... */
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    QS_FILTER_ON(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_ON(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_ON(QS_QF_PUBLISH);

    QS_FILTER_ON(TL_STAT);

    return CreateThread(NULL, 1024, &idleThread, (void *)0, 0, NULL)
             != (HANDLE)0; /* return the status of creating the idle thread */
}
/*..........................................................................*/
void QS_onCleanup(void) {
    l_running = (uint8_t)0;
    QSPY_stop();
}
/*..........................................................................*/
void QS_onFlush(void) {
    for (;;) {
        uint16_t nBytes = 1024;
        uint8_t const *block;

        QF_CRIT_ENTRY(dummy);
        block = QS_getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);

        if (block != (uint8_t const *)0) {
            QSPY_parse(block, nBytes);
            nBytes = 1024;
        }
        else {
            break;
        }
    }
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) {
    return (QSTimeCtr)clock();
}
/*..........................................................................*/
void QSPY_onPrintLn(void) {
    fputs(QSPY_line, stdout);
    fputc('\n', stdout);
}
/*..........................................................................*/
/*! callback function to reset the target (to be implemented in the BSP) */
void QS_onReset(void) {
    //TBD
}
/*..........................................................................*/
/*! callback function to execute a user command (to be implemented in BSP) */
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
    QS_BEGIN(COMMAND_STAT, (void *)1) /* application-specific record begin */
        QS_U8(2, cmdId);
        QS_U32(8, param1);
        QS_U32(8, param2);
        QS_U32(8, param3);
    QS_END()

    if (cmdId == 10U) {
        Q_onAssert("command", 10);
    }
}
#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/
