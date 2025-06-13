#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"

#define BUZZER_PIN 21 

void buzzer_init();
void beep(uint duration_ms);

#endif