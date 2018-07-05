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
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

#include "qwin_gui.h" /* QWIN GUI */
#include <stdio.h>    /* for _snprintf_s() */
#include <stdlib.h>
#include "resource.h" /* GUI resource IDs generated by the resource editior */

#ifdef Q_SPY
    #include "qspy.h"
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>  /* Win32 API for multithreading */
    #include <winsock2.h> /* for Windows network facilities */
#endif


Q_DEFINE_THIS_FILE

/* local variables ---------------------------------------------------------*/
static HINSTANCE l_hInst;   /* this application instance */
static HWND      l_hWnd;    /* main window handle */
static LPSTR     l_cmdLine; /* the command line string */

static SegmentDisplay   l_trafficlights[3];   /* SegmentDisplay to show Philo status */
static SegmentDisplay   l_pedLed;
static SegmentDisplay   l_timeDisplay;
static OwnerDrawnButton l_pedBtn;
// static uint8_t keyPressed = 0;

static const DWORD trafficlightsSeg[MaxIdentity][3] = {
    { IDC_RED_A, IDC_YELLOW_A, IDC_GREEN_A },
    { IDC_RED_B, IDC_YELLOW_B, IDC_GREEN_B },
    { IDC_RED_P, 0xFFFFu,      IDC_GREEN_P }
};
static const DWORD idLedPed[] = { IDC_LED_PED };
static const DWORD trafficlightsBmp[NO_LIGHT + 1] = {
    IDB_TL_OFF, IDB_TL_RED, IDB_TL_YELLOW, IDB_TL_GREEN
};
static const DWORD ledBmp[2] = {
    IDB_TL_LEDWHITE, IDB_TL_LEDRED
};

static const DWORD timeDisplaySeg[] = {
    IDC_DIGIT1, IDC_DIGIT2, IDC_DIGIT3
};
static const DWORD timeDisplayBmp[] = {
    IDB_SEG0, IDB_SEG1, IDB_SEG2, IDB_SEG3, IDB_SEG4,
    IDB_SEG5, IDB_SEG6, IDB_SEG7, IDB_SEG8, IDB_SEG9
};

#ifdef Q_SPY
    enum {
        TL_STAT = QS_USER,
        COMMAND_STAT
    };
    static SOCKET l_sock = INVALID_SOCKET;
    static uint8_t const l_clock_tick = 0x00U;
    static uint8_t const l_button     = 0x01U;
#endif

/* Local functions ---------------------------------------------------------*/
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam);

static void initSegmentDisplay(SegmentDisplay *sd, DWORD const seg[], DWORD const segCnt, DWORD const bmp[], DWORD const bmpCnt);

static void WriteTime(DWORD t);

/*..........................................................................*/
/* thread function for running the application main_gui() */
static DWORD WINAPI appThread(LPVOID par) {
    (void)par; /* unused parameter */
    return (DWORD)main_gui(); /* run the QF application */
}

/*--------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                   LPSTR cmdLine, int iCmdShow)
{
    HWND hWnd;
    MSG  msg;

    (void)hPrevInst; /* unused parameter */

    l_hInst   = hInst;   /* save the application instance */
    l_cmdLine = cmdLine; /* save the command line string */

    //AllocConsole();

    /* create the main custom dialog window */
    hWnd = CreateCustDialog(hInst, IDD_APPLICATION, NULL,
                            &WndProc, "QP_APP");
    ShowWindow(hWnd, iCmdShow);  /* show the main window */

    /* enter the message loop... */
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    //FreeConsole();
    BSP_terminate(0);

    return msg.wParam;
}

