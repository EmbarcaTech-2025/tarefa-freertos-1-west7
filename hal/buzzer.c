#include "inc/buzzer.h"

void buzzer_init() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_init(slice_num, &config, true);

    pwm_set_gpio_level(BUZZER_PIN, 0);
}

void beep(uint duration_ms)
{
    pwm_set_gpio_level(BUZZER_PIN, 1024); 
    sleep_ms(duration_ms);
    pwm_set_gpio_level(BUZZER_PIN, 0);
    sleep_ms(duration_ms);
}