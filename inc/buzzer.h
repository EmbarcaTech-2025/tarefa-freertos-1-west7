#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "FreeRTOS.h"
#include "task.h"

#define BUZZER_PIN 21

static inline void buzzer_init()
{
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(BUZZER_PIN, 0);
}

static inline void beep(uint duration_ms)
{
    pwm_set_gpio_level(BUZZER_PIN, 1024);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    pwm_set_gpio_level(BUZZER_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}

#endif