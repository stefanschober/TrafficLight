/*$file${./Source::trafficlight.h} #########################################*/
/*
* Model: TrafficLight.qm
* File:  ${./Source::trafficlight.h}
*
* This code has been generated by QM 4.2.1 (https://www.state-machine.com/qm).
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
/*$endhead${./Source::trafficlight.h} ######################################*/
#ifndef trafficlight_h
#define trafficlight_h

enum TLSignals {
    TIME_TICK_SIG = Q_USER_SIG, /* time ticker event */
    GLOBAL_START_SIG,
    STARTNEWCYCLE_SIG,             /* published by traffic light to start cycle */
    PEDREQUEST_SIG,       /* published by pedestrian light to start cycle */
    TL_IS_RED_SIG,
    PL_IS_RED_SIG,
    BUTTON_SIG,                 /* published by BSP to notify about pedestrian requests */
    START_BLINK_SIG,
    STOP_BLINK_SIG,
    OFF_BLINK_SIG,
    EMERGENCY_SIG,
    EM_RELEASE_SIG,
    MAX_PUBLISH_SIG,
    TIMEOUT_SIG,                /* used for time events */
    MAX_SIG                     /* the last signal */
};

typedef enum TLidentity eTLidentity_t;
typedef enum TLlight    eTLlight_t;

enum TLidentity {
    TrafficLightA = 0,
    TrafficLightB,
    PedestrianLight,
    MaxIdentity
};

enum TLlight {
    RED = 0,
    YELLOW,
    GREEN,
    NO_LIGHT
};

#define _TIMETICK_(t)    ((t) * (BSP_TICKS_PER_SEC))
#define _TIMETICKms_(t)  ((_TIMETICK_(t) / 1000u) ? (_TIMETICK_(t) / 1000u) : 1u)

/* timeout definitions */
#define T_5ms     _TIMETICKms_(5u)
#define T_10ms    _TIMETICKms_(10u)
#define T_50ms    _TIMETICKms_(50u)
#define T_100ms   _TIMETICKms_(100u)
#define T_250ms   _TIMETICKms_(250u)
#define T_500ms   _TIMETICKms_(500u)
#define T_1sec    _TIMETICKms_(1000u)
#define T_2sec    _TIMETICK_(2u)
#define T_5sec    _TIMETICK_(5u)
#define T_10sec   _TIMETICK_(10u)
#define T_15sec   _TIMETICK_(15u)
#define T_20sec   _TIMETICK_(20u)
#define T_30sec   _TIMETICK_(30u)

/*$declare${Events::TLevt} #################################################*/
/*${Events::TLevt} .........................................................*/
typedef struct {
/* protected: */
    QEvt super;
} TLevt;
/*$enddecl${Events::TLevt} #################################################*/

/* number of traffic lights */
#define N_TL ((uint8_t)2)

/*$declare${AOs::TLtraffic_ctor} ###########################################*/
/*${AOs::TLtraffic_ctor} ...................................................*/
void TLtraffic_ctor(void);
/*$enddecl${AOs::TLtraffic_ctor} ###########################################*/
/*$declare${AOs::AO_TLtraffic[N_TL]} #######################################*/
extern QActive * const AO_TLtraffic[N_TL];
/*$enddecl${AOs::AO_TLtraffic[N_TL]} #######################################*/

/*$declare${AOs::TLpedestrian_ctor} ########################################*/
/*${AOs::TLpedestrian_ctor} ................................................*/
void TLpedestrian_ctor(void);
/*$enddecl${AOs::TLpedestrian_ctor} ########################################*/
/*$declare${AOs::AO_TLpedestrian} ##########################################*/
extern QActive * const AO_TLpedestrian;
/*$enddecl${AOs::AO_TLpedestrian} ##########################################*/

// $declare(AOs::TLbutton_ctor)
// $declare(AOs::AO_TLbutton)

/*$declare${AOs::TLblinker_ctor} ###########################################*/
/*${AOs::TLblinker_ctor} ...................................................*/
void TLblinker_ctor(void);
/*$enddecl${AOs::TLblinker_ctor} ###########################################*/
/*$declare${AOs::AO_TLblinker} #############################################*/
extern QActive * const AO_TLblinker;
/*$enddecl${AOs::AO_TLblinker} #############################################*/

#endif /* trafficlight_h */