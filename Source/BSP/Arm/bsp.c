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
#include <stm32f0xx_hal.h>
#include <main.h>  /* CMSIS-compliant header file for the MCU used */
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"


/* add other drivers if necessary... */

Q_DEFINE_THIS_FILE

// plausibility checks
#if !defined(KERNEL_QV) && !defined(KERNEL_QK)
#error "No real time kernel (neither KERNEL_QV nor KERNEL_QK) is defined. Aborting!"
#endif

#if !(__ARM_ARCH == 6 || __ARM_ARCH == 7)
#error "unknown ARM architecture found. Aborting!"
#endif

extern void hal_init(void);

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
* DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
*/
enum KernelAwareISRs {
    GPIOPORTA_PRIO = QF_AWARE_ISR_CMSIS_PRI, /* see NOTE00 */
    SYSTICK_PRIO,
    /* ... */
    MAX_KERNEL_AWARE_CMSIS_PRI /* keep always last */
};
/* "kernel-aware" interrupts should not overlap the PendSV priority */
Q_ASSERT_COMPILE(MAX_KERNEL_AWARE_CMSIS_PRI <= (0xFF >>(8-__NVIC_PRIO_BITS)));

void HAL_SYSTICK_Callback(void);
static void readUserButtons(void);

/* Local-scope defines -----------------------------------------------------*/
/* LED pins available on the board (just one user LED LD2--Green on PA.5) */
#define LED_LD2  (1U << 5)

/* Button pins available on the board (just one user Button B1 on PC.13) */
#define BTN_B1   (1U << 13)

#define READ_UserBtn()    (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) ? 0 : 1)

#define ARRAY_SIZE(x)    (sizeof(x)/sizeof((x)[0]))

// static uint8_t keyPressed = 0;

#ifdef Q_SPY
    QSTimeCtr QS_tickTime_;
    QSTimeCtr QS_tickPeriod_;

    /* event-source identifiers used for tracing */
    static uint8_t const l_SysTick_Handler = 0U;
    static uint8_t const l_button          = 1U;

    enum AppRecords { /* application-specific trace records */
        PHILO_STAT = QS_USER
    };

#endif

/* ISRs used in the application ==========================================*/
void HAL_SYSTICK_Callback(void) {   /* system clock tick ISR */
#ifdef KERNEL_QK
    QK_ISR_ENTRY();   /* inform QK about entering an ISR */
#endif

#ifdef Q_SPY
    {
        (void)SysTick->CTRL; /* clear CTRL_COUNTFLAG */
        QS_tickTime_ += QS_tickPeriod_; /* account for the clock rollover */
    }
#endif

    //QF_TICK_X(0U, &l_SysTick_Handler); /* process time events for rate 0 */
    QACTIVE_POST(the_Ticker0, 0, &l_SysTick_Handler); /* post to Ticker0 */

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
    if ( (debouncedButtons & chgButtons & 0x01) || (0 == time2Min) )
    {
    	static QEvt const buttonEvt = { BUTTON_SIG, 0U, 0U };

        QF_PUBLISH(&buttonEvt, &l_button); /* publish to all subscribers */
    	time2Min = 120ul * BSP_TICKS_PER_SEC;
    }
    else if (debouncedButtons & chgButtons & 0x02)
    {
    	static QEvt buttonEvt = { EM_RELEASE_SIG, 0U, 0U };

        buttonEvt.sig = ((buttonEvt.sig == EMERGENCY_SIG) ? EM_RELEASE_SIG : EMERGENCY_SIG);
        QF_PUBLISH(&buttonEvt, &l_button); /* publish to all subscribers */
    }
}

