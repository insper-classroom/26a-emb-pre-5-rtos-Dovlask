#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

const int BTN_PIN_R = 28;
const int BTN_PIN_G = 26;

const int LED_PIN_R = 4;
const int LED_PIN_G = 6;

QueueHandle_t qx;
QueueHandle_t qy;

void a_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int t = 0;
    while (true) {
        if (xQueueReceive(qx, &t, 0)) {
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

    int t = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_R)) {

            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (t < 1000) {
                t += 100;
            } else {
                t = 100;
            }
            printf("delay btn r %d \n", t);
            xQueueSend(qx, &t, 0);
        }
    }
}

void c_task(void *p) {
    gpio_init(LED_PIN_G);
    gpio_set_dir(LED_PIN_G, GPIO_OUT);

    int u = 0;
    while (true) {
        if (xQueueReceive(qy, &u, 0)) {
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

    int u = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_G)) {

            while (!gpio_get(BTN_PIN_G)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            if (u < 1000) {
                u += 100;
            } else {
                u = 100;
            }
            printf("delay btn g %d \n", u);
            xQueueSend(qy, &u, 0);
        }
    }
}


int main() {
    stdio_init_all();
    printf("Start RTOS \n");

    qx = xQueueCreate(32, sizeof(int));
    qy = xQueueCreate(32, sizeof(int));

    xTaskCreate(a_task, "A", 256, NULL, 1, NULL);
    xTaskCreate(b_task, "B", 256, NULL, 1, NULL);
    xTaskCreate(c_task, "C", 256, NULL, 1, NULL);
    xTaskCreate(d_task, "D", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
