/*****************************************************************************
* Product: DPP example, POSIX
* Last updated for version 6.2.0
* Last updated on  2018-03-10
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2018 Quantum Leaps, LLC. All rights reserved.
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
* https://www.state-machine.com
* mailto:info@state-machine.com
*****************************************************************************/
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>      /* for memcpy() and memset() */
#include <unistd.h>

#if defined(__WIN32__)
#   include <windows.h>
#else
#   include <sys/select.h>
#   include <termios.h>
#endif

Q_DEFINE_THIS_FILE

/* Local objects -----------------------------------------------------------*/
#if defined(__WIN32__)
#define close(x)    closesocket(x)
static DWORD ConsoleMode = 0;
static HANDLE hStdin;
#else
static struct termios l_tsav; /* structure with saved terminal attributes */
// static uint8_t keyPressed = 0;
#endif

#if defined Q_SPY
static uint8_t l_running;
#endif

static void setupConsole(void)
{
#if defined(__WIN32__)
    DWORD mode = 0;
    hStdin = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(hStdin, &ConsoleMode);
    mode = ConsoleMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hStdin, mode);
#else
    struct termios tio;   /* modified terminal attributes */

    tcgetattr(0, &l_tsav); /* save the current terminal attributes */
    tcgetattr(0, &tio);    /* obtain the current terminal attributes */
    tio.c_lflag &= ~(ICANON | ECHO); /* disable the canonical mode & echo */
    tcsetattr(0, TCSANOW, &tio);     /* set the new attributes */
#endif
}

static void shutdownConsole(void)
{
#ifdef __WIN32__
    SetConsoleMode(hStdin, ConsoleMode);
#else
    tcsetattr(0, TCSANOW, &l_tsav);/* restore the saved terminal attributes */
#endif
}
/*..........................................................................*/
static int readConsoleChar(void)
{
    char ch = 0;
#if defined(__WIN32__)
    INPUT_RECORD inp;
    DWORD read;

    if(PeekConsoleInput(hStdin, &inp, 1, &read))
    {
        if(KEY_EVENT == inp.EventType)
        {
            char c;

            if(ReadConsole(hStdin, &c, 1, &read, NULL))
            {
                ch = c;
            }
        }
    }
#else
    struct timeval timeout = { 0, 0 };  /* timeout for select() */
    fd_set con; /* FD set representing the console */

    FD_ZERO(&con);
    FD_SET(0, &con);
    /* check if a console input is available, returns immediately */
    if (0 != select(1, &con, 0, 0, &timeout)) { /* any descriptor set? */
        read(0, &ch, 1);
    }
#endif

    return ch;
}

/* main()        ===========================================================*/
#if !defined Q_UTEST
int main(int argc, char *argv[])
{
    BSP_HW_init();
    QF_init();    /* initialize the framework and the underlying RT kernel */
    BSP_init(argc, argv); /* initialize the Board Support Package */

    return tlMain();
}
#endif

