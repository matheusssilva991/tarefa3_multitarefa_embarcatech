#include "led.h"

void init_led(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

void init_leds() {
    init_led(GREEN_LED_PIN);
    init_led(BLUE_LED_PIN);
    init_led(RED_LED_PIN);
}
