/*.$file${./Source::TLpedestrian.c} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: TrafficLight.qm
* File:  ${./Source::TLpedestrian.c}
*
* This code has been generated by QM 5.1.3 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*.$endhead${./Source::TLpedestrian.c} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

Q_DEFINE_THIS_FILE

/* Active object class -----------------------------------------------------*/
/*.$declare${AOs::TLpedestrian} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::TLpedestrian} ....................................................*/
typedef struct {
/* protected: */
    QActive super;

/* private: */
    uint8_t light;
    QTimeEvt timeEvt;
    uint8_t tlRedCount;

/* public: */
    uint16_t ledStatus;
    uint32_t toRed;
    uint32_t toGreen;
} TLpedestrian;

/* private: */
static void TLpedestrian_sendMessage(TLpedestrian * const me, uint16_t msg);
static void TLpedestrian_setLight(TLpedestrian * const me, uint8_t light);

/* protected: */
static QState TLpedestrian_initial(TLpedestrian * const me, void const * const par);
static QState TLpedestrian_RUN(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_GREEN(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED_3(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED_5(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED_2(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED_4(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED_1(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_RED_6(TLpedestrian * const me, QEvt const * const e);
static QState TLpedestrian_INIT_WAIT(TLpedestrian * const me, QEvt const * const e);
/*.$enddecl${AOs::TLpedestrian} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* Local objects -----------------------------------------------------------*/
static TLpedestrian l_pedestrian;   /* storage for PL */

/* Global objects ----------------------------------------------------------*/
QActive * const AO_TLpedestrian = &l_pedestrian.super;   /* "opaque" pointer to PL AO */

/* PL definition --------------------------------------------------------*/
/*.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*. Check for the minimum required QP version */
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpc version 6.9.0 or higher required
#endif
/*.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${AOs::TLpedestrian_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::TLpedestrian_ctor} ...............................................*/
void TLpedestrian_ctor(void) {
    TLpedestrian *me = &l_pedestrian;

    QActive_ctor(&me->super, Q_STATE_CAST(&TLpedestrian_initial));
    QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);

    me->light = RED;
    me->tlRedCount = 0;

    me->ledStatus = 0;

    me->toRed = T_2sec;
    me->toGreen = T_5sec;
}
/*.$enddef${AOs::TLpedestrian_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*.$define${AOs::TLpedestrian} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${AOs::TLpedestrian} ....................................................*/
/*.${AOs::TLpedestrian::sendMessage} .......................................*/
static void TLpedestrian_sendMessage(TLpedestrian * const me, uint16_t msg) {
    QEvt *evt = Q_NEW(QEvt, msg);
    Q_ASSERT(evt != (void *)0);
    QF_PUBLISH(evt, me);
}

/*.${AOs::TLpedestrian::setLight} ..........................................*/
static void TLpedestrian_setLight(TLpedestrian * const me, uint8_t light) {
    me->light = light;
    BSP_setlight(PedestrianLight, me->light);
}

/*.${AOs::TLpedestrian::SM} ................................................*/
static QState TLpedestrian_initial(TLpedestrian * const me, void const * const par) {
    /*.${AOs::TLpedestrian::SM::initial} */
    QS_FUN_DICTIONARY(TLpedestrian_initial);
    QS_FUN_DICTIONARY(TLpedestrian_RUN);
    QS_FUN_DICTIONARY(TLpedestrian_GREEN);
    QS_FUN_DICTIONARY(TLpedestrian_RED);
    QS_FUN_DICTIONARY(TLpedestrian_RED_1);
    QS_FUN_DICTIONARY(TLpedestrian_RED_2);
    QS_FUN_DICTIONARY(TLpedestrian_RED_3);
    QS_FUN_DICTIONARY(TLpedestrian_RED_4);
    QS_FUN_DICTIONARY(TLpedestrian_RED_5);
    QS_FUN_DICTIONARY(TLpedestrian_INIT_WAIT);

    QActive_subscribe((QActive *)me, TL_IS_RED_SIG);
    QActive_subscribe((QActive *)me, BUTTON_SIG);
    QActive_subscribe((QActive *)me, GLOBAL_START_SIG);
    QActive_subscribe((QActive *)me, EMERGENCY_SIG);
    QActive_subscribe((QActive *)me, EM_RELEASE_SIG);
    return Q_TRAN(&TLpedestrian_RUN);
}
/*.${AOs::TLpedestrian::SM::RUN} ...........................................*/
static QState TLpedestrian_RUN(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&TLpedestrian_INIT_WAIT);
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::EMERGENCY} */
        case EMERGENCY_SIG: {
            status_ = Q_TRAN(&TLpedestrian_INIT_WAIT);
            break;
        }
        default: {
            status_ = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::GREEN} ....................................*/
static QState TLpedestrian_GREEN(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::GREEN} */
        case Q_ENTRY_SIG: {
            TLpedestrian_setLight(me, GREEN);
            QTimeEvt_rearm(&me->timeEvt, me->toGreen); // was 15s
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::GREEN::TIMEOUT} */
        case TIMEOUT_SIG: {
            TLpedestrian_sendMessage(me, PL_IS_RED_SIG);
            status_ = Q_TRAN(&TLpedestrian_RED);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RUN);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED} ......................................*/
static QState TLpedestrian_RED(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED} */
        case Q_ENTRY_SIG: {
            TLpedestrian_setLight(me, RED);
            TLpedestrian_sendMessage(me, OFF_BLINK_SIG);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED} */
        case Q_EXIT_SIG: {
            TLpedestrian_sendMessage(me, STOP_BLINK_SIG);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED::initial} */
        case Q_INIT_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED_1);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RUN);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED::RED_3} ...............................*/
static QState TLpedestrian_RED_3(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_3} */
        case Q_ENTRY_SIG: {
            TLpedestrian_sendMessage(me, PEDREQUEST_SIG);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_3::TL_IS_RED} */
        case TL_IS_RED_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED_4);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RED);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED::RED_5} ...............................*/
