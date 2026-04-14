#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t q1;
QueueHandle_t q2;
QueueHandle_t q3;
QueueHandle_t q4;

void z_irq(uint gpio, uint32_t events) {
    BaseType_t y = pdFALSE;
    uint32_t t = 1;

    if ((events & GPIO_IRQ_EDGE_FALL) != 0) {
        if (gpio == BTN_PIN_R) {
            xQueueSendFromISR(q3, &t, &y);
        } else if (gpio == BTN_PIN_G) {
            xQueueSendFromISR(q4, &t, &y);
        }
    }

    portYIELD_FROM_ISR(y);
}

void a_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int t = 0;

    while (true) {
        if (xQueueReceive(q1, &t, 0)) {
            printf("%d\n", t);
        }

        if (t > 0) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(t));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(t));
        }
    }
}

void b_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true,
                                       &z_irq);

    int t = 0;
    uint32_t e = 0;
    while (true) {
        if (xQueueReceive(q3, &e, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (t < 1000) {
                t += 100;
            } else {
                t = 100;
            }
            printf("delay btn r %d \n", t);
            xQueueSend(q1, &t, 0);
        }
    }
}

void c_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int u = 0;

    while (true) {
        if (xQueueReceive(q2, &u, 0)) {
            printf("%d\n", u);
        }

        if (u > 0) {
            gpio_put(LED_PIN_G, 1);
            vTaskDelay(pdMS_TO_TICKS(u));
            gpio_put(LED_PIN_G, 0);
            vTaskDelay(pdMS_TO_TICKS(u));
        }
    }
}

void d_task(void *p) {
    gpio_init(BTN_PIN_G);
    gpio_set_dir(BTN_PIN_G, GPIO_IN);
    gpio_pull_up(BTN_PIN_G);
    gpio_set_irq_enabled_with_callback(BTN_PIN_G, GPIO_IRQ_EDGE_FALL, true,
                                       &z_irq);

    int u = 0;
    uint32_t f = 0;
    while (true) {
        if (xQueueReceive(q4, &f, pdMS_TO_TICKS(500)) == pdTRUE) {
            if (u < 1000) {
                u += 100;
            } else {
                u = 100;
            }
            printf("delay btn g %d \n", u);
            xQueueSend(q2, &u, 0);
        }
    }
}

int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    q1 = xQueueCreate(32, sizeof(int));
    q2 = xQueueCreate(32, sizeof(int));
    q3 = xQueueCreate(32, sizeof(uint32_t));
    q4 = xQueueCreate(32, sizeof(uint32_t));

    xTaskCreate(a_task, "A", 256, NULL, 1, NULL);
    xTaskCreate(b_task, "B", 256, NULL, 1, NULL);
    xTaskCreate(c_task, "C", 256, NULL, 1, NULL);
    xTaskCreate(d_task, "D", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
