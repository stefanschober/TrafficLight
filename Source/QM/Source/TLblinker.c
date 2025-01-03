//$file${./Source::TLblinker.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: TrafficLight.qm
// File:  ${./Source::TLblinker.c}
//
// This code has been generated by QM 6.2.3 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
//                 ____________________________________
//                /                                   /
//               /    GGGGGGG    PPPPPPPP   LL       /
//              /   GG     GG   PP     PP  LL       /
//             /   GG          PP     PP  LL       /
//            /   GG   GGGGG  PPPPPPPP   LL       /
//           /   GG      GG  PP         LL       /
//          /     GGGGGGG   PP         LLLLLLL  /
//         /___________________________________/
//
// Copyright (c) 2005 Quantum Leaps, LLC
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open-source software licensed under the GNU
// General Public License (GPL) as published by the Free Software Foundation
// (see <https://www.gnu.org/licenses>).
//
// NOTE:
// The GPL does NOT permit the incorporation of this code into proprietary
// programs. Please contact Quantum Leaps for commercial licensing options,
// which expressly supersede the GPL and are designed explicitly for licensees
// interested in retaining the proprietary status of the generated code.
//
// Quantum Leaps contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${./Source::TLblinker.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

enum {
    ledOFF = 0,
    ledON
};

#define setLEDon()   TLblinker_setLed(me, ledON)
#define setLEDoff()  TLblinker_setLed(me, ledOFF)

/* Active object class -----------------------------------------------------*/
//$declare${AOs::TLblinker} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::TLblinker} ..........................................................
typedef struct {
// protected:
    QActive super;

// private:
    QTimeEvt timeEvt;
} TLblinker;

// public:
static void TLblinker_setLed(TLblinker * const me,
    uint16_t ledStatus);

// protected:
static QState TLblinker_initial(TLblinker * const me, void const * const par);
static QState TLblinker_RUN(TLblinker * const me, QEvt const * const e);
static QState TLblinker_INIT(TLblinker * const me, QEvt const * const e);
static QState TLblinker_INACTIVE(TLblinker * const me, QEvt const * const e);
static QState TLblinker_ACTIVE(TLblinker * const me, QEvt const * const e);
static QState TLblinker_OFF(TLblinker * const me, QEvt const * const e);
static QState TLblinker_ON(TLblinker * const me, QEvt const * const e);
static QState TLblinker_SHUTDOWN(TLblinker * const me, QEvt const * const e);
//$enddecl${AOs::TLblinker} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* Local objects -----------------------------------------------------------*/
static TLblinker l_blinker;   /* storage for PL */

/* Global objects ----------------------------------------------------------*/
QActive * const AO_TLblinker = &l_blinker.super;   /* "opaque" pointer to PL AO */

/* PL definition --------------------------------------------------------*/
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U)%0x2710U))
#error qpc version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${AOs::TLblinker_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::TLblinker_ctor} .....................................................
void TLblinker_ctor(void) {
    TLblinker *me = &l_blinker;

    QS_FUN_DICTIONARY(TLblinker_initial);
    QS_FUN_DICTIONARY(TLblinker_RUN);
    QS_FUN_DICTIONARY(TLblinker_INIT);
    QS_FUN_DICTIONARY(TLblinker_INACTIVE);
    QS_FUN_DICTIONARY(TLblinker_ACTIVE);
    QS_FUN_DICTIONARY(TLblinker_OFF);
    QS_FUN_DICTIONARY(TLblinker_ON);
    QS_FUN_DICTIONARY(TLblinker_SHUTDOWN);
    QS_OBJ_DICTIONARY(AO_TLblinker);

    QActive_ctor(&me->super, Q_STATE_CAST(&TLblinker_initial));
    QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
}
//$enddef${AOs::TLblinker_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${AOs::TLblinker} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::TLblinker} ..........................................................

