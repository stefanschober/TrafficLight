#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "RP2040.h"

#define BUTTON_PIN  10u
#define SYSTICK_PRIO 1u

static uint16_t const period = 250;
static uint8_t      btn_press = 0;

static void SysTick_Handler(void)
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


static void setup_hw(void)
{
    SystemCoreClockUpdate();

    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    // We are using the button to pull down to 0v when pressed, so ensure that when
    // unpressed, it uses internal pull ups. Otherwise when unpressed, the input will
    // be floating.
    gpio_pull_up(BUTTON_PIN);

    sleep_ms(5000);
    printf("systick prio = %u\n", SYSTICK_PRIO);
    NVIC_SetPriority(SysTick_IRQn, SYSTICK_PRIO);

    SysTick_Config(SystemCoreClock / 1000u);
}

void main(void)
{
    uint8_t oldbtn = btn_press;

    setup_hw();

    while(true)
    {
        sleep_ms(50);
        if(btn_press && (oldbtn != btn_press)) SysTick->CTRL ^= 0x03ul;
        oldbtn = btn_press;
    }
}
