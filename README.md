# 简介
开发板基于 STM32F407VGT6 ，支持CAN、UART、I2C、SPI通信。带有多路pwm输出和adc输入可配置引脚。带有三路24VMOS输出接口。板载一颗WS2812用于显示状态。

# 时钟频率
外置时钟频率为12MHz。
# 功能
### RST按键
### UFD模式
按住BOOT0同时按下RST，进入UFD模式。
### USB1接口（OTG）
### USB2接口（UART1）
该接口板载一颗，板载CH340
### SWD接口

### 可配置LED
### 可配置按键
### WS2812
可编程RGB灯珠。
输入接口为PB0可配置为TIM3_CH3。

### 蜂鸣器
### CAN
### UART
### I2C
### 16PIN插针
| 路号   | STM32引脚 | CubeMX配置建议        |
| ---- |  ------- | ----------------- |
| CH1       | PD9#     |  USART3_RX(DBUS)        |
| CH2   | PA15    | TIM2_CH1 / GPIO   |
| CH3   | PB3     | TIM2_CH2 / GPIO   |
| CH4   | PE14    | TIM1_CH4 / GPIO   |
| CH5   | PE13    | TIM1_CH3 / GPIO   |
| CH6   | PE11    | TIM1_CH2 / GPIO   |
| CH7   | PE9     | TIM1_CH1 / GPIO   |
| CH8       | PA0     | ADC123_IN0 / GPIO |
| CH9       | PA1     | ADC123_IN1 / GPIO |
| CH10      | PA2     | ADC123_IN2 / GPIO |
| CH11      | PA3     | ADC123_IN3 / GPIO |
| CH12      | PC4     | ADC12_IN14 / GPIO |
| CH13      | PC5     | ADC12_IN15 / GPIO |
| CH14  | PB10    | TIM2_CH3 / GPIO   |
| CH15  | PB11    | TIM2_CH4 / GPIO   |
| CH16 | PC9     | GPIO / 可复用功能      |

注意CH1(PD9)为兼容DJI DBUS，带有反相器。
CH1-CH7 配有5V供电引脚（只允许输出，不能反向给mcu供电）
CH8-CH16 配套有3.3V供电引脚（只允许输出，不能反向给mcu供电）

### 24V MOS输出接口
| 通道   | 引脚         |接口号|
| ---- | --------------- |---|
| MOS1 | PD12 |CN1|
| MOS2 | PD13 |CN2|
| MOS3 | PD14 |CN3|

三路均可通过TIM4输出pwm信号。
### FPC拓展接口
| FPC信号         | 引脚 |
| ------------- | -------- |
| FPC_PB14      | PB14     |
| FPC_PE7       | PE7      |
| FPC_PE8       | PE8      |
| FPC_PE10      | PE10     |
| FPC_PC13      | PC13     |
| FPC_PC14      | PC14     |
| FPC_PC15      | PC15     |
| FPC_I2C1_SCL  | PB6      |
| FPC_I2C1_SDA  | PB7      |
| FPC_I2C2_SCL  | PB10     |
| FPC_I2C2_SDA  | PB11     |
| FPC_SPI1_SCK  | PA5      |
| FPC_SPI1_MISO | PA6      |
| FPC_SPI1_MOSI | PA7      |
| FPC_SPI3_SCK  | PC10     |
| FPC_SPI3_MISO | PC11     |
| FPC_SPI3_MOSI | PC12     |

# 电源

# 示例程序

# 故障排查