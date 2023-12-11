/*****************************************************************************
 * Product: DPP example, Win32-GUI
 * Last updated for version 5.9.0
 * Last updated on  2017-04-15
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
#include <unistd.h>
#include <gtk/gtk.h>

#if defined RASPI
#include "pigpio.h"
#endif

#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"
#include "bsp-gui.h"

Q_DEFINE_THIS_FILE

// LOCAL functions ----------------------------------------------------------
#if defined RASPI
enum outputPins {
    pinTLAred     = 17,
    pinTLAyellow  = 27,
    pinTLAgreen   = 22,

    pinTLBred     = 23,
    pinTLByellow  = 24,
    pinTLBgreen   = 25,

    pinTLPedred   = 10,
    pinTLPedgreen =  9,

    pinTLblink   = 11
};

enum inputPins {
    pinUsrButton = 2
};
#endif

// static struct termios l_tsav; /* structure with saved terminal attributes */

int main (gint argc, gchar *argv[])
{
    return startGui(argc, argv);
}


/*..........................................................................*/
void BSP_HW_init() {

}
/*..........................................................................*/
void BSP_init(gint argc, gchar *argv[])
{
    Q_ALLEGE(QS_INIT(argc > 1 ? argv[1] : (gpointer)0));
#if defined RASPI
    // uint16_t io;

	Q_ALLEGE(gpioInitialise() >= 0);
	Q_ALLEGE(gpioSetMode(pinUsrButton,  PI_INPUT) == 0);

    Q_ALLEGE(gpioSetMode(pinTLAred,     PI_OUTPUT) == 0);
    Q_ALLEGE(gpioSetMode(pinTLAyellow,  PI_OUTPUT) == 0);
    Q_ALLEGE(gpioSetMode(pinTLAgreen,   PI_OUTPUT) == 0);

    Q_ALLEGE(gpioSetMode(pinTLBred,     PI_OUTPUT) == 0);
    Q_ALLEGE(gpioSetMode(pinTLByellow,  PI_OUTPUT) == 0);
    Q_ALLEGE(gpioSetMode(pinTLBgreen,   PI_OUTPUT) == 0);

    Q_ALLEGE(gpioSetMode(pinTLPedred,   PI_OUTPUT) == 0);
    Q_ALLEGE(gpioSetMode(pinTLPedgreen, PI_OUTPUT) == 0);
#endif
    QS_OBJ_DICTIONARY(&l_SysTick_Handler); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_Button_Handler); /* must be called *after* QF_init() */
    QS_USR_DICTIONARY(TL_STAT);
}
/*..........................................................................*/
void BSP_terminate(gint16 result) {
    (void)result;
    QF_stop(); /* stop the main QF application and the ticker thread */
#if defined RASPI
    gpioTerminate();
#endif
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, uint8_t light)
{
    guiSetLight(id, light);
#if defined RASPI
    switch(id)
    {
        case TrafficLightA:
            gpioWrite(pinTLAred,     light & RED    ? 1 : 0);
            gpioWrite(pinTLAyellow,  light & YELLOW ? 1 : 0);
            gpioWrite(pinTLAgreen,   light & GREEN  ? 1 : 0);
            break;
        case TrafficLightB:
            gpioWrite(pinTLBred,     light & RED    ? 1 : 0);
            gpioWrite(pinTLByellow,  light & YELLOW ? 1 : 0);
            gpioWrite(pinTLBgreen,   light & GREEN  ? 1 : 0);
            break;
        case PedestrianLight:
            gpioWrite(pinTLPedred,   light & RED    ? 1 : 0);
            gpioWrite(pinTLPedgreen, light & GREEN  ? 1 : 0);
           break;
        default:
            break;
    }
#endif
#if 0
    QS_BEGIN(T, AO_Philo[n]) /* application-specific record begin */
    QS_U8(1, n);  /* Philosopher number */
    QS_STR(stat); /* Philosopher status */
    QS_END()
#endif
}
/*..........................................................................*/
void BSP_setPedLed(guint16 status)
{
    guiSetPedLed(status);
#if defined RASPI
    gpioWrite(pinTLblink, status ? 1 : 0);
#endif
}
/*..........................................................................*/
// called when button PEDESTRIAN is clicked
void BSP_publishBtnEvt(void)
{
	static QEvt const buttonEvt = { BUTTON_SIG, 0U, QEVT_MARKER };

    QF_PUBLISH(&buttonEvt, &l_Button_Handler); /* publish to all subscribers */
}
// called when button EMERGENCY is clicked
void BSP_publishEmergencyEvt(void)
{
	static QEvt emergencyEvt = { EM_RELEASE_SIG, 0U, QEVT_MARKER };

    emergencyEvt.sig = ((emergencyEvt.sig == EMERGENCY_SIG) ? EM_RELEASE_SIG : EMERGENCY_SIG);
    QF_PUBLISH(&emergencyEvt, &l_Button_Handler); /* publish to all subscribers */
}

/* QF callbacks ============================================================*/
/*..........................................................................*/
void QF_onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC, 30); /* set the desired tick rate */
}
/*..........................................................................*/
void QF_onCleanup(void) {
    g_message("\nBye! Bye!\n");

    QS_EXIT(); /* perfomr the QS cleanup */
}
/*..........................................................................*/
void QF_onClockTick(void) {
    QTIMEEVT_TICK(&l_SysTick_Handler); /* post to Ticker0 */
}
/*..........................................................................*/
void Q_onAssert(gchar const * const module, gint loc) {
    QF_stop(); /* stop ticking */
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */
    g_error("Assertion failed in module %s location %d", module, loc);
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY /* define QS callbacks */

/*..........................................................................*/
/*! callback function to execute a uesr command (to be implemented in BSP) */
void QS_onCommand(guint8 cmdId,
                  guint32 param1, guint32 param2, guint32 param3)
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
/*..........................................................................*/
void QSPY_onPrintLn(void) {
}
#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/

