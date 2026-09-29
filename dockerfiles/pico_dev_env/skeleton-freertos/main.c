#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

void blink_task(void *params) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(250));
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

int main() {
    xTaskCreate(blink_task, "blink", 256, NULL, 1, NULL);
    vTaskStartScheduler();
    while (true) {}
}
