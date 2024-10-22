/*****************************************************************************
* Product: DPP example, NUCLEO-L053R8 board, preemptive QK kernel
* Last Updated for Version: 5.9.9
* Date of the Last Update:  2017-10-09
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
#include "RP2040.h"
#include "pico/stdlib.h"
#include "hardware/exception.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

/* add other drivers if necessary... */

Q_DEFINE_THIS_FILE

// plausibility checks
#if !defined(KERNEL_QV) && !defined(KERNEL_QK)
#error "No real time kernel (neither KERNEL_QV nor KERNEL_QK) is defined. Aborting!"
#endif

#if !(__ARM_ARCH == 6 || __ARM_ARCH == 7)
#error "unknown ARM architecture found. Aborting!"
#endif

void SysTick_Handler(void);
static void readUserButtons(void);

#ifdef Q_SPY
void UART0_IRQ_Handler(void);
#endif

/* Local-scope defines -----------------------------------------------------*/
/* LED pins available on the board (just one user LED LD2--Green on PA.5) */
#define LED_LD2  PICO_DEFAULT_LED_PIN

/* Button pins available on the board (just one user Button B1 on PC.13) */
#define READ_UserBtn()    (!gpio_get(PIN_BUTTON_Pin))

#define ARRAY_SIZE(x)    (sizeof(x)/sizeof((x)[0]))

#define PICO_FIRST_PORT  2

// static uint8_t keyPressed = 0;

#ifdef Q_SPY
    QSTimeCtr QS_tickTime_;
    QSTimeCtr QS_tickPeriod_;
#endif

enum {
    PIN_A_RED_Pin = PICO_FIRST_PORT, // 2
    PIN_A_YLW_Pin,  //  3
    PIN_A_GRN_Pin,  //  4
    
    PIN_B_RED_Pin,  //  5
    PIN_B_YLW_Pin,  //  6
    PIN_B_GRN_Pin,  //  7
    
    PIN_P_RED_Pin, //  8
    PIN_P_GRN_Pin, //  9
    
    PIN_BUTTON_Pin, // 10

    PIN_EMERGENCY_Pin, // 11

    PIN_UART0_TX,   // 12
    PIN_UART0_RX,   // 13

    PIN_LAST_GPIO   // 14
};
 
static const uint8_t outPins[] = {
    PIN_A_RED_Pin, // 2
    PIN_A_YLW_Pin, //  3
    PIN_A_GRN_Pin, //  4
    
    PIN_B_RED_Pin, //  5
    PIN_B_YLW_Pin, //  6
    PIN_B_GRN_Pin, //  7
    
    PIN_P_RED_Pin, //  8
    PIN_P_GRN_Pin, //  9

};

static const uint8_t inPins[] ={
    PIN_BUTTON_Pin, // 10
    PIN_EMERGENCY_Pin // 10
};

/* ISRs used in the application ==========================================*/
#ifdef Q_SPY
void UART0_IRQ_Handler(void)
{
    // is RX register NOT empty?
    while (uart_is_readable(uart0)) {
        uint32_t b = uart_getc(uart0);
        QS_RX_PUT(b);
    }

#if defined KERNEL_QK
    QK_ARM_ERRATUM_838869();
#elif defined KERLEL_QV
    QV_ARM_ERRATUM_838869();
#else
#   warning "no valid KERNEL defined. Are you sure?...
#endif
}
#endif

void SysTick_Handler(void) {   /* system clock tick ISR */
#ifdef KERNEL_QK
    QK_ISR_ENTRY();   /* inform QK about entering an ISR */
#endif

#ifdef Q_SPY
    {
        (void)SysTick->CTRL; /* clear CTRL_COUNTFLAG */
        QS_tickTime_ += QS_tickPeriod_; /* account for the clock rollover */
    }
#endif

    QTICKER_TRIG(the_Ticker0, &l_SysTick_Handler); /* post to Ticker0 */

    readUserButtons();

#ifdef KERNEL_QK
    QK_ISR_EXIT();             /* inform QK about exiting an ISR */
#endif
}