static QState TLpedestrian_RED_5(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_5} */
        case Q_ENTRY_SIG: {
            QTimeEvt_rearm(&me->timeEvt, me->toRed);
            me->tlRedCount = 0;
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_5::TIMEOUT} */
        case TIMEOUT_SIG: {
            status_ = Q_TRAN(&TLpedestrian_GREEN);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RED);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED::RED_2} ...............................*/
static QState TLpedestrian_RED_2(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_2} */
        case Q_EXIT_SIG: {
            TLpedestrian_sendMessage(me, START_BLINK_SIG);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_2::BUTTON} */
        case BUTTON_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED_3);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RED);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED::RED_4} ...............................*/
static QState TLpedestrian_RED_4(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_4::TL_IS_RED} */
        case TL_IS_RED_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED_5);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RED);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED::RED_1} ...............................*/
static QState TLpedestrian_RED_1(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_1::GLOBAL_START} */
        case GLOBAL_START_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED_2);
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_1::BUTTON} */
        case BUTTON_SIG: {
            TLpedestrian_sendMessage(me, START_BLINK_SIG);
            status_ = Q_TRAN(&TLpedestrian_RED_6);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RED);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::RED::RED_6} ...............................*/
static QState TLpedestrian_RED_6(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_6} */
        case Q_ENTRY_SIG: {
            TLpedestrian_sendMessage(me, START_BLINK_SIG);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::RED::RED_6::GLOBAL_START} */
        case GLOBAL_START_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED_3);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RED);
            break;
        }
    }
    return status_;
}
/*.${AOs::TLpedestrian::SM::RUN::INIT_WAIT} ................................*/
static QState TLpedestrian_INIT_WAIT(TLpedestrian * const me, QEvt const * const e) {
    QState status_;
    switch (e->sig) {
        /*.${AOs::TLpedestrian::SM::RUN::INIT_WAIT} */
        case Q_ENTRY_SIG: {
            QTimeEvt_disarm(&me->timeEvt);
            TLpedestrian_setLight(me, RED);
            status_ = Q_HANDLED();
            break;
        }
        /*.${AOs::TLpedestrian::SM::RUN::INIT_WAIT::GLOBAL_START} */
        case GLOBAL_START_SIG: {
            status_ = Q_TRAN(&TLpedestrian_RED);
            break;
        }
        default: {
            status_ = Q_SUPER(&TLpedestrian_RUN);
            break;
        }
    }
    return status_;
}
/*.$enddef${AOs::TLpedestrian} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
