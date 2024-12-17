/**
  ******************************************************************************
  * @file      startup_stm32f091xc.c
  * @author    Stefan Schober
  * @brief     STM32F091xC devices vector table for GCC toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M0 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * 
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
// Includes
#include <stdint.h>
#include <stddef.h>
#include "stm32f0xx.h"
#include "system_stm32f0xx.h"

// typedefs
typedef void (*p_isr_t)(void);

// function prototypes
__attribute__(( weak)) void Default_Handler(void);
__attribute__((weak, noreturn, used)) void Reset_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void NMI_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void HardFault_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void SVC_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void PendSV_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void SysTick_Handler(void);
__attribute__((weak, alias("Default_Handler"))) void WWDG_IRQHandler(void);                   /* Window WatchDog              */
__attribute__((weak, alias("Default_Handler"))) void PVD_VDDIO2_IRQHandler(void);             /* PVD and VDDIO2 through EXTI Line detect */
__attribute__((weak, alias("Default_Handler"))) void RTC_IRQHandler(void);                    /* RTC through the EXTI line    */
__attribute__((weak, alias("Default_Handler"))) void FLASH_IRQHandler(void);                  /* FLASH                        */
__attribute__((weak, alias("Default_Handler"))) void RCC_CRS_IRQHandler(void);                /* RCC and CRS                  */
__attribute__((weak, alias("Default_Handler"))) void EXTI0_1_IRQHandler(void);                /* EXTI Line 0 and 1            */
__attribute__((weak, alias("Default_Handler"))) void EXTI2_3_IRQHandler(void);                /* EXTI Line 2 and 3            */
__attribute__((weak, alias("Default_Handler"))) void EXTI4_15_IRQHandler(void);               /* EXTI Line 4 to 15            */
__attribute__((weak, alias("Default_Handler"))) void TSC_IRQHandler(void);                    /* TSC                          */
__attribute__((weak, alias("Default_Handler"))) void DMA1_Ch1_IRQHandler(void);               /* DMA1 Channel 1               */
__attribute__((weak, alias("Default_Handler"))) void DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler(void);  /* DMA1 Channel 2 and 3 & DMA2 Channel 1 and 2 */
__attribute__((weak, alias("Default_Handler"))) void DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler(void);  /* DMA1 Channel 4 to 7 & DMA2 Channel 3 to 5 */
__attribute__((weak, alias("Default_Handler"))) void ADC1_COMP_IRQHandler(void);              /* ADC1, COMP1 and COMP2         */
__attribute__((weak, alias("Default_Handler"))) void TIM1_BRK_UP_TRG_COM_IRQHandler(void);    /* TIM1 Break, Update, Trigger and Commutation */
__attribute__((weak, alias("Default_Handler"))) void TIM1_CC_IRQHandler(void);                /* TIM1 Capture Compare         */
__attribute__((weak, alias("Default_Handler"))) void TIM2_IRQHandler(void);                   /* TIM2                         */
__attribute__((weak, alias("Default_Handler"))) void TIM3_IRQHandler(void);                   /* TIM3                         */
__attribute__((weak, alias("Default_Handler"))) void TIM6_DAC_IRQHandler(void);               /* TIM6 and DAC                 */
__attribute__((weak, alias("Default_Handler"))) void TIM7_IRQHandler(void);                   /* TIM7                         */
__attribute__((weak, alias("Default_Handler"))) void TIM14_IRQHandler(void);                  /* TIM14                        */
__attribute__((weak, alias("Default_Handler"))) void TIM15_IRQHandler(void);                  /* TIM15                        */
__attribute__((weak, alias("Default_Handler"))) void TIM16_IRQHandler(void);                  /* TIM16                        */
__attribute__((weak, alias("Default_Handler"))) void TIM17_IRQHandler(void);                  /* TIM17                        */
__attribute__((weak, alias("Default_Handler"))) void I2C1_IRQHandler(void);                   /* I2C1                         */
__attribute__((weak, alias("Default_Handler"))) void I2C2_IRQHandler(void);                   /* I2C2                         */
__attribute__((weak, alias("Default_Handler"))) void SPI1_IRQHandler(void);                   /* SPI1                         */
__attribute__((weak, alias("Default_Handler"))) void SPI2_IRQHandler(void);                   /* SPI2                         */
__attribute__((weak, alias("Default_Handler"))) void USART1_IRQHandler(void);                 /* USART1                       */
__attribute__((weak, alias("Default_Handler"))) void USART2_IRQHandler(void);                 /* USART2                       */
__attribute__((weak, alias("Default_Handler"))) void USART3_8_IRQHandler(void);               /* USART3, USART4, USART5, USART6, USART7, USART8 */
__attribute__((weak, alias("Default_Handler"))) void CEC_CAN_IRQHandler(void);                /* CEC and CAN                  */

