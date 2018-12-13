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

#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"
#include "bsp-gui.h"

#if defined RASPI
#include <pigpio.h>
#endif

#include <gtk/gtk.h>

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

#ifdef Q_SPY
#include "qspy.h"
static guint8            l_running = 0;

enum {
    TL_STAT = QS_USER,
    COMMAND_STAT
};
static guint const l_clock_tick = 0U;
static guint const l_button     = 1U;
#endif


int main (gint argc, gchar *argv[])
{
    return startGui(argc, argv);
}


/*..........................................................................*/
void BSP_init(gint argc, gchar *argv[])
{
    Q_ALLEGE(QS_INIT(argc > 1 ? argv[1] : (gpointer)0));
#if defined RASPI
    uint16_t io;

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
    QS_OBJ_DICTIONARY(&l_clock_tick); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_button); /* must be called *after* QF_init() */
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
void BSP_setlight(eTLidentity_t id, eTLlight_t light)
{
    guiSetLight(id, light);
#if defined RASPI
    switch(id)
    {
        case TrafficLightA:
            gpioWrite(pinTLAred,     light == RED    ? 1 : 0);
            gpioWrite(pinTLAyellow,  light == YELLOW ? 1 : 0);
            gpioWrite(pinTLAgreen,   light == GREEN  ? 1 : 0);
            break;
        case TrafficLightB:
            gpioWrite(pinTLBred,     light == RED    ? 1 : 0);
            gpioWrite(pinTLByellow,  light == YELLOW ? 1 : 0);
            gpioWrite(pinTLBgreen,   light == GREEN  ? 1 : 0);
            break;
        case PedestrianLight:
            gpioWrite(pinTLPedred,   light == RED    ? 1 : 0);
            gpioWrite(pinTLPedgreen, light == GREEN  ? 1 : 0);
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
	static QEvt const buttonEvt = { BUTTON_SIG, 0U, 0U };

    QF_PUBLISH(&buttonEvt, &l_button); /* publish to all subscribers */
}
// called when button EMERGENCY is clicked
void BSP_publishEmergencyEvt(void)
{
	static QEvt emergencyEvt = { EM_RELEASE_SIG, 0U, 0U };

    emergencyEvt.sig = ((emergencyEvt.sig == EMERGENCY_SIG) ? EM_RELEASE_SIG : EMERGENCY_SIG);
    QF_PUBLISH(&emergencyEvt, &l_button); /* publish to all subscribers */
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
    //static QEvt const tickEvt = { TIME_TICK_SIG, 0U, 0U };

    QACTIVE_POST(the_Ticker0, 0, &l_clock_tick); /* post to Ticker0 */
    //QF_PUBLISH(&tickEvt, &l_clock_tick); /* publish to all subscribers */
}
/*..........................................................................*/
void Q_onAssert(gchar const * const module, gint loc) {
    QF_stop(); /* stop ticking */
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */
    g_error("Assertion failed in module %s location %d", module, loc);
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY /* define QS callbacks */

#include <gio/gio.h>
static GSocket *socket = NULL;

/*
* In this demo, the QS software tracing output is sent out of the application
* through a TCP/IP socket. This requires the QSPY host application to
* be started first to open a server socket (qspy -t ...) to wait for the
* incoming TCP/IP connection from the DPP demo.
*
* In an embedded target, the QS software tracing output can be sent out
* using any method available, such as a UART. This would require changing
* the implementation of the functions in this section, but the rest of the
* application code does not "know" (and should not care) how the QS ouptut
* is actually performed. In other words, the rest of the application does NOT
* need to change in any way to produce QS output.
*/

/*..........................................................................*/
static gpointer idleThread(gpointer par) {/* signature for CreateThread() */
    (void)par;

    l_running = (uint8_t)1;
    while ((socket != NULL) || (l_running != 0)) {
        guint16  nBytes = 256;
        guchar   const *block;

#if defined QSPY_NET
        /* try to receive bytes from the QS socket... */
        nBytes = QS_rxGetNfree();
        if (nBytes > 0U) {
            guint8 buf[64];
            guint16 status;

            if (nBytes > sizeof(buf)) {
                nBytes = sizeof(buf);
            }
            status = (guint16)g_socket_receive(socket, (gchar *)buf, (gsize)nBytes, NULL, NULL);
            if (status > 0) {
                guint16 i;
                nBytes = (guint16)status;
                for (i = 0U; i < nBytes; ++i) {
                    QS_RX_PUT(buf[i]);
                }
            }
        }
        QS_rxParse();  /* parse all the received bytes */

        nBytes = 1024U;
#endif
        QF_CRIT_ENTRY(dummy);
        block = QS_getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);

        if (block != (gpointer)0) {
#if defined QSPY_NET
            g_socket_send(socket, (gchar const *)block, nBytes, NULL, NULL);
#else
            QSPY_parse(block, nBytes);
#endif
        }
        g_usleep(10000);   /* sleep for a while */
    }
    return NULL; /* return success */
}
/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static guint8 qsBuf[2 * 1024];  /* buffer for QS output */
    static guint8 qsRxBuf[100]; /* buffer for QS receive channel */

    QS_initBuf(qsBuf, sizeof(qsBuf));
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

#if defined QSPY_NET
    gchar const *hostName, *src, **splitSrc;
    guint16 port         = 6601; /* default QSPY server port */
    GError           *err = NULL;
    GSocketClient *client = g_socket_client_new();

    src = (arg == NULL) ? "localhost" : (char const *)arg;
    splitSrc = (gchar const **)g_strsplit(src, ":", 2);

    hostName = splitSrc[0];
    if (NULL != splitSrc[1]) {
        port = (uint16_t)g_ascii_strtoull(splitSrc[1], NULL, 10);
    }

    connection = g_socket_client_connect_to_host(client, hostName, port, NULL, &err);
    socket     = g_socket_connection_get_socket(connection);
    g_object_unref(client);
    if ((err != NULL) || (socket == NULL)) {
        g_message("Cannot connect to the QSPY server; error 0x%08X\n"
                  "(%s)", err->code, err->message);
        if (err) g_error_free(err);
        return (uint8_t)0; /* failure */
    }

    /* Set the socket to non-blocking mode. */
    g_socket_set_blocking(socket, FALSE);

#else

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

#endif

    /* set up the QS filters... */
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_OFF(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_OFF(QS_QEP_UNHANDLED);

    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_PUBLISH);

    // QS_FILTER_ON(TL_STAT);
    QS_FILTER_ON(COMMAND_STAT);

    /* return the status of creating the idle thread */
    return (g_thread_try_new(NULL, &idleThread, NULL, NULL)
            != NULL) ? (uint8_t)1 : (uint8_t)0;
}
/*..........................................................................*/
void QS_onCleanup(void) {
#if defined QSPY_NET
    if (connection != NULL) {
		g_socket_close(socket, NULL);
		socket = NULL;
        g_object_unref(connection);
        connection = NULL;
    }
#else
    l_running = (uint8_t)0;
    QSPY_stop();
#endif
}
/*..........................................................................*/
void QS_onFlush(void) {
#if defined QSPY_NET
#  define NBYTES    1000
#else
#  define NBYTES    1024
#endif
    guint16 nBytes = NBYTES;
    guint8 const *block;

    for (; socket || l_running;) {
        QF_CRIT_ENTRY(dummy);
        block = QS_getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);

        if (block != (uint8_t const *)0) {
#if defined QSPY_NET
            g_socket_send(socket, (char const *)block, nBytes, NULL, NULL);
#else
            QSPY_parse(block, nBytes);
#endif
            nBytes = NBYTES;
        }
        else {
            break;
        }
    }
    g_usleep(50000);
#undef NBYTES
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) {
    return (QSTimeCtr)clock();
}
/*..........................................................................*/
/*! callback function to reset the target (to be implemented in the BSP) */
void QS_onReset(void) {
    //TBD
}
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
    fputs(QSPY_line, stderr);
    fputc('\n', stderr);
}
#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/

