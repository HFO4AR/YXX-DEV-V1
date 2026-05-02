//
// Created by yuki on 2026/5/2.
// Author HFO4AR https://github.com/HFO4AR
//

#ifndef HAL_BEGIN_H
#define HAL_BEGIN_H

#ifdef __cplusplus
#include "dal_ws2812.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void AppBegin(void);
void AppLoop(void);

#ifdef __cplusplus
}
#endif

#endif  // HAL_BEGIN_H
