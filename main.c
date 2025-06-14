#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "inc/buzzer.h"
#include "inc/ssd1306.h"

#define R_LED 13
#define B_LED 12
#define G_LED 11
#define BTN_A 5
#define BTN_B 6
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_PORT i2c1

struct render_area frame = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1,
};

uint8_t ssd[ssd1306_buffer_length];

SemaphoreHandle_t led_semaphore;
SemaphoreHandle_t buzzer_semaphore;

TaskHandle_t led_task_handle = NULL;
TaskHandle_t buzzer_task_handle = NULL;

void update_display(bool led_on, bool buzzer_on)
{
  memset(ssd, 0, ssd1306_buffer_length);

  if (led_on)
    ssd1306_draw_string(ssd, 0, 0,  "LED TASK:    ON");
  else
    ssd1306_draw_string(ssd, 0, 0,  "LED TASK:    OFF");

  if (buzzer_on)
    ssd1306_draw_string(ssd, 0, 16, "BUZZER TASK: ON");
  else
    ssd1306_draw_string(ssd, 0, 16, "BUZZER TASK: OFF");

  render_on_display(ssd, &frame);
}

void led_task(void *params)
{
  gpio_init(R_LED);
  gpio_set_dir(R_LED, GPIO_OUT);
  gpio_init(B_LED);
  gpio_set_dir(B_LED, GPIO_OUT);
  gpio_init(G_LED);
  gpio_set_dir(G_LED, GPIO_OUT);

  while (true)
  {

    if (xSemaphoreTake(led_semaphore, portMAX_DELAY) == pdTRUE)
    {

      gpio_put(B_LED, 0);
      gpio_put(R_LED, 1);
      vTaskDelay(pdMS_TO_TICKS(500));
      gpio_put(R_LED, 0);
      gpio_put(G_LED, 1);
      vTaskDelay(pdMS_TO_TICKS(500));
      gpio_put(G_LED, 0);
      gpio_put(B_LED, 1);
      vTaskDelay(pdMS_TO_TICKS(500));

      xSemaphoreGive(led_semaphore);
    }
  }
}

void buzzer_task(void *params)
{
  buzzer_init();
  while (true)
  {
    if (xSemaphoreTake(buzzer_semaphore, portMAX_DELAY) == pdTRUE)
    {
      beep(1000);
      xSemaphoreGive(buzzer_semaphore);
    }
  }
}

void button_task(void *params)
{
  gpio_init(BTN_A);
  gpio_set_dir(BTN_A, GPIO_IN);
  gpio_pull_up(BTN_A);

  gpio_init(BTN_B);
  gpio_set_dir(BTN_B, GPIO_IN);
  gpio_pull_up(BTN_B);

  bool led_suspended = false, buzzer_suspended = false;
  
  update_display(!led_suspended, !buzzer_suspended);
  
  while (true)
  {
    bool button_a_pressed = !gpio_get(BTN_A);
    bool button_b_pressed = !gpio_get(BTN_B);
    
    bool changed = false;

    if (button_a_pressed && !led_suspended)
    {
      // vTaskSuspend(led_task_handle);
      xSemaphoreTake(led_semaphore, portMAX_DELAY);
      led_suspended = true;
      changed = true;
    }
    else if (!button_a_pressed && led_suspended)
    {
      // vTaskResume(led_task_handle);
      xSemaphoreGive(led_semaphore);
      led_suspended = false;
      changed = true;
    }

    if (button_b_pressed && !buzzer_suspended)
    {
      // vTaskSuspend(buzzer_task_handle);
      xSemaphoreTake(buzzer_semaphore, portMAX_DELAY);
      buzzer_suspended = true;
      changed = true;
    }
    else if (!button_b_pressed && buzzer_suspended)
    {
      // vTaskResume(buzzer_task_handle);
      xSemaphoreGive(buzzer_semaphore);
      buzzer_suspended = false;
      changed = true;
    }

    if (changed) 
      update_display(!led_suspended, !buzzer_suspended);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

int main()
{
  stdio_init_all();

  i2c_init(I2C_PORT, ssd1306_i2c_clock * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  ssd1306_init();

  calculate_render_area_buffer_length(&frame);
  memset(ssd, 0, ssd1306_buffer_length);
  render_on_display(ssd, &frame);

  led_semaphore = xSemaphoreCreateBinary();
  buzzer_semaphore = xSemaphoreCreateBinary();

  xSemaphoreGive(led_semaphore);
  xSemaphoreGive(buzzer_semaphore);

  xTaskCreate(led_task, "LED_Task", 256, NULL, 1, /* &led_task_handle */ NULL);
  xTaskCreate(buzzer_task, "Buzzer_Task", 256, NULL, 1, /* &buzzer_task_handle */ NULL);
  xTaskCreate(button_task, "Button_Task", 256, NULL, 2, NULL);
  vTaskStartScheduler();

  while (1)
    ;
}