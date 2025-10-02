#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "program.pio.h"
#include "hardware/clocks.h"

#define BAUD_RATE 31250
#define HDD_CLICK_TIME 1000

// All hdd note definitions
#define HDD1_NOTE 35
#define HDD2_NOTE 36
#define HDD3_NOTE 37
#define HDD4_NOTE 38
#define HDD5_NOTE 39
#define HDD6_NOTE 40
#define HDD7_NOTE 41
#define HDD8_NOTE 42

void init_uart() {
    // Init UART
    uart_init(uart0, BAUD_RATE);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
}

void init_sio() {
    // Init and turn on the onboard led
    gpio_init(25); gpio_set_dir(25, true);
    gpio_put(25, 1);
}

void hdd_program_init(PIO pio, uint sm, uint pin) {
    // Init one HDD program
    uint offset = pio_add_program(pio, &hdd_program);
    pio_gpio_init(pio, pin);
    pio_gpio_init(pio, pin + 1);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, true);
    pio_sm_config config = hdd_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 2);
    float div = (float)clock_get_hz(clk_sys) / 1e6f;
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void init_pio() {
    /* Initialises the HDD driving programs */

    // Load the program into the state machine
    hdd_program_init(pio0, 0, 2);
    hdd_program_init(pio0, 1, 4);
    hdd_program_init(pio0, 2, 6);
    hdd_program_init(pio0, 3, 8);
    hdd_program_init(pio1, 0, 10);
    hdd_program_init(pio1, 1, 12);
    hdd_program_init(pio1, 2, 14);
    hdd_program_init(pio1, 3, 16);
    // Load the click time into the OSR
    pio_sm_put_blocking(pio0, 0, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio0, 1, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio0, 2, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio0, 3, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio1, 0, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio1, 1, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio1, 2, HDD_CLICK_TIME);
    pio_sm_put_blocking(pio1, 3, HDD_CLICK_TIME);
    // Enable the HDDs
    pio_sm_set_enabled(pio0, 0, true);
    pio_sm_set_enabled(pio0, 1, true);
    pio_sm_set_enabled(pio0, 2, true);
    pio_sm_set_enabled(pio0, 3, true);
    pio_sm_set_enabled(pio1, 0, true);
    pio_sm_set_enabled(pio1, 1, true);
    pio_sm_set_enabled(pio1, 2, true);
    pio_sm_set_enabled(pio1, 3, true);
}

void hdd_click(uint note) {
    /* Deblock the according pio program so it toggles the H-bridge. */

    switch (note) {
        case HDD1_NOTE: pio_sm_put_blocking(pio0, 0, HDD_CLICK_TIME); break;
        case HDD2_NOTE: pio_sm_put_blocking(pio0, 1, HDD_CLICK_TIME); break;
        case HDD3_NOTE: pio_sm_put_blocking(pio0, 2, HDD_CLICK_TIME); break;
        case HDD4_NOTE: pio_sm_put_blocking(pio0, 3, HDD_CLICK_TIME); break;
        case HDD5_NOTE: pio_sm_put_blocking(pio1, 4, HDD_CLICK_TIME); break;
        case HDD6_NOTE: pio_sm_put_blocking(pio1, 5, HDD_CLICK_TIME); break;
        case HDD7_NOTE: pio_sm_put_blocking(pio1, 6, HDD_CLICK_TIME); break;
        case HDD8_NOTE: pio_sm_put_blocking(pio1, 7, HDD_CLICK_TIME); break;
    }
}

void run_command(uint channel, uint command, uint data1, uint data2) {
    switch (command) {
        case 0: // Note Off
            break;

        case 1: // Note On
            // Make a click sound on the specified hdd
            if (channel == 9) {
                if (data2 > 0) {
                    hdd_click(data1);
                }
            }
            break;

        case 2: // Polyphonic Pressure
            break;

        case 3: // Control Change
            break;

        case 4: // Program Change
            break;

        case 5: // Channel Pressure
            break;

        case 6: // Pitch Bend
            break;
    }
}

int main() {
    // Call all startup functions
    stdio_usb_init(); // Only because the ability of updating software without entering BOOTSEL mode manually
    init_uart();
    init_pio();
    init_sio();
    
    // Here is the status stored
    uint8_t status;
    uint8_t status_MSB;
    uint8_t command;
    uint8_t channel;
    // Here is data1 stored
    uint8_t data1;
    uint8_t data1_MSB;
    // Here is data2 stored
    uint8_t data2;
    uint8_t data2_MSB;

    for (;;) {
        // Get the status byte
        status = uart_getc(uart0);
        status_MSB = (status >> 7u) & 1u;
        command = (status >> 4u) & 7u;
        channel = status & 15u;

        // Check if "status" is really an status byte and has MSB 1
        if (status_MSB == 1) {
            // Get data1 byte
            data1 = uart_getc(uart0);
            data1_MSB = (data1 >> 7u) & 1u;
            // Then check if data1 is a data byte and has MSB 0
            // Also check if data2 is required
            if (data1_MSB == 0) {
                if (command != 4 && command != 5) {
                    // Get data2 byte
                    data2 = uart_getc(uart0);
                    data2_MSB = (data2 >> 7u) & 1u;
                    // Data2 also has to be a data byte
                    if (data2_MSB == 0) {
                        // Run the command received
                        run_command(channel, command, data1, data2);
                    }
                } else {
                    // Run the command received
                    run_command(channel, command, data1, 0);
                }
            }
        }
    }
}