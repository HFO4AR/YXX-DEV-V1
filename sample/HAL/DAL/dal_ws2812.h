/**
 * @file dal_ws2812.h
 * @brief WS2812 RGB LED driver using timer DMA
 */
#ifndef DAL_WS2812_H
#define DAL_WS2812_H

#include "drv_timer.h"
#include "stm32f4xx_hal_dma.h"
#include <cstdint>
#include <cstring>

namespace dal {

/**
 * @brief Color representation (GRB order for WS2812)
 */
struct Color {
    uint8_t g;  ///< Green
    uint8_t r;  ///< Red
    uint8_t b;  ///< Blue

    constexpr Color() : g(0), r(0), b(0) {}
    constexpr Color(uint8_t green, uint8_t red, uint8_t blue)
        : g(green), r(red), b(blue) {}

    static constexpr Color Black() { return Color(0, 0, 0); }
    static constexpr Color Red() { return Color(0, 255, 0); }
    static constexpr Color Green() { return Color(255, 0, 0); }
    static constexpr Color Blue() { return Color(0, 0, 255); }
    static constexpr Color White() { return Color(255, 255, 255); }
    static constexpr Color Yellow() { return Color(255, 255, 0); }
    static constexpr Color Cyan() { return Color(255, 0, 255); }
    static constexpr Color Magenta() { return Color(0, 255, 255); }

    static constexpr Color RGB(uint8_t red, uint8_t green, uint8_t blue) {
        return Color(green, red, blue);
    }
};

/**
 * @brief WS2812 RGB LED strip driver
 * @note Uses PWM with DMA to generate the WS2812 protocol timing
 */
class WS2812 {
public:
    /**
     * @brief Construct a WS2812 driver
     * @param tim_handle Timer handle (must support DMA)
     * @param channel Timer channel (TIM_CHANNEL_1-4)
     * @param led_count Number of LEDs in the strip
     */
    WS2812(TIM_HandleTypeDef* tim_handle, uint32_t channel, uint16_t led_count);
    ~WS2812();

    WS2812(const WS2812&) = delete;
    WS2812& operator=(const WS2812&) = delete;
    WS2812(WS2812&& other) noexcept;
    WS2812& operator=(WS2812&& other) noexcept;

    /**
     * @brief Initialize the driver
     * @return true if initialization successful
     */
    bool init();

    /**
     * @brief Set color of a single LED
     * @param index LED index (0 to led_count-1)
     * @param color Color to set
     */
    void setColor(uint16_t index, const Color& color);

    /**
     * @brief Set color of all LEDs
     * @param color Color to set
     */
    void fill(const Color& color);

    /**
     * @brief Clear all LEDs (set to black)
     */
    void clear();

    /**
     * @brief Show/Update the strip (send data via DMA)
     * @return true if DMA transfer started successfully
     */
    bool show();

    /**
     * @brief Check if DMA transfer is complete
     * @return true if transfer complete
     */
    bool isReady() const;

    /**
     * @brief Get number of LEDs
     * @return LED count
     */
    uint16_t ledCount() const { return led_count_; }

private:
    /**
     * @brief Convert color bits to PWM duty cycle values
     */
    void encodeColors();

    /**
     * @brief Calculate reset pulse duration
     */
    void prepareResetPulse();

    static constexpr uint16_t PWM_PERIOD = 105;   ///< 1.25us at 84MHz (ARR = 104)
    static constexpr uint16_t T0H = 34;          ///< 0.4us at 84MHz
    static constexpr uint16_t T1H = 67;          ///< 0.8us at 84MHz
    static constexpr uint16_t RESET_LEN = 42;    ///< >50us (42 * 1.25us = 52.5us)

    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
    uint16_t led_count_;
    Color* colors_;
    uint16_t* pwm_data_;          ///< PWM duty cycle data for DMA
    uint16_t pwm_data_size_;      ///< Size of PWM data array
    DMA_HandleTypeDef hdma_;      ///< DMA handle for CCR update
    bool initialized_;
};

}  // namespace dal

#endif  // DAL_WS2812_H