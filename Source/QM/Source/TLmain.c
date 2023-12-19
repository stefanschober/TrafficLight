//$file${./Source::TLmain.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: TrafficLight.qm
// File:  ${./Source::TLmain.c}
//
// This code has been generated by QM 6.0.0 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${./Source::TLmain.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

/* "fudge factor" for Windows, see NOTE1 */
#define WIN_FUDGE    1
#if defined(QWIN_GUI)
#   undef WIN_FUDGE
#   define WIN_FUDGE 10

#   undef main
#   define main main_gui
#endif

enum { WIN_FUDGE_FACTOR = WIN_FUDGE };

Q_DEFINE_THIS_FILE

//$declare${AOs::Ticker_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Ticker_ctor} ........................................................
static void Ticker_ctor(void);
//$enddecl${AOs::Ticker_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$declare${AOs::callConstructors} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::callConstructors} ...................................................
static void callConstructors(void);
//$enddecl${AOs::callConstructors} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$declare${AOs::startAllActiveObjects} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::startAllActiveObjects} ..............................................
static void startAllActiveObjects(void);
//$enddecl${AOs::startAllActiveObjects} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* AO event queues & event pool data storage */
static QEvt const *trafficQueueSto[N_TL][5 * WIN_FUDGE_FACTOR];
static QEvt const *pedestrianQueueSto[5 * WIN_FUDGE_FACTOR];
static QEvt const *blinkerQueueSto[5 * WIN_FUDGE_FACTOR];
static QSubscrList subscrSto[MAX_PUBLISH_SIG];
static QF_MPOOL_EL(QEvt) smlPoolSto[10 * WIN_FUDGE_FACTOR];

/*..........................................................................*/

#if defined(QWIN_GUI) && defined(__WIN32__)
static const char *argv[] = {
    "trafficlight",
    NULL
};
static const size_t argc = ((sizeof(argv) / sizeof(argv[0])) - 1)

int     main_gui(void)
#else
int main(int argc, char *argv[])
#endif
{
    BSP_HW_init();
    QF_init();    /* initialize the framework and the underlying RT kernel */
    BSP_init(argc, argv); /* initialize the Board Support Package */

    /* object dictionaries... */
    QS_OBJ_DICTIONARY(smlPoolSto);
    QS_OBJ_DICTIONARY(pedestrianQueueSto);
    QS_OBJ_DICTIONARY(trafficQueueSto[0]);
    QS_OBJ_DICTIONARY(trafficQueueSto[1]);
    QS_OBJ_DICTIONARY(blinkerQueueSto);

    QS_SIG_DICTIONARY(TIME_TICK_SIG, (void *)0);
    QS_SIG_DICTIONARY(GLOBAL_START_SIG, (void *)0);
    QS_SIG_DICTIONARY(STARTNEWCYCLE_SIG, (void *)0);
    QS_SIG_DICTIONARY(PEDREQUEST_SIG, (void *)0);
    QS_SIG_DICTIONARY(TL_IS_RED_SIG, (void *)0);
    QS_SIG_DICTIONARY(PL_IS_RED_SIG, (void *)0);
    QS_SIG_DICTIONARY(BUTTON_SIG, (void *)0);
    QS_SIG_DICTIONARY(START_BLINK_SIG, (void *)0);
    QS_SIG_DICTIONARY(STOP_BLINK_SIG, (void *)0);
    QS_SIG_DICTIONARY(OFF_BLINK_SIG, (void *)0);
    QS_SIG_DICTIONARY(EMERGENCY_SIG, (void *)0);
    QS_SIG_DICTIONARY(EM_RELEASE_SIG, (void *)0);
    QS_SIG_DICTIONARY(TIMEOUT_SIG, (void *)0);

    /* initialize publish-subscribe... */
    QF_psInit(subscrSto, Q_DIM(subscrSto));

    /* initialize event pools... */
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    /* call all AO constructors */
    callConstructors();

    /* start all AOs */
    startAllActiveObjects();

    return QF_run(); /* run the QF application */
}

