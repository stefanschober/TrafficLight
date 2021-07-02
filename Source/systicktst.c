#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/exception.h"
#include "hardware/structs/systick.h"
#include "hardware/address_mapped.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/m0plus.h"
#include "pico/binary_info.h"

#define BUTTON_PIN  10u
#define SYSTICK_PRIO 1u

static exception_handler_t origSysTick_hdl = (exception_handler_t)NULL;
static uint32_t     SystemCoreClock = 0u;
static uint16_t const period = 250;
static uint8_t      btn_press = 0;

static void systick_callback(void)
{
    static uint16_t tick = 0u;
    static uint8_t  btn = 0;

    tick++;
    if(tick >= period)
    {
        tick = 0;
        gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
    }

    if(!gpio_get(BUTTON_PIN))
    {
        if(btn < 15) btn++;
        else btn_press = 1;
    }
    else
    {
        if(btn) btn--;
        else btn_press = 0;
    }
}

static void SysTick_Config(uint32_t f)
{
    systick_hw->csr  = 0x00ul;
    systick_hw->cvr  = 0x00ul;
    systick_hw->rvr  = f;
    systick_hw->csr  = 0x06ul;
    systick_hw->csr |= 0x01ul;
}

static void setup_hw(void)
{
    stdio_init_all();
    clocks_init();

    SystemCoreClock = clock_get_hz(clk_sys);

    origSysTick_hdl = exception_get_vtable_handler(SYSTICK_EXCEPTION);
    exception_set_exclusive_handler(SYSTICK_EXCEPTION, systick_callback);
    
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    // We are using the button to pull down to 0v when pressed, so ensure that when
    // unpressed, it uses internal pull ups. Otherwise when unpressed, the input will
    // be floating.
    gpio_pull_up(BUTTON_PIN);

    sleep_ms(5000);
    printf("systick prio = %u (0x%08X, 0x%08X)\n", SYSTICK_PRIO, SYSTICK_PRIO << M0PLUS_SHPR3_PRI_15_LSB, M0PLUS_SHPR3_PRI_15_BITS);
    hw_write_masked((uint32_t * const)(PPB_BASE + M0PLUS_SHPR3_OFFSET), (SYSTICK_PRIO << M0PLUS_SHPR3_PRI_15_LSB), M0PLUS_SHPR3_PRI_15_BITS);
    SysTick_Config(SystemCoreClock / 1000u);
}

void main(void)
{
    uint8_t oldbtn = btn_press;

    setup_hw();

    while(true)
    {
        sleep_ms(50);
        if(btn_press && (oldbtn != btn_press)) systick_hw->csr ^= 0x03ul;
        oldbtn = btn_press;
    }
}
