/**
 * @file drv_timer.h
 * @brief C++ wrapper for STM32 HAL Timer
 */
#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#include "stm32f4xx_hal.h"
#include <cstdint>

namespace drv {

/**
 * @brief Timer mode selection
 */
enum class TimerMode {
    Polling,     ///< Polling mode
    Interrupt,   ///< Interrupt mode
    DMA,         ///< DMA mode
};

/**
 * @brief Basic timer wrapper for STM32 HAL
 */
class Timer {
public:
    explicit Timer(TIM_HandleTypeDef* tim_handle);
    ~Timer();

    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&& other) noexcept;
    Timer& operator=(Timer&& other) noexcept;

    bool init(uint16_t prescaler, uint16_t period);
    bool start(TimerMode mode = TimerMode::Polling);
    bool stop();

    uint32_t getCounter();
    void setCounter(uint32_t value);
    bool isRunning();

    void registerPeriodCallback(void (*callback)(TIM_HandleTypeDef*));

    TIM_HandleTypeDef* handle();
    const TIM_HandleTypeDef* handle() const;

private:
    TIM_HandleTypeDef* htim_;
};

/**
 * @brief PWM Timer wrapper
 */
class PwmTimer {
public:
    explicit PwmTimer(TIM_HandleTypeDef* tim_handle, uint32_t channel);
    ~PwmTimer();

    PwmTimer(const PwmTimer&) = delete;
    PwmTimer& operator=(const PwmTimer&) = delete;
    PwmTimer(PwmTimer&& other) noexcept;
    PwmTimer& operator=(PwmTimer&& other) noexcept;

    bool init(uint16_t period, uint16_t pulse);
    bool start(TimerMode mode = TimerMode::Polling);
    bool stop();
    void setPulse(uint16_t pulse);

private:
    TIM_HandleTypeDef* htim_;
    uint32_t channel_;
};

}  // namespace drv

#endif  // DRV_TIMER_H