/* BSP functions ===========================================================*/
void BSP_HW_init(void) {
    /* NOTE: SystemInit() has been already called from the startup code
    *  but SystemCoreClock needs to be updated
    */
    hal_init();
    SystemCoreClockUpdate();
}
/*..........................................................................*/
void BSP_init(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

#if 0
    /* enable GPIOA clock port for the LED LD2 */
    RCC->IOPENR |= (1U << 0);

    /* configure LED (PA.5) pin as push-pull output, no pull-up, pull-down */
    GPIOA->MODER   &= ~((3U << 2*5));
    GPIOA->MODER   |=  ((1U << 2*5));
    GPIOA->OTYPER  &= ~((1U <<   5));
    GPIOA->OSPEEDR &= ~((3U << 2*5));
    GPIOA->OSPEEDR |=  ((1U << 2*5));
    GPIOA->PUPDR   &= ~((3U << 2*5));

    /* enable GPIOC clock port for the Button B1 */
    RCC->IOPENR |=  (1U << 2);

    /* configure Button (PC.13) pins as input, no pull-up, pull-down */
    GPIOC->MODER   &= ~(3U << 2*13);
    GPIOC->OSPEEDR &= ~(3U << 2*13);
    GPIOC->OSPEEDR |=  (1U << 2*13);
    GPIOC->PUPDR   &= ~(3U << 2*13);

    // BSP_randomSeed(1234U); /* seed the random number generator */
#endif

    /* initialize the QS software tracing... */
    if (QS_INIT((void *)0) == 0U) {
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_button);
}
/*..........................................................................*/
void BSP_terminate(int16_t result) {
    (void)result;
}
/*..........................................................................*/
void BSP_wait4SW1(void) {
    while (READ_UserBtn() != 0U) {
        GPIOA->BSRR |= (LED_LD2);        /* turn LED2 on  */
        GPIOA->BSRR |= (LED_LD2 << 16);  /* turn LED2 off */
    }
}
/*..........................................................................*/
void BSP_ledOn(void) {
    //GPIOA->BSRR |= (LED_LD2);        /* turn LED2 on  */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
}
/*..........................................................................*/
void BSP_ledOff(void) {
    //GPIOA->BSRR |= (LED_LD2 << 16);  /* turn LED2 off */
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}
/*..........................................................................*/
void BSP_setlight(eTLidentity_t id, eTLlight_t light)
{
    GPIO_TypeDef *gpio = NULL;
    uint16_t IDCs[3], curlight;
    uint8_t n;

    switch(id)
    {
        case TrafficLightA:
            gpio = PIN_A_RED_GPIO_Port;
            IDCs[0] = PIN_A_RED_Pin;
            IDCs[1] = PIN_A_YLW_Pin;
            IDCs[2] = PIN_A_GRN_Pin;
            break;
        case TrafficLightB:
            gpio = PIN_B_RED_GPIO_Port;
            IDCs[0] = PIN_B_RED_Pin;
            IDCs[1] = PIN_B_YLW_Pin;
            IDCs[2] = PIN_B_GRN_Pin;
            break;
        case PedestrianLight:
            gpio = PIN_P_RED_GPIO_Port;
            IDCs[0] = PIN_P_RED_Pin;
            IDCs[1] = 0xFFFFu;
            IDCs[2] = PIN_P_GRN_Pin;
            break;
        default:
            IDCs[0] = 0xFFFF;
            IDCs[1] = 0xFFFF;
            IDCs[2] = 0xFFFF;
            break;
    }
    switch(light)
    {
        case RED:
            curlight = IDCs[0];
            break;
        case YELLOW:
            curlight = IDCs[1];
            break;
        case GREEN:
            curlight = IDCs[2];
            break;
        default:
            curlight = 0xFFFFu;
            break;

    }
    for (n = 0; n < 3; n++)
    {
        if (0xFFFFu != IDCs[n])
            HAL_GPIO_WritePin(gpio, IDCs[n], GPIO_PIN_RESET);
    }
    if (curlight != 0xFFFFu)
        HAL_GPIO_WritePin(gpio, curlight, GPIO_PIN_SET);
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
    /* set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate */
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    /* set priorities of ALL ISRs used in the system, see NOTE00
    *
    * !!!!!!!!!!!!!!!!!!!!!!!!!!!! CAUTION !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    * Assign a priority to EVERY ISR explicitly by calling NVIC_SetPriority().
    * DO NOT LEAVE THE ISR PRIORITIES AT THE DEFAULT VALUE!
    */
    HAL_NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIO, 0);
    /* ... */

    /* enable IRQs... */
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
    if ((USART2->ISR & 0x0080U) != 0) {  /* is TXE empty? */
        uint16_t b;

        QF_INT_DISABLE();
        b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {  /* not End-Of-Data? */
            USART2->TDR  = (b & 0xFFU);  /* put into the DR register */
        }
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
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY
/*..........................................................................*/
#define __DIV(__PCLK, __BAUD)       (((__PCLK / 4) *25)/(__BAUD))
#define __DIVMANT(__PCLK, __BAUD)   (__DIV(__PCLK, __BAUD)/100)
#define __DIVFRAQ(__PCLK, __BAUD)   \
    (((__DIV(__PCLK, __BAUD) - (__DIVMANT(__PCLK, __BAUD) * 100)) \
        * 16 + 50) / 100)
#define __USART_BRR(__PCLK, __BAUD) \
    ((__DIVMANT(__PCLK, __BAUD) << 4)|(__DIVFRAQ(__PCLK, __BAUD) & 0x0F))

uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[2*1024]; /* buffer for Quantum Spy */

    (void)arg; /* avoid the "unused parameter" compiler warning */
    QS_initBuf(qsBuf, sizeof(qsBuf));

#if 0 // USART2 initialized in HAL_init()
    /* enable peripheral clock for USART2 */
    RCC->IOPENR  |= ( 1ul <<  0);   /* Enable GPIOA clock   */
    RCC->APB1ENR |= ( 1ul << 17);   /* Enable USART#2 clock */

    /* Configure PA3 to USART2_RX, PA2 to USART2_TX */
    GPIOA->AFR[0] &= ~((15ul << 4* 3) | (15ul << 4* 2) );
    GPIOA->AFR[0] |=  (( 4ul << 4* 3) | ( 4ul << 4* 2) );
    GPIOA->MODER  &= ~(( 3ul << 2* 3) | ( 3ul << 2* 2) );
    GPIOA->MODER  |=  (( 2ul << 2* 3) | ( 2ul << 2* 2) );

    USART2->BRR  = __USART_BRR(SystemCoreClock, 115200ul);  /* baud rate */
    USART2->CR3  = 0x0000;         /* no flow control */
    USART2->CR2  = 0x0000;         /* 1 stop bit      */
    USART2->CR1  = ((1ul <<  2) |  /* enable RX       */
                    (1ul <<  3) |  /* enable TX       */
                    (0ul << 12) |  /* 8 data bits     */
                    (0ul << 28) |  /* 8 data bits     */
                    (1ul <<  0) ); /* enable USART    */
#endif

    QS_tickPeriod_ = SystemCoreClock / BSP_TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; /* to start the timestamp at zero */

    /* setup the QS filters... */
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

    QS_FILTER_ON(PHILO_STAT);

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
        while ((USART2->ISR & 0x0080U) == 0U) { /* while TXE not empty */
        }
        USART2->TDR  = (b & 0xFFU);  /* put into the DR register */
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}
#endif /* Q_SPY */
/*--------------------------------------------------------------------------*/

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
