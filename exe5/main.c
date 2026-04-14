/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN_R 28
#define BTN_PIN_Y 21

#define LED_PIN_R 5
#define LED_PIN_Y 10

QueueHandle_t xQueueBtn;
SemaphoreHandle_t xSemaphoreLedR;
SemaphoreHandle_t xSemaphoreLedY;

void z_irq(uint gpio, uint32_t events) {
    BaseType_t y = pdFALSE;

    if ((events & GPIO_IRQ_EDGE_FALL) != 0) {
        uint32_t t = gpio;
        xQueueSendFromISR(xQueueBtn, &t, &y);
    }

    portYIELD_FROM_ISR(y);
}

void a_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &z_irq);
    gpio_set_irq_enabled(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true);

    TickType_t r = 0;
    TickType_t y = 0;
    uint32_t t = 0;

    while (true) {
        if (xQueueReceive(xQueueBtn, &t, portMAX_DELAY) == pdTRUE) {
            TickType_t u = xTaskGetTickCount();

            if (t == BTN_PIN_R) {
                if ((u - r) > pdMS_TO_TICKS(80)) {
                    xSemaphoreGive(xSemaphoreLedR);
                    r = u;
                }
            } else if (t == BTN_PIN_Y) {
                if ((u - y) > pdMS_TO_TICKS(80)) {
                    xSemaphoreGive(xSemaphoreLedY);
                    y = u;
                }
            }
        }
    }
}

void b_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_put(LED_PIN_R, 0);

    int t = 0;
    int u = 0;

    while (true) {
        if (t == 0) {
            gpio_put(LED_PIN_R, 0);
            if (xSemaphoreTake(xSemaphoreLedR, portMAX_DELAY) == pdTRUE) {
                t = 1;
                u = 1;
                gpio_put(LED_PIN_R, u);
            }
        } else {
            if (xSemaphoreTake(xSemaphoreLedR, pdMS_TO_TICKS(100)) == pdTRUE) {
                t = 0;
                u = 0;
                gpio_put(LED_PIN_R, 0);
            } else {
                u = !u;
                gpio_put(LED_PIN_R, u);
            }
        }
    }
}

void c_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);
    gpio_put(LED_PIN_Y, 0);

    int t = 0;
    int u = 0;

    while (true) {
        if (t == 0) {
            gpio_put(LED_PIN_Y, 0);
            if (xSemaphoreTake(xSemaphoreLedY, portMAX_DELAY) == pdTRUE) {
                t = 1;
                u = 1;
                gpio_put(LED_PIN_Y, u);
            }
        } else {
            if (xSemaphoreTake(xSemaphoreLedY, pdMS_TO_TICKS(100)) == pdTRUE) {
                t = 0;
                u = 0;
                gpio_put(LED_PIN_Y, 0);
            } else {
                u = !u;
                gpio_put(LED_PIN_Y, u);
            }
        }
    }
}

int main() {
    stdio_init_all();

    xQueueBtn = xQueueCreate(32, sizeof(uint32_t));
    xSemaphoreLedR = xSemaphoreCreateBinary();
    xSemaphoreLedY = xSemaphoreCreateBinary();

    xTaskCreate(a_task, "A", 256, NULL, 1, NULL);
    xTaskCreate(b_task, "B", 256, NULL, 1, NULL);
    xTaskCreate(c_task, "C", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {
    }

    return 0;
}
