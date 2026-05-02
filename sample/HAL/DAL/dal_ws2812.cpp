/**
 * @file dal_ws2812.cpp
 * @brief WS2812 RGB LED driver — PWM + DMA on TIM3_CH3
 */
#include "dal_ws2812.h"

namespace dal {

WS2812::WS2812(TIM_HandleTypeDef* tim_handle, uint32_t channel, uint16_t led_count)
    : htim_(tim_handle)
    , channel_(channel)
    , led_count_(led_count)
    , colors_(nullptr)
    , pwm_data_(nullptr)
    , pwm_data_size_(0)
    , initialized_(false) {
    colors_ = new Color[led_count_];
}

WS2812::~WS2812() {
    if (colors_) {
        delete[] colors_;
        colors_ = nullptr;
    }
    if (pwm_data_) {
        delete[] pwm_data_;
        pwm_data_ = nullptr;
    }
}

WS2812::WS2812(WS2812&& other) noexcept
    : htim_(other.htim_)
    , channel_(other.channel_)
    , led_count_(other.led_count_)
    , colors_(other.colors_)
    , pwm_data_(other.pwm_data_)
    , pwm_data_size_(other.pwm_data_size_)
    , hdma_(other.hdma_)
    , initialized_(other.initialized_) {
    other.colors_ = nullptr;
    other.pwm_data_ = nullptr;
    other.initialized_ = false;
}

WS2812& WS2812::operator=(WS2812&& other) noexcept {
    if (this != &other) {
        htim_ = other.htim_;
        channel_ = other.channel_;
        led_count_ = other.led_count_;
        colors_ = other.colors_;
        pwm_data_ = other.pwm_data_;
        pwm_data_size_ = other.pwm_data_size_;
        hdma_ = other.hdma_;
        initialized_ = other.initialized_;
        other.colors_ = nullptr;
        other.pwm_data_ = nullptr;
        other.initialized_ = false;
    }
    return *this;
}

bool WS2812::init() {
    if (htim_ == nullptr || led_count_ == 0) {
        return false;
    }

    // ---------- Timer: 84 MHz, ARR = 104 → 1.25 us / PWM period ----------
    htim_->Instance->ARR = PWM_PERIOD - 1;
    htim_->Instance->PSC = 0;
    __HAL_TIM_ENABLE_OCxPRELOAD(htim_, channel_);   // shadow CCRx → atomically update
    htim_->Instance->EGR = TIM_EGR_UG;               // latch
    __HAL_TIM_CLEAR_FLAG(htim_, TIM_FLAG_UPDATE);

    // Output compare: PWM mode 1, active high
    TIM_OC_InitTypeDef oc{};
    oc.OCMode     = TIM_OCMODE_PWM1;
    oc.Pulse      = 0;
    oc.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(htim_, &oc, channel_) != HAL_OK) {
        return false;
    }

    // ---------- DMA: TIM3_CH3 → DMA1 Stream2 Channel5 ----------
    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma_.Instance                 = DMA1_Stream2;
    hdma_.Init.Channel             = DMA_CHANNEL_5;
    hdma_.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_.Init.Mode                = DMA_NORMAL;
    hdma_.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&hdma_) != HAL_OK) {
        return false;
    }

    __HAL_LINKDMA(htim_, hdma[TIM_DMA_ID_CC3], hdma_);

    // ---------- Buffer: 24 entries per LED + reset ----------
    pwm_data_size_ = led_count_ * 24 + RESET_LEN;
    pwm_data_ = new uint16_t[pwm_data_size_]();

    initialized_ = true;
    return true;
}

// ---- helpers ----------------------------------------------------------------

void WS2812::encodeColors() {
    uint16_t pos = 0;

    for (uint16_t led = 0; led < led_count_; ++led) {
        // GRB, MSB first
        uint32_t grb = (static_cast<uint32_t>(colors_[led].g) << 16) |
                       (static_cast<uint32_t>(colors_[led].r) << 8) |
                       static_cast<uint32_t>(colors_[led].b);

        for (int8_t bit = 23; bit >= 0; --bit) {
            pwm_data_[pos++] = (grb >> bit) & 1 ? T1H : T0H;
        }
    }
}

void WS2812::prepareResetPulse() {
    uint16_t pos = led_count_ * 24;
    for (uint16_t i = 0; i < RESET_LEN; ++i) {
        pwm_data_[pos++] = 0;
    }
}

// ---- public API ------------------------------------------------------------

void WS2812::setColor(uint16_t index, const Color& color) {
    if (index < led_count_ && initialized_) {
        colors_[index] = color;
    }
}

void WS2812::fill(const Color& color) {
    if (!initialized_) return;
    for (uint16_t i = 0; i < led_count_; ++i) {
        colors_[i] = color;
    }
}

void WS2812::clear() {
    fill(Color::Black());
}

bool WS2812::show() {
    if (!initialized_) return false;

    // Abort previous transfer (harmless if already done)
    HAL_DMA_Abort(&hdma_);

    encodeColors();
    prepareResetPulse();

    // Load first CCR value into preload register
    __HAL_TIM_SET_COMPARE(htim_, channel_, pwm_data_[0]);

    // Generate update event: latches preload → active register
    htim_->Instance->EGR = TIM_EGR_UG;
    __HAL_TIM_CLEAR_FLAG(htim_, TIM_FLAG_UPDATE);

    // DMA feeds subsequent CCR values, one per PWM period
    if (HAL_DMA_Start(&hdma_,
            reinterpret_cast<uint32_t>(pwm_data_ + 1),
            reinterpret_cast<uint32_t>(&htim_->Instance->CCR3),
            pwm_data_size_ - 1) != HAL_OK) {
        return false;
    }

    // Enable CC3 DMA request (triggered on each compare match)
    __HAL_TIM_ENABLE_DMA(htim_, TIM_DMA_CC3);

    // Start PWM output
    if (HAL_TIM_PWM_Start(htim_, channel_) != HAL_OK) {
        HAL_DMA_Abort(&hdma_);
        __HAL_TIM_DISABLE_DMA(htim_, TIM_DMA_CC3);
        return false;
    }

    return true;
}

bool WS2812::isReady() const {
    if (!initialized_) return false;
    return !(hdma_.Instance->CR & DMA_SxCR_EN);
}

}  // namespace dal
