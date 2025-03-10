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
#include <stdio.h>    /* for _snprintf_s() */
#include <stdlib.h>

#include "qpc.h"
#include "safe_std.h"
#include "qwin_gui.h" /* QWIN GUI */

#include "trafficlight.h"
#include "bsp.h"
#include "bsp-gui.h"

#ifdef Q_SPY
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>  /* Win32 API for multithreading */
    #include <winsock2.h> /* for Windows network facilities */

    Q_DEFINE_THIS_FILE
#endif

/* local variables ---------------------------------------------------------*/
#ifdef Q_SPY
    static SOCKET l_sock = INVALID_SOCKET;
#endif

/*..........................................................................*/
void QF_onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC, 30); /* set the desired tick rate */
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QF_onClockTick(void) {
    QTICKER_TRIG(the_Ticker0, &l_SysTick_Handler); /* post to Ticker0 */
}

/*..........................................................................*/
void Q_onAssert(char const * const module, int_t loc) {
    guiAssert(module, loc);
}

/*..........................................................................*/
void BSP_HW_init(void) {

}
/*..........................................................................*/
void BSP_init(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

#ifdef Q_SPY
    char *l_cmdLine = NULL;

    if(argc)
    {
        l_cmdLine = argv[0];
    }
    if (QS_INIT(l_cmdLine) == (uint8_t)0) { /* QS initialization failed? */
        MessageBox(NULL,
                   "Cannot connect to QSPY via TCP/IP\n"
                   "Please make sure that 'qspy -t' is running",
                   "QS_INIT() Error",
                   MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }
#endif
    QS_OBJ_DICTIONARY(&l_SysTick_Handler); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_Button_Handler); /* must be called *after* QF_init() */
    QS_USR_DICTIONARY(TL_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);

    QS_GLB_FILTER(QS_ALL_RECORDS);
}
/*..........................................................................*/
void BSP_terminate(int16_t result)
{
#ifdef Q_SPY
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
#endif
    QF_stop(); /* stop the main QF application and the ticker thread */

    guiTerminate(result);
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, uint8_t light)
{
    guiSetLight(id, light);
}
/*..........................................................................*/
void BSP_setPedLed(uint16_t status)
{
    guiSetPedLed(status);
}
/*..........................................................................*/
// called when button PEDESTRIAN is clicked
void BSP_publishBtnEvt(void)
{
	static QEvt const buttonEvt = QEVT_INITIALIZER(BUTTON_SIG);

    QF_PUBLISH(&buttonEvt, &l_Button_Handler); /* publish to all subscribers */
}
// called when button EMERGENCY is clicked
void BSP_publishEmergencyEvt(void)
{
	static QEvt emergencyEvt = QEVT_INITIALIZER(EM_RELEASE_SIG);

    emergencyEvt.sig = ((emergencyEvt.sig == EMERGENCY_SIG) ? EM_RELEASE_SIG : EMERGENCY_SIG);
    QF_PUBLISH(&emergencyEvt, &l_Button_Handler); /* publish to all subscribers */
}
/*..........................................................................*/

/*--------------------------------------------------------------------------*/
#ifdef Q_SPY /* define QS callbacks */

