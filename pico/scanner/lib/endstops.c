#include "endstops.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"

void init_core1() {
    // Start the direction manager core
    multicore_launch_core1(&endstops);
}

void endstops() {
    // An array with the ignored switches (they need to be ignored if being held)
    bool ignored_switches[4] = {0, 0, 0, 0};
    
    // Init the direction gpios (outputs)
    // (It al so looks cool to let them go in different directions at startup, that's why 1-0-1-0)
    gpio_init(3); gpio_set_dir(3, true); gpio_put(3, 0);
    gpio_init(5); gpio_set_dir(5, true); gpio_put(5, 1);
    gpio_init(7); gpio_set_dir(7, true); gpio_put(7, 0);
    gpio_init(9); gpio_set_dir(9, true); gpio_put(9, 1);
    // Init the endstop gpios (inputs)
    gpio_init(14); gpio_set_dir(14, false);
    gpio_init(15); gpio_set_dir(15, false);
    gpio_init(16); gpio_set_dir(16, false);
    gpio_init(17); gpio_set_dir(17, false);

    for (;;) {
        // Change direction in case of endstops activated
        if (gpio_get(14) && !ignored_switches[0]) {
            gpio_put(3, !gpio_get_out_level(3));
            ignored_switches[0] = true;
        }
        if (gpio_get(15) && !ignored_switches[1]) {
            gpio_put(5, !gpio_get_out_level(5));
            ignored_switches[1] = true;
        }
        if (gpio_get(16) && !ignored_switches[2]) {
            gpio_put(7, !gpio_get_out_level(7));
            ignored_switches[2] = true;
        }
        if (gpio_get(17) && !ignored_switches[3]) {
            gpio_put(9, !gpio_get_out_level(9));
            ignored_switches[3] = true;
        }

        // Also heck for switch release
        if (!gpio_get(14)) {
            ignored_switches[0] = false;
        }
        if (!gpio_get(15)) {
            ignored_switches[1] = false;
        }
        if (!gpio_get(16)) {
            ignored_switches[2] = false;
        }
        if (!gpio_get(17)) {
            ignored_switches[3] = false;
        }
        // Little delay against core stress
        sleep_ms(1);
    }
}