static void readUserButtons(void)
{
    /* get state of the user button */
    /* Perform the debouncing of buttons. The algorithm for debouncing
    * adapted from the book "Embedded Systems Dictionary" by Jack Ganssle
    * and Michael Barr, page 71.
    */
    static uint8_t debounceBuffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    static uint8_t debounceIndex = 0;
    static uint8_t debouncedButtons = 0;
    static uint8_t chgButtons = 0;
    static uint32_t time2Min = 120ul * BSP_TICKS_PER_SEC;
    uint8_t currentButtons = 0;
    uint8_t currentAND = 0xFFu;
    uint8_t currentOR = 0x00u;
    uint8_t n;

    if (0 < time2Min)
    {
    	time2Min--;
    }

    chgButtons = debouncedButtons;
    if (READ_UserBtn())
        currentButtons |= 0x01u;
    debounceBuffer[debounceIndex++] = currentButtons;
    debounceIndex &= (0x08 - 1);
    for (n = 0; n < ARRAY_SIZE(debounceBuffer); n++)
    {
        currentAND &= debounceBuffer[n];
        currentOR |= debounceBuffer[n];
    }
    debouncedButtons |= currentAND;
    debouncedButtons &= currentOR;
    chgButtons ^= debouncedButtons;
    if ( (debouncedButtons & chgButtons & 0x01u) || (0 == time2Min) )
    {
    	static const QEvt buttonEvt =  QEVT_INITIALIZER(BUTTON_SIG);

        QF_PUBLISH(&buttonEvt, &l_Button_Handler); /* publish to all subscribers */
    	time2Min = 120ul * BSP_TICKS_PER_SEC;
    }
    else if (debouncedButtons & chgButtons & 0x02u)
    {
    	static const QEvt emergencyEvt = QEVT_INITIALIZER(EMERGENCY_SIG);
    	static const QEvt releaseEvt = QEVT_INITIALIZER(EM_RELEASE_SIG);
        static QEvt *e;
        
        e = (debouncedButtons & 0x02u) ? &emergencyEvt : &releaseEvt;
        QF_PUBLISH(e, &l_Button_Handler); /* publish to all subscribers */
    }
}

/* main()        ===========================================================*/
int main(void)
{
    BSP_HW_init();
    QF_init();
    BSP_init(0, NULL);
    
    return tlMain(); 
}

/* BSP functions ===========================================================*/
void BSP_HW_init(void)
{
    // initialize the Raspberry Pi Pico
    set_sys_clock_48mhz();

}

/*..........................................................................*/
void BSP_init(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    SystemCoreClockUpdate();

    for (uint8_t i = 0; i < sizeof(outPins) / sizeof(outPins[0]); i++)
    {
        uint8_t gpio = outPins[i];
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_OUT);
        gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_8MA);
    }
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // We are using the button to pull down to 0v when pressed, so ensure that when
    // unpressed, it uses internal pull ups. Otherwise when unpressed, the input will
    // be floating.
    for (uint8_t i = 0; i < sizeof(inPins) / sizeof(inPins[0]); i++)
    {
        uint8_t gpio = inPins[i];
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
    }

   /* initialize the QS software tracing... */
    if (QS_INIT((void *)0) == 0U) {
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_Button_Handler);
}
/*..........................................................................*/
void BSP_terminate(int16_t result) {
    (void)result;
}
/*..........................................................................*/
void BSP_wait4SW1(void) {
    while (READ_UserBtn() != 0U) {
        gpio_put(LED_LD2, 1);  /* turn LED2 on  */
        gpio_put(LED_LD2, 0);  /* turn LED2 off */
    }
}
/*..........................................................................*/
void BSP_ledOn(void) {
    /* turn LED2 on  */
    gpio_put(LED_LD2, 1);
}
/*..........................................................................*/
void BSP_ledOff(void) {
    /* turn LED2 off */
    gpio_put(LED_LD2, 0);
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, uint8_t light)
{
    uint32_t offMask, onMask;
    static uint8_t shiftmasks[MaxIdentity] = {PIN_A_RED_Pin, PIN_B_RED_Pin, PIN_P_RED_Pin};

    Q_ASSERT(light <= GREEN && id < MaxIdentity);
    offMask = ((id == PedestrianLight) ? 0x03 : 0x07u) << shiftmasks[id];
    switch(light)
    {
        case RED:
        case YELLOW:
        case RED_YELLOW:
            onMask = light << shiftmasks[id];
            break;
        case GREEN:
            onMask = (id == PedestrianLight ? 0x02 : 0x04u) << shiftmasks[id];
            break;
        default:
            onMask = 0;
            break;
    }

    gpio_clr_mask(offMask);
    gpio_set_mask(onMask);
}

/*..........................................................................*/
void BSP_setPedLed(uint16_t status)
{
    if (status)
        BSP_ledOn();
    else
        BSP_ledOff();
}