/*..........................................................................*/
static LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg,
                                WPARAM wParam, LPARAM lParam)
{
    UINT n;

    switch (iMsg) {

        /* Perform initialization upon cration of the main dialog window
        * NOTE: Any child-windows are NOT created yet at this time, so
        * the GetDlgItem() function can't be used (it will return NULL).
        */
        case WM_CREATE: {
            l_hWnd = hWnd; /* save the window handle */

            /* initialize the owner-drawn buttons...
            * NOTE: must be done *before* the first drawing of the buttons,
            * so WM_INITDIALOG is too late.
            */
            OwnerDrawnButton_init(&l_pedBtn, IDC_PED_BTN,
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_UP)),
                LoadBitmap(l_hInst, MAKEINTRESOURCE(IDB_BTN_DWN)),
                LoadCursor(NULL, IDC_HAND));
            return 0;
        }

        /* Perform initialization after all child windows have been created */
        case WM_INITDIALOG: {
            for (n = TrafficLightA; n < MaxIdentity; n++)
            {
                initSegmentDisplay(&l_trafficlights[n],
                                   trafficlightsSeg[n], sizeof(trafficlightsSeg[n])/sizeof(trafficlightsSeg[n][0]),
                                   trafficlightsBmp, sizeof(trafficlightsBmp)/sizeof(trafficlightsBmp[0]));
            }
            initSegmentDisplay(&l_pedLed,
                               idLedPed, sizeof(idLedPed)/sizeof(idLedPed[0]),
                               ledBmp, sizeof(ledBmp)/sizeof(ledBmp[0]));
            initSegmentDisplay(&l_timeDisplay,
                               timeDisplaySeg, sizeof(timeDisplaySeg)/sizeof(timeDisplaySeg[0]),
                               timeDisplayBmp, sizeof(timeDisplayBmp)/sizeof(timeDisplayBmp[0]));

            /* --> QP: spawn the application thread to run main_gui() */
            Q_ALLEGE(CreateThread(NULL, 0, &appThread, NULL, 0, NULL)
                     != (HANDLE)0);

            Q_ALLEGE(SetTimer(hWnd, 1000u, 1000u, (TIMERPROC)NULL) != 0);
            return 0;
        }

        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }

        /* commands from regular buttons and menus... */
        case WM_COMMAND: {
            SetFocus(hWnd);
            switch (wParam) {
                case IDOK:
                case IDCANCEL: {
                    BSP_terminate(0);
                    KillTimer(hWnd, 1000u);
                    MessageBox(hWnd, "Bye bye...\r\nLooking forward to seeing you again", "TrafficLight", MB_OK);
                    PostQuitMessage(0);
                    break;
                case IDC_PED_BTN:
                	BSP_publishBtnEvt(); /* publish to all subscribers */
                    break;
                case IDC_EMRG_BTN:
                	{
                		static enum {
                			SetEm = 0,
							RelEm
                		} state = SetEm;
						BSP_publishEmergencyEvt(); /* publish to all subscribers */
						switch (state)
						{
							case RelEm:
								state = SetEm;
								SetDlgItemText(hWnd, IDC_EMRG_BTN, "Emergency");
								break;
							case SetEm:
								state = RelEm;
								SetDlgItemText(hWnd, IDC_EMRG_BTN, "Release");
								break;
							default:
								break;
						}

                	}
                    break;
                default:
                    break;
                }
            }
            return 0;
        }

        case WM_TIMER: {
            static DWORD timCount = 0;

            timCount++;
            if (timCount >= 1000)
                timCount = 0;
            WriteTime(timCount);
            Q_ALLEGE(SetTimer(hWnd, 1000, 1000, (TIMERPROC)NULL) != 0);
            return 0;
        }

        /* owner-drawn buttons... */
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (IDC_PED_BTN == pdis->CtlID) switch (OwnerDrawnButton_draw(&l_pedBtn,pdis))
			{
				case BTN_DEPRESSED:
					break;
				case BTN_RELEASED:
					BSP_publishBtnEvt();
					break;
				default:
					break;
			}
            return 0;
        }

        /* mouse input... */
        case WM_MOUSEWHEEL: {
            return 0;
        }

        /* keyboard input... */
        case WM_KEYDOWN: {
            return 0;
        }

        /* character input... */
        case WM_CHAR: {
            return 0;
        }
    }
    return DefWindowProc(hWnd, iMsg, wParam, lParam) ;
}
/*..........................................................................*/
static void initSegmentDisplay(SegmentDisplay *sd, DWORD const seg[], DWORD const segCnt, DWORD const bmp[], DWORD const bmpCnt)
{
    DWORD n;

    SegmentDisplay_init(sd, segCnt, bmpCnt);
    for (n = 0; n < segCnt; n++)
    {
        SegmentDisplay_initSegment(sd, n, seg[n]);
    }
    for (n = 0; n < bmpCnt; n++)
    {
        SegmentDisplay_initBitmap(sd, n, LoadBitmap(l_hInst, MAKEINTRESOURCE(bmp[n])));
    }
}
/*..........................................................................*/
static void WriteTime(DWORD t)
{
    DWORD t100, t010, t001;

    t100 = t / 100;
    t010 = (t % 100) / 10;
    t001 = ((t %100) % 10);

    SegmentDisplay_setSegment(&l_timeDisplay, 0, t001);
    SegmentDisplay_setSegment(&l_timeDisplay, 1, t010);
    SegmentDisplay_setSegment(&l_timeDisplay, 2, t100);
}
/*..........................................................................*/
void QF_onStartup(void) {
    QF_setTickRate(BSP_TICKS_PER_SEC); /* set the desired tick rate */
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QF_onClockTick(void) {
    static QEvt const tickEvt = { TIME_TICK_SIG, 0U, 0U };

    QACTIVE_POST(the_Ticker0, 0, &l_clock_tick); /* post to Ticker0 */
    QF_PUBLISH(&tickEvt, &l_clock_tick); /* publish to all subscribers */
}

/*..........................................................................*/
void Q_onAssert(char const * const module, int_t loc) {
    char message[80];
    QF_stop(); /* stop ticking */
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */
    SNPRINTF_S(message, Q_DIM(message) - 1,
               "Assertion failed in module %s location %d", module, loc);
    MessageBox(l_hWnd, message, "!!! ASSERTION !!!",
               MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

    PostQuitMessage(-1);
}

/*..........................................................................*/
void BSP_init(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

#ifdef Q_SPY
    if (QS_INIT(l_cmdLine) == (uint8_t)0) { /* QS initialization failed? */
        MessageBox(l_hWnd,
                   "Cannot connect to QSPY via TCP/IP\n"
                   "Please make sure that 'qspy -t' is running",
                   "QS_INIT() Error",
                   MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
    }
#endif
    QS_OBJ_DICTIONARY(&l_clock_tick); /* must be called *after* QF_init() */
    QS_OBJ_DICTIONARY(&l_button); /* must be called *after* QF_init() */
    QS_USR_DICTIONARY(TL_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);
}
/*..........................................................................*/
void BSP_terminate(int16_t result)
{
    UINT n;
    (void)result;
#ifdef Q_SPY
    if (l_sock != INVALID_SOCKET) {
        closesocket(l_sock);
        l_sock = INVALID_SOCKET;
    }
#endif
    QF_stop(); /* stop the main QF application and the ticker thread */

    /* cleanup all QWIN resources... */
    for (n = 0; n < 3; n++)
        SegmentDisplay_xtor(&l_trafficlights[n]);     /* cleanup the l_philos resources */
    SegmentDisplay_xtor(&l_pedLed);
    SegmentDisplay_xtor(&l_timeDisplay);

    /* end the main dialog */
    EndDialog(l_hWnd, result);
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, eTLlight_t light)
{
    eTLlight_t  n;

    for (n = RED; n < NO_LIGHT; n++)
    {
        if (0xFFFFu != trafficlightsSeg[id][n])
        {
            UINT lidx = 0U;

            if (light == n)
            {
                lidx = 1u + (UINT)light;
            }
            SegmentDisplay_setSegment(&l_trafficlights[id], n, lidx);
        }
    }

#if 0
    QS_BEGIN(T, AO_Philo[n]) /* application-specific record begin */
        QS_U8(1, n);  /* Philosopher number */
        QS_STR(stat); /* Philosopher status */
    QS_END()
#endif
}
/*..........................................................................*/
void BSP_setPedLed(uint16_t status)
{
    SegmentDisplay_setSegment(&l_pedLed, 0U, status ? 1 : 0);
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
/*..........................................................................*/
#if 0
uint16_t BSP_getButton(void)
{
    uint16_t retCode;

    retCode = keyPressed;
    keyPressed = 0;

    return retCode;
}
#endif

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
        QF_CRIT_ENTRY(dummy);
        block = QS_getBlock(&nBytes);
        QF_CRIT_EXIT(dummy);

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
    QS_FILTER_OFF(QS_ALL_RECORDS);

    QS_FILTER_ON(QS_SM_RECORDS);
    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
    QS_FILTER_OFF(QS_QEP_IGNORED);
    QS_FILTER_OFF(QS_QEP_UNHANDLED);

    // QS_FILTER_ON(TL_STAT);
    QS_FILTER_ON(COMMAND_STAT);

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
    fputs(QSPY_line, stdout);
    fputc('\n', stdout);
}
#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/