#include <time.h>

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
#if 0
static DWORD WINAPI idleThread(LPVOID par) {/* signature for CreateThread() */
    (void)par;
    while (l_sock != INVALID_SOCKET) {
        uint16_t nBytes;
        uint8_t const *block;

        /* try to receive bytes from the QS socket... */
        nBytes = QS_rxGetNfree();
        if (nBytes > 0U) {
            uint8_t buf[64];
            int status;

            if (nBytes > sizeof(buf)) {
                nBytes = sizeof(buf);
            }
            status = recv(l_sock, (char *)buf, (int)nBytes, 0);
            if (status != SOCKET_ERROR) {
                uint16_t i;
                nBytes = (uint16_t)status;
                for (i = 0U; i < nBytes; ++i) {
                    QS_RX_PUT(buf[i]);
                }
            }
        }
        QS_rxParse();  /* parse all the received bytes */

        nBytes = 1024U;
        QF_CRIT_ENTRY();
        block = QS_getBlock(&nBytes);
        QF_CRIT_EXIT();

        if (block != (uint8_t *)0) {
            send(l_sock, (char const *)block, nBytes, 0);
        }
        Sleep(20); /* sleep for xx milliseconds */
    }
    return (DWORD)0; /* return success */
}
/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[1024];  /* buffer for QS output */
    static uint8_t qsRxBuf[100]; /* buffer for QS receive channel */
    static WSADATA wsaData;
    char hostName[64];
    char const *src;
    char *dst;
    USHORT port = 6601; /* default QSPY server port */
    ULONG ioctl_opt = 1;
    struct sockaddr_in sockAddr;
    struct hostent *server;

    QS_initBuf(qsBuf, sizeof(qsBuf));
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    /* initialize Windows sockets */
    if (WSAStartup(MAKEWORD(2,0), &wsaData) == SOCKET_ERROR) {
        printf("Windows Sockets cannot be initialized.");
        return (uint8_t)0;
    }

    src = (arg != (void const *)0)
          ? (char const *)arg
          : "localhost";
    dst = hostName;
    while ((*src != '\0')
           && (*src != ':')
           && (dst < &hostName[sizeof(hostName)]))
    {
        *dst++ = *src++;
    }
    *dst = '\0';
    if (*src == ':') {
        port = (USHORT)strtoul(src + 1, NULL, 10);
    }

    l_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* TCP socket */
    if (l_sock == INVALID_SOCKET){
        printf("Socket cannot be created; error 0x%08X\n",
               WSAGetLastError());
        return (uint8_t)0; /* failure */
    }

    server = gethostbyname(hostName);
    if (server == NULL) {
        printf("QSpy host name %s cannot be resolved; error 0x%08X\n",
               hostName, WSAGetLastError());
        return (uint8_t)0;
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = AF_INET;
    memcpy(&sockAddr.sin_addr, server->h_addr, server->h_length);
    sockAddr.sin_port = htons(port);
    if (connect(l_sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr))
        == SOCKET_ERROR)
    {
        printf("Cannot connect to the QSPY server; error 0x%08X\n",
               WSAGetLastError());
        QS_EXIT();
        return (uint8_t)0; /* failure */
    }

    /* Set the socket to non-blocking mode. */
    if (ioctlsocket(l_sock, FIONBIO, &ioctl_opt) == SOCKET_ERROR) {
        printf("Socket configuration failed.\n"
               "Windows socket error 0x%08X.",
               WSAGetLastError());
        QS_EXIT();
        return (uint8_t)0; /* failure */
    }

    /* set up the QS filters... */
#if 0
    QS_GLB_FILTER(QS_QEP_STATE_ENTRY);
    QS_GLB_FILTER(QS_QEP_STATE_EXIT);
    QS_GLB_FILTER(QS_QEP_STATE_INIT);
    QS_GLB_FILTER(QS_QEP_INIT_TRAN);
    QS_GLB_FILTER(-QS_QEP_INTERN_TRAN);
    QS_GLB_FILTER(QS_QEP_TRAN);
    QS_GLB_FILTER(-QS_QEP_IGNORED);
    QS_GLB_FILTER(QS_QEP_DISPATCH);
    QS_GLB_FILTER(-QS_QEP_UNHANDLED);

    QS_GLB_FILTER(-QS_QF_ACTIVE_POST);
    QS_GLB_FILTER(-QS_QF_ACTIVE_POST_LIFO);
    QS_GLB_FILTER(-QS_QF_PUBLISH);
#endif
    QS_LOC_FILTER(TL_STAT);
    QS_LOC_FILTER(COMMAND_STAT);

    /* return the status of creating the idle thread */
    return (CreateThread(NULL, 1024, &idleThread, (void *)0, 0, NULL)
            != (HANDLE)0) ? (uint8_t)1 : (uint8_t)0;
}
/*..........................................................................*/
void QS_onCleanup(void) {
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
    WSACleanup();
}
/*..........................................................................*/
void QS_onFlush(void) {
    uint16_t nBytes = 1000;
    uint8_t const *block;
    while ((block = QS_getBlock(&nBytes)) != (uint8_t *)0) {
        send(l_sock, (char const *)block, nBytes, 0);
        nBytes = 1000;
    }
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
#endif

/*..........................................................................*/
/*! callback function to execute a uesr command (to be implemented in BSP) */
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
/*..........................................................................*/
void QSPY_onPrintLn(void) {
}
#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/