/* BSP functions ===========================================================*/
void BSP_HW_init(void) {

}
/*..........................................................................*/
void BSP_init(int argc, char **argv) {

#ifndef Q_SPY
    (void)argc; /* unused parameter */
    (void)argv; /* unused parameter */
#endif

    printf("Traffic Light example"
           "\nQP %s\n"
           "Press 'p' for pedestrian signal\n"
           "Press ESC to quit...\n",
           QP_versionStr);

    Q_ALLEGE(QS_INIT((argc > 1) ? argv[1] : ""));
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
void BSP_terminate(int16_t result) {
    (void)result;
#ifdef Q_SPY
    l_running = (uint8_t)0; /* stop the QS output thread */
#endif
    QF_stop();
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, uint8_t light)
{

    static char * const strIdentity[MaxIdentity] = {
        "Traffic Light A", "Traffic Light B", "Pedestrian Light"
    };
    static char * const strColor[] = {
        "NONW", "RED", "YELLOW", "RED/YELLOW", "GREEN"
    };

    // Q_ASSERT(light <= GREEN && id < MaxIdentity);
    printf("%s is %s\n", strIdentity[id], strColor[light]);

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
    (void)status;
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

/* QF callbacks ============================================================*/
void QF_onStartup(void) {
    QF_consoleSetup();
    QF_setTickRate(BSP_TICKS_PER_SEC, 30); /* set the desired tick rate */
}
/*..........................................................................*/
void QF_onCleanup(void) {
    printf("\nBye! Bye!\n");
    QF_consoleCleanup();
    QS_EXIT(); /* perform the QS cleanup */
}
/*..........................................................................*/
void QF_onClockTick(void) {
    static QEvt const buttonEvt = { BUTTON_SIG, 0U, QEVT_MARKER };

    QTICKER_TRIG(the_Ticker0, &l_SysTick_Handler); /* post to Ticker0 */

    switch (QF_consoleGetKey())
    {
        case '\33':  // ESC pressed
        case 'q':    // q pressed
        case 'Q':    // Q pressed
            BSP_terminate(0);
            break;
        case 'P':
        case 'p':
            printf("Pedestrian button pressed...\n");
            QF_PUBLISH(&buttonEvt, &l_Button_Handler); /* publish to all subscribers */
            // keyPressed = 1;
            break;
        default:
            break;
    }
}
/*..........................................................................*/
void Q_onAssert(char const *module, int loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    printf("Assertion failed in %s:%d", module, loc);
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */
    exit(-1);
}

// QSPY ======================================================================
#ifdef Q_SPY

/*
* NOTE:
* The QS target-resident component is implemented in two different ways:
* 1. Output to the TCP/IP socket, which requires a separate QSPY host
*    application running; or
* 2. Direct linking with the QSPY host application to perform direct output
*    to the console from the running application. (This option requires
*    the QSPY source code, which is part of the QTools collection).
*
* The two options are selected by the following QS_IMPL_OPTION macro.
* Please set the value of this macro to either 1 or 2:
*/
#include "qs.h"

/*--------------------------------------------------------------------------*/
/*
* 1. Output to the TCP/IP socket, which requires a separate QSPY host
*    application running. This option does not link to the QSPY code.
*/
#if !defined(__WIN32__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef int SOCKET;
#endif
#include <errno.h>
#include <time.h>

#define QS_TX_SIZE    (4*1024)
#define QS_RX_SIZE    1024
#define QS_IMEOUT_MS  100
#if !defined(INVALID_SOCKET)
#define INVALID_SOCKET -1
#endif
#if !defined(QSPY_ERROR)
enum {
    QSPY_ERROR,
    QSPY_SUCCESS
};
#endif

/* local variables .........................................................*/
static void *idleThread(void *par); // the expected P-Thread signature
static SOCKET l_sock = INVALID_SOCKET;

/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    static uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    char hostName[64];
    char const *src;
    char *dst;
    uint8_t ret;

    uint16_t port = 6601;  /* default QSPY server port */
    struct sockaddr_in sockAddr;
    struct hostent *server;
#if defined(__WIN32__)
    WSADATA wsaData;
    ULONG sockopt_bool = 1;
#else
    int sockopt_bool = 1;
#endif

    QS_initBuf(qsBuf, sizeof(qsBuf));
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

#if defined(__WIN32__)
    if (WSAStartup(MAKEWORD(2,0), &wsaData) == SOCKET_ERROR) {
        printf("Windows Sockets cannot be initialized.");
        return (uint8_t)0;
    }
#endif

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
        port = (uint16_t)strtoul(src + 1, NULL, 10);
    }

    printf("<TARGET> Connecting to QSPY on Host=%s:%d...\n",
           hostName, port);

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
    sockopt_bool = 1;
    if (ioctlsocket(l_sock, FIONBIO, &sockopt_bool) == SOCKET_ERROR) {
        printf("Socket configuration failed.\n"
               "Windows socket error 0x%08X.",
               WSAGetLastError());
        QS_EXIT();
        return (uint8_t)0; /* failure */
    }
    printf("<TARGET> Connected  to QSPY on Host=%s:%d\n",
           hostName, port);

// create QSPY thread
#if defined(__WIN32__)
    /* return the status of creating the idle thread */
    ret = (uint8_t)((CreateThread(NULL, 1024, (LPTHREAD_START_ROUTINE)(&idleThread), (void *)0, 0, NULL)
            != (HANDLE)0) ? 1 : 0);
#else
    pthread_attr_t attr;
    struct sched_param param;
    pthread_t idle;
    int ret;

    // SCHED_FIFO corresponds to real-time preemptive priority-based
    // scheduler.
    // NOTE: This scheduling policy requires the superuser priviledges

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = sched_get_priority_min(SCHED_FIFO);

    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret (pthread_create(&idle, &attr, &idleThread, 0) != 0);
    if ret {
        // Creating the p-thread with the SCHED_FIFO policy failed.
        // Most probably this application has no superuser privileges,
        // so we just fall back to the default SCHED_OTHER policy
        // and priority 0.
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        if (pthread_create(&idle, &attr, &idleThread, 0) == 0) {
            return false;
        }


    }
    pthread_attr_destroy(&attr);
#endif
    return ret;
}
/*..........................................................................*/
void QS_onCleanup(void) {
#if defined(__WIN32__)
    WSACleanup();
#endif
    l_running = (uint8_t)0;
    if (l_sock != INVALID_SOCKET) {
        close(l_sock);
        l_sock = INVALID_SOCKET;
    }
    //printf("<TARGET> Disconnected from QSPY via TCP/IP\n");
}
/*..........................................................................*/
void QS_onFlush(void) {
    if (l_sock != INVALID_SOCKET) {  // socket initialized?
        uint16_t nBytes = QS_TX_SIZE;
        uint8_t const *data;
        while ((data = QS_getBlock(&nBytes)) != (uint8_t *)0) {
            send(l_sock, (char const *)data, nBytes, 0);
            nBytes = QS_TX_SIZE;
        }
    }
}
/*..........................................................................*/
static void *idleThread(void *par) { // the expected P-Thread signature
    fd_set readSet;
    FD_ZERO(&readSet);

    (void)par; /* unused parameter */
    l_running = (uint8_t)1;
    while (l_running) {
        static struct timeval timeout = {
            (long)0, (long)(QS_IMEOUT_MS * 1000)
        };
        int nrec;
        uint16_t nBytes;
        uint8_t const *block;

        FD_SET(l_sock, &readSet);   /* the socket */

        /* selective, timed blocking on the TCP/IP socket... */
        timeout.tv_usec = (long)(QS_IMEOUT_MS * 1000);
        nrec = select(l_sock + 1, &readSet,
                      (fd_set *)0, (fd_set *)0, &timeout);
        if (nrec < 0) {
            printf("    <CONS> ERROR    select() errno=%d\n", errno);
            QS_onCleanup();
            exit(-2);
        }
        else if (nrec > 0) {
            if (FD_ISSET(l_sock, &readSet)) { /* socket ready to read? */
                uint8_t buf[QS_RX_SIZE];
                int status = recv(l_sock, (char *)buf, (int)sizeof(buf), 0);
                while (status > 0) { /* any data received? */
                    uint8_t *pb;
                    int i = (int)QS_rxGetNfree();
                    if (i > status) {
                        i = status;
                    }
                    status -= i;
                    /* reorder the received bytes into QS-RX buffer */
                    for (pb = &buf[0]; i > 0; --i, ++pb) {
                        QS_RX_PUT(*pb);
                    }
                    QS_rxParse(); /* parse all n-bytes of data */
                }
            }
        }

        nBytes = QS_TX_SIZE;
        //QF_CRIT_ENTRY(dummy);
        block = QS_getBlock(&nBytes);
        //QF_CRIT_EXIT(dummy);

        if (block != (uint8_t *)0) {
            send(l_sock, (char const *)block, nBytes, 0);
        }
    }

    return 0; // return success
}

//............................................................................
QSTimeCtr QS_onGetTime(void) {
    return (QSTimeCtr)clock(); // see NOTE01
}
//............................................................................
void QS_onReset(void) {
    QS_onCleanup();
    exit(0);
}
//............................................................................
//! callback function to execute a user command (to be implemented in BSP)
void QS_onCommand(uint8_t cmdId, uint32_t param1,
                   uint32_t param2, uint32_t param3)
{
    (void)cmdId;  // unused parameter
    (void)param1; // unused parameter
    (void)param2; // unused parameter
    (void)param3; // unused parameter
    //TBD
}
/*..........................................................................*/
void QSPY_onPrintLn(void) {
}

//****************************************************************************
// NOTE01:
// clock() is the most portable facility, but might not provide the desired
// granularity. Other, less-portable alternatives are clock_gettime(),
// rdtsc(), or gettimeofday().
//

#endif // Q_SPY