// extern declarations of linker defined symbols
extern uint32_t _sidata[], _sdata[], _edata[];
extern uint32_t _sbss[], _ebss[];
extern uint32_t _sstack[], _estack[], _stack_size[], __initial_sp[];

// declaration of main()
extern void main(int argc, char *argv[]);

// the stack check pattern
static uint32_t const stackCheckPattern = 0xA5A5A5A5u;

void Reset_Handler(void)
{
    register uint32_t *sp __asm("sp");
    register uint32_t *s, *d;

    sp = __initial_sp;                                  /* set stack pointer */
    s = _sidata;                                        /* Copy the data segment initializers from flash to SRAM */
    d = _sdata;
    while(d < _edata)
        *d++ = *s++;

    d = _sbss;                                          /* Zero fill the bss segment. */
    while(d < _ebss)
        *d++ = 0x00000000u;
    
    d = _sstack;
    while(d < _estack)
        *d++ = stackCheckPattern;

    SystemInit();
    main(0, NULL);

    while(1) {}
}

uint16_t getMaxStackUsage(void)
{
#if defined STACK_CHECK
    const uint8_t checkPattern = (uint8_t)stackCheckPattern;
    register const uint8_t *p = (uint8_t *)_sstack;

    while((p < (uint8_t *)_estack) && (*p == checkPattern))
    {
        p++;
    }
    return (1000u * ((uint32_t)_estack - (uint32_t)p)) / (uint32_t)_stack_size;
#else
    return 0;
#endif
}

uint16_t getUsedStackBytes(void)
{
#if defined STACK_CHECK
    register uint32_t sp __asm("sp");   /* set stack pointer */

    return (uint16_t)((uint32_t )_estack - sp);
#else
    return 0;
#endif
}

uint16_t getFreeStackBytes(void)
{
#if defined STACK_CHECK
    return (uint16_t)((uint32_t)_stack_size - getUsedStackBytes());
#else
    return 0;
#endif
}

/**
 * @brief  This is the code that gets called when the processor receives an
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 *
 * @param  None
 * @retval : None
*/
void Default_Handler(void)
{
    // endless loop
    while(1) {}
}

/******************************************************************************
*
* The minimal vector table for a Cortex M0.  Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
*
******************************************************************************/
p_isr_t __attribute((used, section(".vectors")))g_pfnVectors[] = {
    (p_isr_t)__initial_sp,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SVC_Handler,
    NULL,
    NULL,
    PendSV_Handler,
    SysTick_Handler,
    WWDG_IRQHandler,                   /* Window WatchDog              */
    PVD_VDDIO2_IRQHandler,             /* PVD and VDDIO2 through EXTI Line detect */
    RTC_IRQHandler,                    /* RTC through the EXTI line    */
    FLASH_IRQHandler,                  /* FLASH                        */
    RCC_CRS_IRQHandler,                /* RCC and CRS                  */
    EXTI0_1_IRQHandler,                /* EXTI Line 0 and 1            */
    EXTI2_3_IRQHandler,                /* EXTI Line 2 and 3            */
    EXTI4_15_IRQHandler,               /* EXTI Line 4 to 15            */
    TSC_IRQHandler,                    /* TSC                          */
    DMA1_Ch1_IRQHandler,               /* DMA1 Channel 1               */
    DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler,  /* DMA1 Channel 2 and 3 & DMA2 Channel 1 and 2 */
    DMA1_Ch4_7_DMA2_Ch3_5_IRQHandler,  /* DMA1 Channel 4 to 7 & DMA2 Channel 3 to 5 */
    ADC1_COMP_IRQHandler,              /* ADC1, COMP1 and COMP2         */
    TIM1_BRK_UP_TRG_COM_IRQHandler,    /* TIM1 Break, Update, Trigger and Commutation */
    TIM1_CC_IRQHandler,                /* TIM1 Capture Compare         */
    TIM2_IRQHandler,                   /* TIM2                         */
    TIM3_IRQHandler,                   /* TIM3                         */
    TIM6_DAC_IRQHandler,               /* TIM6 and DAC                 */
    TIM7_IRQHandler,                   /* TIM7                         */
    TIM14_IRQHandler,                  /* TIM14                        */
    TIM15_IRQHandler,                  /* TIM15                        */
    TIM16_IRQHandler,                  /* TIM16                        */
    TIM17_IRQHandler,                  /* TIM17                        */
    I2C1_IRQHandler,                   /* I2C1                         */
    I2C2_IRQHandler,                   /* I2C2                         */
    SPI1_IRQHandler,                   /* SPI1                         */
    SPI2_IRQHandler,                   /* SPI2                         */
    USART1_IRQHandler,                 /* USART1                       */
    USART2_IRQHandler,                 /* USART2                       */
    USART3_8_IRQHandler,               /* USART3, USART4, USART5, USART6, USART7, USART8 */
    CEC_CAN_IRQHandler,                /* CEC and CAN                  */
};
