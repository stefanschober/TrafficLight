//============================================================================
// Product: QUTEST fixture for the DPP components
// Last updated for version 7.3.0
// Last updated on  2023-08-22
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"
#include <stdio.h>

//#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities

Q_DEFINE_THIS_FILE

static const char *lightStr[] = {
    "off", "red", "yellow", "red/yellow", "green"
};
static QActive *theLights[3];

//============================================================================
int main(int argc, char *argv[])
{
    QF_init();    /* initialize the framework and the underlying RT kernel */
    BSP_init(argc, argv); /* initialize the Board Support Package */

    // pause execution of the test and wait for the test script to continue
    QS_TEST_PAUSE();

    return tlMain(); // run the QF application
}
/*..........................................................................*/
void BSP_init(int argc, char **argv)
{
    printf("Traffic Light example (qutest)"
           "\nQP %s\n"
           "Press 'p' for pedestrian signal\n"
           "Press ESC to quit...\n",
           QP_versionStr);

    theLights[0] = AO_TLtrafficA;
    theLights[1] = AO_TLtrafficB;
    theLights[2] = AO_TLpedestrian;
    
    Q_ASSERT(QS_INIT((argc > 1) ? argv[1] : ""));
    QS_OBJ_DICTIONARY(&l_SysTick_Handler); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_Button_Handler); /* must be called *after* QF_init() */
    QS_USR_DICTIONARY(TL_STAT);

    /* setup the QS filters... */
    QS_GLB_FILTER(QS_SM_RECORDS); // state machine records
    QS_GLB_FILTER(QS_UA_RECORDS); // all usedr records
    //QS_GLB_FILTER(QS_MUTEX_LOCK);
    //QS_GLB_FILTER(QS_MUTEX_UNLOCK);
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, uint8_t light)
{
    Q_ASSERT((light < 5) && (id < 3));

//    QS_BEGIN(T, theLights[id]) /* application-specific record begin */
//        QS_U8(1, id);  /* Philosopher number */
//        QS_STR(lightStr[light]); /* Philosopher status */
//    QS_END()
}
/*..........................................................................*/
void BSP_setPedLed(uint16_t status)
{
    (void)status;
}

//============================================================================
void QS_onTestSetup(void) {
    //PRINTF_S("%s\n", "QS_onTestSetup");
}
void QS_onTestTeardown(void) {
    //PRINTF_S("%s\n", "QS_onTestTeardown");
}
//............................................................................
// callback function to execute user commands
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);

    switch (cmdId) {
       case 0U: {
           // QEvt const e_pause = QEVT_INITIALIZER( PAUSE_SIG);
           // QASM_DISPATCH(AO_Table, &e_pause, 0U);
           break;
       }
       case 1U: {
           // QEvt const e_serve = QEVT_INITIALIZER(SERVE_SIG);
           // QASM_DISPATCH(AO_Table, &e_serve, 0U);
           break;
       }
       default:
           break;
    }
}

//============================================================================
// Host callback function to "massage" the event, if necessary
void QS_onTestEvt(QEvt *e) {
    (void)e;
#ifdef Q_HOST  // is this test compiled for a desktop Host computer?
#else // embedded Target
#endif // embedded Target
}
//............................................................................
// callback function to output the posted QP events (not used here)
void QS_onTestPost(void const *sender, QActive *recipient,
                   QEvt const *e, bool status)
{
    Q_UNUSED_PAR(sender);
    Q_UNUSED_PAR(status);
#if 0
    switch (e->sig) {
        case EAT_SIG:
        case DONE_SIG:
        case HUNGRY_SIG:
            QS_BEGIN_ID(QUTEST_ON_POST, 0U) // application-specific record
                QS_SIG(e->sig, recipient);
                QS_U8(0, Q_EVT_CAST(TableEvt)->philoId);
            QS_END()
            break;
        default:
            break;
    }
#endif
}
