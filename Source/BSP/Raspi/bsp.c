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
#if defined PIGPIO_LIB
#	include "pigpio.h"

#	define GPIO_INIT()			gpioInitialise()
#	define GPIO_TERMINATE()		gpioTerminate()

#	define GPIO_SETMODE(_p, _m)	gpioSetMode((_p), (_m))
#	define GPIO_WRITE(_p, _v)	gpioWrite((_p), (_v))
#	define GPIO_READ(_p)		gpioRead(_p)
#   define GPIO_SET_PUD(_p, _m) gpioSetPullUpDown((_p), (_m))

#	define GPIO_CALLBACK(_p, _f) gpioSetISRFunc((_p), EITHER_EDGE, -1, (gpioISRFunc_t)(_f))
#	define GPIO_CBDEL(_cb, _p)	 gpioSetISRFunc((_p), EITHER_EDGE, -1, NULL)

#	define SPI_OPEN(_c, _s, _f) spiOpen((_c), (_s), (_f));
#	define SPI_CLOSE(_h)		spiClose(_h);
#	define SPI_XFER(_h, _t, _r, _c) spiXfer((_h), (char *)(_t), (char *)(_r), (_c))

#	define I2C_OPEN(_b, _a, _f)	i2cOpen((_b), (_a), (_f))
#	define I2C_CLOSE(_h)		i2cClose(_h);
#	define I2C_RDREG8(_h, _r)	i2cReadByteData((_h), (_r))
#	define I2C_RDREG16(_h, _r)	i2cReadWordData((_h), (_r))
#	define I2C_WRREG8(_h, _r, _v)  i2cWriteByteData((_h), (_r), ((_v) & 0x0000FFu))
#	define I2C_WRREG16(_h, _r, _v) i2cWriteWordData((_h), (_r), ((_v) & 0x00FFFFu))
#  else
#   include "pigpiod_if2.h"
#   include "command.h"

#	define GPIO_INIT()			pigpio_start(NULL, NULL)
#	define GPIO_TERMINATE()		pigpio_stop(piHandle)

#	define GPIO_SETMODE(_p, _m) set_mode(piHandle, (_p), (_m))
#	define GPIO_WRITE(_p, _v)	gpio_write(piHandle, (_p), (_v))
#	define GPIO_READ(_p)		gpio_read(piHandle, (_p))
#   define GPIO_SET_PUD(_p, _m) set_pull_up_down(piHandle, (_p), (_m))

#	define GPIO_CALLBACK(_p, _f) callback(piHandle, (_p), EITHER_EDGE, (CBFunc_t)(_f))
#	define GPIO_CBDEL(_cb, _p)	 callback_cancel(_cb)

#	define SPI_OPEN(_c, _s, _f) spi_open(piHandle, (_c), (_s), (_f));
#	define SPI_CLOSE(_h)		spi_close(piHandle, (_h));
#	define SPI_XFER(_h, _t, _r, _c) spi_xfer(piHandle, (_h), (char *)(_t), (char *)(_r), (_c))

#	define I2C_OPEN(_b, _a, _f)	i2c_open(piHandle, (_b), (_a), (_f))
#	define I2C_CLOSE(_h)		i2c_close(piHandle, (_h));
#	define I2C_RDREG8(_h, _r)	i2c_read_byte_data(piHandle, (_h), (_r))
#	define I2C_RDREG16(_h, _r)	i2c_read_word_data(piHandle, (_h), (_r))
#	define I2C_WRREG8(_h, _r, _v)  i2c_write_byte_data(piHandle, (_h), (_r), ((_v) & 0x0000FFu))
#	define I2C_WRREG16(_h, _r, _v) i2c_write_word_data(piHandle, (_h), (_r), ((_v) & 0x00FFFFu))
#  endif /* defined PIGPIO_LIB */
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

static int piHandle = -1;
#endif

// static struct termios l_tsav; /* structure with saved terminal attributes */

int main (gint argc, gchar *argv[])
{
    return startGui(argc, argv);
}


/*..........................................................................*/
void BSP_HW_init()
{
#if defined RASPI
    // uint16_t io;

    piHandle = GPIO_INIT();
	Q_REQUIRE(piHandle >= 0);
	Q_REQUIRE(GPIO_SETMODE(pinUsrButton,  PI_INPUT) == 0);

    Q_REQUIRE(GPIO_SETMODE(pinTLAred,     PI_OUTPUT) == 0);
    Q_REQUIRE(GPIO_SETMODE(pinTLAyellow,  PI_OUTPUT) == 0);
    Q_REQUIRE(GPIO_SETMODE(pinTLAgreen,   PI_OUTPUT) == 0);

    Q_REQUIRE(GPIO_SETMODE(pinTLBred,     PI_OUTPUT) == 0);
    Q_REQUIRE(GPIO_SETMODE(pinTLByellow,  PI_OUTPUT) == 0);
    Q_REQUIRE(GPIO_SETMODE(pinTLBgreen,   PI_OUTPUT) == 0);

    Q_REQUIRE(GPIO_SETMODE(pinTLPedred,   PI_OUTPUT) == 0);
    Q_REQUIRE(GPIO_SETMODE(pinTLPedgreen, PI_OUTPUT) == 0);
#endif
}
/*..........................................................................*/
void BSP_init(gint argc, gchar *argv[])
{
    Q_REQUIRE(QS_INIT(argc > 1 ? argv[1] : (gpointer)0));

    QS_OBJ_DICTIONARY(&l_SysTick_Handler); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_Button_Handler); /* must be called *after* QF_init() */
    QS_USR_DICTIONARY(TL_STAT);
}
/*..........................................................................*/
void BSP_terminate(gint16 result) {
    (void)result;
    QF_stop(); /* stop the main QF application and the ticker thread */
#if defined RASPI
    GPIO_TERMINATE();
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
            GPIO_WRITE(pinTLAred,     light & RED    ? 1 : 0);
            GPIO_WRITE(pinTLAyellow,  light & YELLOW ? 1 : 0);
            GPIO_WRITE(pinTLAgreen,   light & GREEN  ? 1 : 0);
            break;
        case TrafficLightB:
            GPIO_WRITE(pinTLBred,     light & RED    ? 1 : 0);
            GPIO_WRITE(pinTLByellow,  light & YELLOW ? 1 : 0);
            GPIO_WRITE(pinTLBgreen,   light & GREEN  ? 1 : 0);
            break;
        case PedestrianLight:
            GPIO_WRITE(pinTLPedred,   light & RED    ? 1 : 0);
            GPIO_WRITE(pinTLPedgreen, light & GREEN  ? 1 : 0);
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
    GPIO_WRITE(pinTLblink, status ? 1 : 0);
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
    static uint8_t button = 0;

    QTICKER_TRIG(the_Ticker0, &l_SysTick_Handler); /* post to Ticker0 */

#if defined RASPI
    if(GPIO_READ(pinUsrButton))
    {
        if(button < 5)
        {
            button++;
            if(button == 5)
            {
                BSP_publishBtnEvt();
            }
        }
    }
    else
    {
        if(button > 0)
        {
            button--;
        }
    }
#endif
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