/* QF callbacks ============================================================*/
void QF_onStartup(void) {
    /* assing all priority bits for preemption-prio. and none to sub-prio. */
    NVIC_SetPriorityGrouping(0U);

    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set priorities of ALL ISRs used in the system, see NOTE1
    *
    * !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    */
    NVIC_SetPriority(SysTick_IRQn, QF_AWARE_ISR_CMSIS_PRI);
    /* ... */

    /* enable IRQs... */
#ifdef Q_SPY
    irq_set_enabled(UART0_IRQ, true);
#endif
   gpio_put(PICO_DEFAULT_LED_PIN, 1);
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
#if defined KERNEL_QK
void QK_onIdle(void) { /* called with interrupts enabled */
#elif defined KERNEL_QV
void QV_onIdle(void) { /* called with interrupts enabled */
#endif
    /* toggle an LED on and then off (not enough LEDs, see NOTE01) */
    QF_INT_DISABLE();
    //GPIOA->BSRR |= (LED_LD2);        /* turn LED[n] on  */
    //GPIOA->BSRR |= (LED_LD2 << 16);  /* turn LED[n] off */
    QF_INT_ENABLE();

#ifdef Q_SPY
    {
        uint16_t b;

        QF_INT_DISABLE();
        b = QS_getByte();
        QF_INT_ENABLE();

        uart_putc_raw(uart0, (b & 0x00FFu));
    }
#elif defined NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular Cortex-M3 MCU.
    */
    /* !!!CAUTION!!!
    * The WFI instruction stops the CPU clock, which unfortunately disables
    * the JTAG port, so the ST-Link debugger can no longer connect to the
    * board. For that reason, the call to __WFI() has to be used with CAUTION.
    *
    * NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    * reset the board, then connect with ST-Link Utilities and erase the part.
    * The trick with BOOT(0) is it gets the part to run the System Loader
    * instead of your broken code. When done disconnect BOOT0, and start over.
    */
    __WFI(); /* Wait-For-Interrupt */
#endif
}

/*..........................................................................*/
void Q_onAssert(char const *module, int loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, (uint32_t)10000U); /* report assertion to QS */

#ifndef NDEBUG
    BSP_wait4SW1();
#endif
    NVIC_SystemReset();
    
    while(1)
    {
        // should never happen
    }
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY
/*..........................................................................*/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[2*1024]; /* buffer for Quantum Spy */

    (void)arg; /* avoid the "unused parameter" compiler warning */
    QS_initBuf(qsBuf, sizeof(qsBuf));

    // UART
    uart_init(uart0, 115200);
    uart_set_translate_crlf(uart0, false);
    uart_set_irq_enables(uart0, true, false);
    gpio_set_function(PIN_UART0_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART0_RX, GPIO_FUNC_UART);

    QS_tickTime_ = QS_tickPeriod_; /* to start the timestamp at zero */

    /* setup the QS filters... */
    QS_GLB_FILTER(QS_QEP_STATE_ENTRY);
    QS_GLB_FILTER(QS_QEP_STATE_EXIT);
    QS_GLB_FILTER(QS_QEP_STATE_INIT);
    QS_GLB_FILTER(QS_QEP_INIT_TRAN);
    QS_GLB_FILTER(QS_QEP_INTERN_TRAN);
    QS_GLB_FILTER(QS_QEP_TRAN);
    QS_GLB_FILTER(QS_QEP_IGNORED);
    QS_GLB_FILTER(QS_QEP_DISPATCH);
    QS_GLB_FILTER(QS_QEP_UNHANDLED);

    QS_LOC_FILTER(PHILO_STAT);

    return (uint8_t)1; /* return success */
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
QSTimeCtr QS_onGetTime(void) { /* NOTE: invoked with interrupts DISABLED */
    if ((SysTick->CTRL & 0x00010000) == 0) {  /* COUNT no set? */
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else { /* the rollover occured, but the SysTick_ISR did not run yet */
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
/*..........................................................................*/
void QS_onFlush(void) {
    uint16_t b;

    QF_INT_DISABLE();
    while ((b = QS_getByte()) != QS_EOD) {    /* while not End-Of-Data... */
        QF_INT_ENABLE();
        uart_putc_raw(uart0, (b & 0x00FFu));
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
#endif /* Q_SPY */

/*****************************************************************************
* NOTE00:
* The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
* ISR priority that is disabled by the QF framework. The value is suitable
* for the NVIC_SetPriority() CMSIS function.
*
* Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
* with the numerical values of priorities equal or higher than
* QF_AWARE_ISR_CMSIS_PRI) are allowed to call any QF services. These ISRs
* are "QF-aware".
*
* Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
* level (i.e., with the numerical values of priorities less than
* QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
* Such "QF-unaware" ISRs cannot call any QF services. The only mechanism
* by which a "QF-unaware" ISR can communicate with the QF framework is by
* triggering a "QF-aware" ISR, which can post/publish events.
*
* NOTE01:
* Usually, one of the LEDs is used to visualize the idle loop activity.
* However, the board has not enough LEDs (only one, actually), so this
* feature is disabled.
*/
