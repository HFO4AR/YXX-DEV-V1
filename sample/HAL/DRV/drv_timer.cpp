/**
 * @file drv_timer.cpp
 * @brief C++ wrapper implementation for STM32 HAL Timer
 */
#include "drv_timer.h"

namespace drv {

// ============== Timer ==============

Timer::Timer(TIM_HandleTypeDef* tim_handle) : htim_(tim_handle) {}

Timer::~Timer() = default;

Timer::Timer(Timer&& other) noexcept : htim_(other.htim_) {
    other.htim_ = nullptr;
}

Timer& Timer::operator=(Timer&& other) noexcept {
    if (this != &other) {
        htim_ = other.htim_;
        other.htim_ = nullptr;
    }
    return *this;
}

bool Timer::init(uint16_t prescaler, uint16_t period) {
    if (htim_ == nullptr) {
        return false;
    }
    htim_->Init.Prescaler = prescaler;
    htim_->Init.Period = period;
    htim_->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim_->Init.RepetitionCounter = 0;
    htim_->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    return HAL_TIM_Base_Init(htim_) == HAL_OK;
}

bool Timer::start(TimerMode mode) {
    if (htim_ == nullptr) {
        return false;
    }
    switch (mode) {
    case TimerMode::Interrupt:
        return HAL_TIM_Base_Start_IT(htim_) == HAL_OK;
    case TimerMode::DMA:
        return false;
    case TimerMode::Polling:
    default:
        return HAL_TIM_Base_Start(htim_) == HAL_OK;
    }
}

bool Timer::stop() {
    if (htim_ == nullptr) {
        return false;
    }
    return HAL_TIM_Base_Stop(htim_) == HAL_OK;
}

uint32_t Timer::getCounter() {
    return htim_ ? __HAL_TIM_GET_COUNTER(htim_) : 0;
}

void Timer::setCounter(uint32_t value) {
    if (htim_) {
        htim_->Instance->CNT = value;
    }
}

bool Timer::isRunning() {
    return htim_ && __HAL_TIM_GET_COUNTER(htim_) != 0;
}

void Timer::registerPeriodCallback(void (*callback)(TIM_HandleTypeDef*)) {
    if (htim_ && callback) {
#if USE_HAL_TIM_REGISTER_CALLBACKS
        htim_->PeriodElapsedCallback = callback;
#endif
    }
}

TIM_HandleTypeDef* Timer::handle() {
    return htim_;
}

const TIM_HandleTypeDef* Timer::handle() const {
    return htim_;
}

// ============== PwmTimer ==============

PwmTimer::PwmTimer(TIM_HandleTypeDef* tim_handle, uint32_t channel)
    : htim_(tim_handle), channel_(channel) {}

PwmTimer::~PwmTimer() = default;

PwmTimer::PwmTimer(PwmTimer&& other) noexcept
    : htim_(other.htim_), channel_(other.channel_) {
    other.htim_ = nullptr;
}

PwmTimer& PwmTimer::operator=(PwmTimer&& other) noexcept {
    if (this != &other) {
        htim_ = other.htim_;
        channel_ = other.channel_;
        other.htim_ = nullptr;
    }
    return *this;
}

bool PwmTimer::init(uint16_t period, uint16_t pulse) {
    if (htim_ == nullptr) {
        return false;
    }

    TIM_Base_InitTypeDef base_init{};
    base_init.Prescaler = 0;
    base_init.Period = period;
    base_init.CounterMode = TIM_COUNTERMODE_UP;
    base_init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    base_init.RepetitionCounter = 0;
    base_init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(htim_) != HAL_OK) {
        return false;
    }

    TIM_OC_InitTypeDef oc_init{};
    oc_init.OCMode = TIM_OCMODE_PWM1;
    oc_init.Pulse = pulse;
    oc_init.OCPolarity = TIM_OCPOLARITY_HIGH;
    oc_init.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    oc_init.OCFastMode = TIM_OCFAST_DISABLE;
    oc_init.OCIdleState = TIM_OCIDLESTATE_RESET;
    oc_init.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    switch (channel_) {
    case TIM_CHANNEL_1:
        HAL_TIM_OC_ConfigChannel(htim_, &oc_init, TIM_CHANNEL_1);
        break;
    case TIM_CHANNEL_2:
        HAL_TIM_OC_ConfigChannel(htim_, &oc_init, TIM_CHANNEL_2);
        break;
    case TIM_CHANNEL_3:
        HAL_TIM_OC_ConfigChannel(htim_, &oc_init, TIM_CHANNEL_3);
        break;
    case TIM_CHANNEL_4:
        HAL_TIM_OC_ConfigChannel(htim_, &oc_init, TIM_CHANNEL_4);
        break;
    default:
        return false;
    }
    return true;
}

bool PwmTimer::start(TimerMode mode) {
    if (htim_ == nullptr) {
        return false;
    }
    if (mode == TimerMode::Interrupt) {
        return HAL_TIM_PWM_Start_IT(htim_, channel_) == HAL_OK;
    }
    return HAL_TIM_PWM_Start(htim_, channel_) == HAL_OK;
}

bool PwmTimer::stop() {
    return htim_ && HAL_TIM_PWM_Stop(htim_, channel_) == HAL_OK;
}

void PwmTimer::setPulse(uint16_t pulse) {
    if (htim_) {
        switch (channel_) {
        case TIM_CHANNEL_1:
            htim_->Instance->CCR1 = pulse;
            break;
        case TIM_CHANNEL_2:
            htim_->Instance->CCR2 = pulse;
            break;
        case TIM_CHANNEL_3:
            htim_->Instance->CCR3 = pulse;
            break;
        case TIM_CHANNEL_4:
            htim_->Instance->CCR4 = pulse;
            break;
        }
    }
}

}  // namespace drv