//${AOs::TLblinker::setLed} ..................................................
static void TLblinker_setLed(TLblinker * const me,
    uint16_t ledStatus)
{
    Q_ASSERT((ledStatus == ledON) || (ledStatus == ledOFF));
    BSP_setPedLed(ledStatus);
}

//${AOs::TLblinker::SM} ......................................................
static QState TLblinker_initial(TLblinker * const me, void const * const par) {
    //${AOs::TLblinker::SM::initial}
    QActive_subscribe((QActive *)me, START_BLINK_SIG);
    QActive_subscribe((QActive *)me, STOP_BLINK_SIG);
    QActive_subscribe((QActive *)me, OFF_BLINK_SIG);
    QActive_subscribe((QActive *)me, GLOBAL_START_SIG);
    QActive_subscribe((QActive *)me, EMERGENCY_SIG);
    return Q_TRAN(&TLblinker_RUN);
}

//${AOs::TLblinker::SM::RUN} .................................................
static QState TLblinker_RUN(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::initial}
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&TLblinker_INIT);
            break;
        }
        //${AOs::TLblinker::SM::RUN::EMERGENCY}
        case EMERGENCY_SIG: {
            status_ = Q_TRAN(&TLblinker_INIT);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

//${AOs::TLblinker::SM::RUN::INIT} ...........................................
static QState TLblinker_INIT(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::INIT}
        case Q_ENTRY_SIG: {
            setLEDoff();
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::INIT::GLOBAL_START}
        case GLOBAL_START_SIG: {
            status_ = Q_TRAN(&TLblinker_INACTIVE);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLblinker_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLblinker::SM::RUN::INACTIVE} .......................................
static QState TLblinker_INACTIVE(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::INACTIVE}
        case Q_ENTRY_SIG: {
            setLEDon();
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::INACTIVE::START_BLINK}
        case START_BLINK_SIG: {
            status_ = Q_TRAN(&TLblinker_ACTIVE);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLblinker_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLblinker::SM::RUN::ACTIVE} .........................................
static QState TLblinker_ACTIVE(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::ACTIVE}
        case Q_ENTRY_SIG: {
            QTimeEvt_armX(&me->timeEvt, T_250ms, T_250ms);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::ACTIVE}
        case Q_EXIT_SIG: {
            QTimeEvt_disarm(&me->timeEvt);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::ACTIVE::initial}
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&TLblinker_OFF);
            break;
        }
        //${AOs::TLblinker::SM::RUN::ACTIVE::STOP_BLINK}
        case STOP_BLINK_SIG: {
            status_ = Q_TRAN(&TLblinker_SHUTDOWN);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLblinker_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLblinker::SM::RUN::ACTIVE::OFF} ....................................
static QState TLblinker_OFF(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::ACTIVE::OFF}
        case Q_ENTRY_SIG: {
            setLEDoff();
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::ACTIVE::OFF::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&TLblinker_ON);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLblinker_ACTIVE);
            break;
        }
    }
    return status_;
}

//${AOs::TLblinker::SM::RUN::ACTIVE::ON} .....................................
static QState TLblinker_ON(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::ACTIVE::ON}
        case Q_ENTRY_SIG: {
            setLEDon();
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::ACTIVE::ON::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&TLblinker_OFF);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLblinker_ACTIVE);
            break;
        }
    }
    return status_;
}

//${AOs::TLblinker::SM::RUN::SHUTDOWN} .......................................
static QState TLblinker_SHUTDOWN(TLblinker * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLblinker::SM::RUN::SHUTDOWN}
        case Q_ENTRY_SIG: {
            setLEDoff();
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLblinker::SM::RUN::SHUTDOWN::OFF_BLINK}
        case OFF_BLINK_SIG: {
            status_ = Q_TRAN(&TLblinker_INACTIVE);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLblinker_RUN);
            break;
        }
    }
    return status_;
}
//$enddef${AOs::TLblinker} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