/* the ticker */
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::l_ticker0} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::l_ticker0} ..........................................................
static QTicker l_ticker0;
//$enddef${AOs::l_ticker0} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${AOs::the_Ticker0} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::the_Ticker0} ........................................................
QActive * const the_Ticker0  = (QActive * const)&l_ticker0;
//$enddef${AOs::the_Ticker0} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::Ticker_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::Ticker_ctor} ........................................................
static void Ticker_ctor(void) {
    QS_OBJ_DICTIONARY(the_Ticker0);

    QTicker_ctor(&l_ticker0, 0U); /* ticker AO for tick rate 0 */
}
//$enddef${AOs::Ticker_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* call all constructors */
//$define${AOs::callConstructors} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::callConstructors} ...................................................
static void callConstructors(void) {
    Ticker_ctor();
    TLtraffic_ctor(); /* instantiate all Philosopher active objects */
    TLpedestrian_ctor(); /* instantiate the Table active object */
    TLblinker_ctor();
}
//$enddef${AOs::callConstructors} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* start all AOs */
//$define${AOs::startAllActiveObjects} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::startAllActiveObjects} ..............................................
static void startAllActiveObjects(void) {
    uint8_t n;
    uint_fast8_t aoPrio = 1u;

    QS_OBJ_DICTIONARY(AO_TLblinker);
    QACTIVE_START(the_Ticker0, aoPrio++, 0, 0, 0, 0, 0);

    /* start the active objects... */
    // QS_OBJ_DICTIONARY(AO_TLtraffic[0]);
    // QS_OBJ_DICTIONARY(AO_TLtraffic[1]);
    // QS_OBJ_DICTIONARY(AO_TLtraffic[2]);
    for (n = 0U; n < N_TL; ++n) {
        QS_OBJ_DICTIONARY(AO_TLtraffic[n]);
        QACTIVE_START(AO_TLtraffic[n],           /* AO to start */
                      aoPrio++, /* QP priority of the AO */
                      trafficQueueSto[n],      /* event queue storage */
                      Q_DIM(trafficQueueSto[n]), /* queue length [events] */
                      (void *)0,             /* stack storage (not used) */
                      0U,                    /* size of the stack [bytes] */
                     (QEvt *)0);             /* initialization event */
    }

    QS_OBJ_DICTIONARY(AO_TLpedestrian);
    QACTIVE_START(AO_TLpedestrian,                  /* AO to start */
                  aoPrio++, /* QP priority of the AO */
                  pedestrianQueueSto,             /* event queue storage */
                  Q_DIM(pedestrianQueueSto),      /* queue length [events] */
                  (void *)0,                 /* stack storage (not used) */
                  0U,                        /* size of the stack [bytes] */
                  (QEvt *)0);                /* initialization event */

    QS_OBJ_DICTIONARY(AO_TLblinker);
    QACTIVE_START(AO_TLblinker,                  /* AO to start */
                  aoPrio++, /* QP priority of the AO */
                  blinkerQueueSto,             /* event queue storage */
                  Q_DIM(blinkerQueueSto),      /* queue length [events] */
                  (void *)0,                 /* stack storage (not used) */
                  0U,                        /* size of the stack [bytes] */
                  (QEvt *)0);                /* initialization event */
}
//$enddef${AOs::startAllActiveObjects} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/*****************************************************************************
* NOTE1:
* Windows is not a deterministic real-time system, which means that the
* system can occasionally and unexpectedly "choke and freeze" for a number
* of seconds. The designers of Windows have dealt with these sort of issues
* by massively oversizing the resources available to the applications. For
* example, the default Windows GUI message queues size is 10,000 entries,
* which can dynamically grow to an even larger number. Also the stacks of
* Win32 threads can dynamically grow to several megabytes.
*
* In contrast, the event queues, event pools, and stack size inside the
* real-time embedded (RTE) systems can be (and must be) much smaller,
* because you typically can put an upper bound on the real-time behavior
* and the resulting delays.
*
* To be able to run the unmodified applications designed originally for
* RTE systems on Windows, and to reduce the odds of resource shortages in
* this case, the generous WIN_FUDGE_FACTOR is used to oversize the
* event queues and event pools.
*/
