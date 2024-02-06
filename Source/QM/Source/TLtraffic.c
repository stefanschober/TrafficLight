//$file${./Source::TLtraffic.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: TrafficLight.qm
// File:  ${./Source::TLtraffic.c}
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
//$endhead${./Source::TLtraffic.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

#define startTimeout(timeout)    QTimeEvt_rearm(&me->timeEvt, timeout)
#define sendMessage(msg)         { QEvt *e = Q_NEW(QEvt, msg); QF_PUBLISH(e, (QActive *)me); }
/* helper macro to provide the ID of Philo "me_" */
// #define TL_ID(me_)    ((eTLidentity_t)((me_) - l_traffic))

/* Active object class -----------------------------------------------------*/
//$declare${AOs::TLtraffic} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::TLtraffic} ..........................................................
typedef struct {
// protected:
    QActive super;

// private:
    eTLidentity_t identity;
    uint8_t light;
    QTimeEvt timeEvt;

// public:
    uint32_t toGreen1;
    uint32_t toGreen2;
    uint32_t toYellow;
    uint32_t toRed;
    uint32_t toEmergency;
    uint32_t toInitEmergency;
    uint32_t toInit;
    uint32_t toRdYlw;
} TLtraffic;

// private:
static void TLtraffic_setLight(TLtraffic * const me,
    uint8_t light);

