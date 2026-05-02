//
// Created by yuki on 2026/5/2.
// Author HFO4AR https://github.com/HFO4AR
//

#include "begin.h"
#include <cmath>

extern "C" TIM_HandleTypeDef htim3;

// ===================================================================
// WS2812 bit‑bang via PB0 | DWT cycle‑counter (168 MHz)
// ===================================================================

static void dwt_init() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL     |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT    = 0;
}

static inline void delay_cycles(uint32_t n) {
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < n) {}
}

// 168 MHz → 1 cycle ≈ 6 ns
// T0H=0.30 µs → 50    T0L=0.80 µs → 134
// T1H=0.75 µs → 126   T1L=0.35 µs → 59
static void send_byte(uint8_t byte) {
    for (int i = 7; i >= 0; --i) {
        if (byte & (1 << i)) {
            GPIOB->BSRR = GPIO_PIN_0;
            delay_cycles(120);
            GPIOB->BSRR = (uint32_t)GPIO_PIN_0 << 16;
            delay_cycles(55);
        } else {
            GPIOB->BSRR = GPIO_PIN_0;
            delay_cycles(42);
            GPIOB->BSRR = (uint32_t)GPIO_PIN_0 << 16;
            delay_cycles(130);
        }
    }
}

// Send GRB data for nleds, then >80 µs reset
static void ws2812_send(const uint8_t* grb, int nleds) {
    __disable_irq();
    for (int i = 0; i < nleds * 3; ++i)
        send_byte(grb[i]);
    // Reset: >50 µs LOW → 100 µs for margin
    GPIOB->BSRR = (uint32_t)GPIO_PIN_0 << 16;
    delay_cycles(16800);  // 100 µs
    __enable_irq();
}

static void pb0_to_gpio_out() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIOB->MODER   = (GPIOB->MODER & ~GPIO_MODER_MODER0) | GPIO_MODER_MODER0_0;
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT_0;
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~GPIO_OSPEEDER_OSPEEDR0) | GPIO_OSPEEDER_OSPEEDR0;
    GPIOB->PUPDR   &= ~GPIO_PUPDR_PUPDR0;
    GPIOB->BSRR     = (uint32_t)GPIO_PIN_0 << 16;
}

// ===================================================================

namespace app {

void Begin() {
    dwt_init();
    pb0_to_gpio_out();

    uint8_t frame[3];  // GRB for 1 LED

    // Pre‑compute sine table: 150 steps, 0..255
    static uint8_t sine[150];
    for (int i = 0; i < 150; ++i) {
        float phase = 2.0f * 3.14159f * i / 150.0f;
        sine[i] = (uint8_t)(127.5f + 127.5f * sinf(phase));
    }

    int step = 0;
    while (1) {
        uint8_t b = sine[step];
        step = (step + 1) % 150;
        frame[0] = 0;    // G
        frame[1] = 0;    // R
        frame[2] = b;    // B

        ws2812_send(frame, 1);

        // ~20 ms → 50 FPS
        delay_cycles(3360000);
    }
}

void Loop() {
}

}  // namespace app

extern "C" {

void AppBegin() {
    app::Begin();
}

void AppLoop() {
    app::Loop();
}

}