// protected:
static QState TLtraffic_initial(TLtraffic * const me, void const * const par);
static QState TLtraffic_RUN(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_RED(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_RED_1(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_RED_2(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_RED_3(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_RED_4(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_INIT_TL(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_GREEN(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_GREEN_1(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_GREEN_2(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_GREEN_3(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_YELLOW(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_YELLOW_1(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_YELLOW_2(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_EMERGENCY(TLtraffic * const me, QEvt const * const e);
static QState TLtraffic_RED_YELLOW_1(TLtraffic * const me, QEvt const * const e);
//$enddecl${AOs::TLtraffic} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

/* Local objects -----------------------------------------------------------*/
static TLtraffic l_trafficA;   /* storage for all TLs */
static TLtraffic l_trafficB;   /* storage for all TLs */
static TLtraffic *l_traffic[N_TL] = {
    &l_trafficA,
    &l_trafficB
};

/* Global objects ----------------------------------------------------------*/
QActive * const AO_TLtrafficA = &l_trafficA.super; /* "opaque" pointers to TL AO */
QActive * const AO_TLtrafficB = &l_trafficB.super; /* "opaque" pointers to TL AO */

/* TL definition --------------------------------------------------------*/
//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 730U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 7.3.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${AOs::TLtraffic_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::TLtraffic_ctor} .....................................................
int16_t TLtraffic_ctor(void) {
    QS_FUN_DICTIONARY(TLtraffic_initial);
    QS_FUN_DICTIONARY(TLtraffic_RUN);
    QS_FUN_DICTIONARY(TLtraffic_EMERGENCY);
    QS_FUN_DICTIONARY(TLtraffic_INIT_TL);
    QS_FUN_DICTIONARY(TLtraffic_GREEN);
    QS_FUN_DICTIONARY(TLtraffic_GREEN_1);
    QS_FUN_DICTIONARY(TLtraffic_GREEN_2);
    QS_FUN_DICTIONARY(TLtraffic_GREEN_3);
    QS_FUN_DICTIONARY(TLtraffic_YELLOW);
    QS_FUN_DICTIONARY(TLtraffic_YELLOW_1);
    QS_FUN_DICTIONARY(TLtraffic_YELLOW_2);
    QS_FUN_DICTIONARY(TLtraffic_RED);
    QS_FUN_DICTIONARY(TLtraffic_RED_1);
    QS_FUN_DICTIONARY(TLtraffic_RED_2);
    QS_FUN_DICTIONARY(TLtraffic_RED_3);
    QS_FUN_DICTIONARY(TLtraffic_RED_4);

    QS_OBJ_DICTIONARY(AO_TLtrafficA);
    QS_OBJ_DICTIONARY(AO_TLtrafficB);

    for (uint8_t n = 0; n < N_TL; n++)
    {
        TLtraffic *me = l_traffic[n];

        QActive_ctor(&me->super, Q_STATE_CAST(&TLtraffic_initial));
        QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
        me->identity = (eTLidentity_t)n;
        Q_ENSURE((me->identity == TrafficLightA) || (me->identity == TrafficLightB));
        me->light = RED;

        me->toInit = T_5sec;
        me->toEmergency = T_10sec;
        me->toInitEmergency = T_500ms;
        me->toRed = T_2sec;
        me->toYellow = T_2sec;
        me->toRdYlw = T_2sec;
        me->toGreen1 = T_5sec;
        me->toGreen2 = T_5sec;
    }
}
//$enddef${AOs::TLtraffic_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//$define${AOs::TLtraffic} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${AOs::TLtraffic} ..........................................................

//${AOs::TLtraffic::setLight} ................................................
static void TLtraffic_setLight(TLtraffic * const me,
    uint8_t light)
{
    me->light = light;
    BSP_setlight(me->identity, me->light);
}

//${AOs::TLtraffic::SM} ......................................................
static QState TLtraffic_initial(TLtraffic * const me, void const * const par) {
    //${AOs::TLtraffic::SM::initial}
    QActive_subscribe((QActive *)me, PL_IS_RED_SIG);
    QActive_subscribe((QActive *)me, STARTNEWCYCLE_SIG);
    QActive_subscribe((QActive *)me, PEDREQUEST_SIG);
    QActive_subscribe((QActive *)me, EMERGENCY_SIG);
    QActive_subscribe((QActive *)me, EM_RELEASE_SIG);
    return Q_TRAN(&TLtraffic_RUN);
}

//${AOs::TLtraffic::SM::RUN} .................................................
static QState TLtraffic_RUN(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::initial}
        case Q_INIT_SIG: {
            startTimeout(me->toInit); // was 10sec
            status_ = Q_TRAN(&TLtraffic_INIT_TL);
            break;
        }
        //${AOs::TLtraffic::SM::RUN::EMERGENCY}
        case EMERGENCY_SIG: {
            status_ = Q_TRAN(&TLtraffic_EMERGENCY);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::RED} ............................................
static QState TLtraffic_RED(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::RED}
        case Q_ENTRY_SIG: {
            TLtraffic_setLight(me, RED);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::RED::initial}
        case Q_INIT_SIG: {
            sendMessage(TL_IS_RED_SIG);
            status_ = Q_TRAN(&TLtraffic_RED_1);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::RED::RED_1} .....................................
static QState TLtraffic_RED_1(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::RED::RED_1::PEDREQUEST}
        case PEDREQUEST_SIG: {
            sendMessage(TL_IS_RED_SIG);
            status_ = Q_TRAN(&TLtraffic_RED_2);
            break;
        }
        //${AOs::TLtraffic::SM::RUN::RED::RED_1::STARTNEWCYCLE}
        case STARTNEWCYCLE_SIG: {
            status_ = Q_TRAN(&TLtraffic_RED_3);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RED);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::RED::RED_2} .....................................
static QState TLtraffic_RED_2(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::RED::RED_2::PL_IS_RED}
        case PL_IS_RED_SIG: {
            status_ = Q_TRAN(&TLtraffic_RED_1);
            break;
        }
        //${AOs::TLtraffic::SM::RUN::RED::RED_2::STARTNEWCYCLE}
        case STARTNEWCYCLE_SIG: {
            status_ = Q_TRAN(&TLtraffic_RED_4);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RED);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::RED::RED_3} .....................................
static QState TLtraffic_RED_3(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::RED::RED_3}
        case Q_ENTRY_SIG: {
            startTimeout(me->toRed);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::RED::RED_3::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&TLtraffic_RED_YELLOW_1);
            break;
        }
        //${AOs::TLtraffic::SM::RUN::RED::RED_3::PEDREQUEST}
        case PEDREQUEST_SIG: {
            sendMessage(TL_IS_RED_SIG);
            status_ = Q_TRAN(&TLtraffic_RED_4);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RED);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::RED::RED_4} .....................................
static QState TLtraffic_RED_4(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::RED::RED_4::PL_IS_RED}
        case PL_IS_RED_SIG: {
            status_ = Q_TRAN(&TLtraffic_RED_3);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RED);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::INIT_TL} ........................................
static QState TLtraffic_INIT_TL(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::INIT_TL}
        case Q_ENTRY_SIG: {
            TLtraffic_setLight(me, RED);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::INIT_TL::TIMEOUT}
        case TIMEOUT_SIG: {
            //${AOs::TLtraffic::SM::RUN::INIT_TL::TIMEOUT::[(TrafficLightA==(me->identity))~}
            if (( TrafficLightA == (me->identity) )) {
                status_ = Q_TRAN(&TLtraffic_RED_YELLOW_1);
            }
            //${AOs::TLtraffic::SM::RUN::INIT_TL::TIMEOUT::[else]}
            else {
                status_ = Q_TRAN(&TLtraffic_RED);
            }
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::GREEN} ..........................................
static QState TLtraffic_GREEN(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::GREEN}
        case Q_ENTRY_SIG: {
            sendMessage(GLOBAL_START_SIG);
            TLtraffic_setLight(me, GREEN);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::GREEN::initial}
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&TLtraffic_GREEN_1);
            break;
        }
        //${AOs::TLtraffic::SM::RUN::GREEN::TIMEOUT,PEDREQUEST}
        case TIMEOUT_SIG: // intentionally fall through
        case PEDREQUEST_SIG: {
            status_ = Q_TRAN(&TLtraffic_YELLOW);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::GREEN::GREEN_1} .................................
static QState TLtraffic_GREEN_1(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::GREEN::GREEN_1}
        case Q_ENTRY_SIG: {
            startTimeout(me->toGreen1); // was 10s
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::GREEN::GREEN_1::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&TLtraffic_GREEN_2);
            break;
        }
        //${AOs::TLtraffic::SM::RUN::GREEN::GREEN_1::PEDREQUEST}
        case PEDREQUEST_SIG: {
            status_ = Q_TRAN(&TLtraffic_GREEN_3);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_GREEN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::GREEN::GREEN_2} .................................
static QState TLtraffic_GREEN_2(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::GREEN::GREEN_2}
        case Q_ENTRY_SIG: {
            startTimeout(me->toGreen2); // was 20s
            status_ = Q_HANDLED();
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_GREEN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::GREEN::GREEN_3} .................................
static QState TLtraffic_GREEN_3(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        default: {
            status_ = Q_SUPER(&TLtraffic_GREEN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::YELLOW} .........................................
static QState TLtraffic_YELLOW(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::YELLOW}
        case Q_ENTRY_SIG: {
            TLtraffic_setLight(me, YELLOW);
            startTimeout(me->toYellow); // was 5s
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::YELLOW::initial}
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&TLtraffic_YELLOW_1);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::YELLOW::YELLOW_1} ...............................
static QState TLtraffic_YELLOW_1(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::YELLOW::YELLOW_1::TIMEOUT}
        case TIMEOUT_SIG: {
            sendMessage(STARTNEWCYCLE_SIG);
            status_ = Q_TRAN(&TLtraffic_YELLOW_2);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_YELLOW);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::YELLOW::YELLOW_2} ...............................
static QState TLtraffic_YELLOW_2(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::YELLOW::YELLOW_2::STARTNEWCYCLE}
        case STARTNEWCYCLE_SIG: {
            status_ = Q_TRAN(&TLtraffic_RED);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_YELLOW);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::EMERGENCY} ......................................
static QState TLtraffic_EMERGENCY(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::EMERGENCY}
        case Q_ENTRY_SIG: {
            TLtraffic_setLight(me, RED);
            startTimeout(me->toEmergency); // was 10sec
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::EMERGENCY::TIMEOUT,EM_RELEASE}
        case TIMEOUT_SIG: // intentionally fall through
        case EM_RELEASE_SIG: {
            startTimeout(me->toInitEmergency); // was 10sec
            status_ = Q_TRAN(&TLtraffic_INIT_TL);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RUN);
            break;
        }
    }
    return status_;
}

//${AOs::TLtraffic::SM::RUN::RED_YELLOW_1} ...................................
static QState TLtraffic_RED_YELLOW_1(TLtraffic * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        //${AOs::TLtraffic::SM::RUN::RED_YELLOW_1}
        case Q_ENTRY_SIG: {
            startTimeout(me->toRdYlw);
            TLtraffic_setLight(me, RED_YELLOW);
            status_ = Q_HANDLED();
            break;
        }
        //${AOs::TLtraffic::SM::RUN::RED_YELLOW_1::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&TLtraffic_GREEN);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLtraffic_RUN);
            break;
        }
    }
    return status_;
}
//$enddef${AOs::TLtraffic} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

