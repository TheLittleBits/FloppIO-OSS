#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "program.pio.h"
#include "hardware/clocks.h"
#include "endstops.h"
#include <math.h>

#define BAUD_RATE 31250

// All scanner channels
// (duplicates are not allowed)
#define SCANNER1_CHANNEL 0
#define SCANNER2_CHANNEL 1
#define SCANNER3_CHANNEL 98
#define SCANNER4_CHANNEL 99


// Make a struct which contains all the scanner's data
struct Channels {
    int note;
    uint16_t pitchwheel;
    uint8_t velocity;
}; struct Channels channels[16];

void init_data() {
    // Reset all data in the "channels" struct
    for (int i = 0; i < 16; i++) {
        channels[i].velocity = 0;
        channels[i].note = 0;
        channels[i].pitchwheel = 8192;
    }
}

void init_uart() {
    // Init UART
    uart_init(uart0, BAUD_RATE);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
}

void init_sio() {
    // Init the 4 SLP pins of the DRV8825s
    gpio_init(10); gpio_set_dir(10, true);
    gpio_init(11); gpio_set_dir(11, true);
    gpio_init(12); gpio_set_dir(12, true);
    gpio_init(13); gpio_set_dir(13, true);
    // Init and turn on the onboard led
    gpio_init(25); gpio_set_dir(25, true);
    gpio_put(25, 1);
}

void scanner_program_init(PIO pio, uint sm, uint pin) {
    // Init the scanner program
    uint offset = pio_add_program(pio, &scanner_program);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
    pio_sm_config config = scanner_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 1);
    float div = (float)clock_get_hz(clk_sys) / 1e6f;
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void init_pio() {
    // Init the scanner programs
    scanner_program_init(pio0, 0, 2);
    scanner_program_init(pio0, 1, 4);
    scanner_program_init(pio0, 2, 6);
    scanner_program_init(pio0, 3, 8);
    pio_sm_set_enabled(pio0, 0, true);
    pio_sm_set_enabled(pio0, 1, true);
    pio_sm_set_enabled(pio0, 2, true);
    pio_sm_set_enabled(pio0, 3, true);
}

void stop_playing(int channel) {
    // Turn off the according DRV8825
    if (channel == SCANNER1_CHANNEL) {gpio_put(10, false);}
    if (channel == SCANNER2_CHANNEL) {gpio_put(11, false);}
    if (channel == SCANNER3_CHANNEL) {gpio_put(12, false);}
    if (channel == SCANNER4_CHANNEL) {gpio_put(13, false);}
}

void start_playing(int channel) {
    // Turn on the according DRV8825
    if (channel == SCANNER1_CHANNEL) {gpio_put(10, true);}
    if (channel == SCANNER2_CHANNEL) {gpio_put(11, true);}
    if (channel == SCANNER3_CHANNEL) {gpio_put(12, true);}
    if (channel == SCANNER4_CHANNEL) {gpio_put(13, true);}
}

void set_frequency(int channel, int freq) {
    // Load the delay value into the according TX FIFO
    if (channel == SCANNER1_CHANNEL) {pio_sm_put_blocking(pio0, 0, 1000000/freq/2);}
    if (channel == SCANNER2_CHANNEL) {pio_sm_put_blocking(pio0, 1, 1000000/freq/2);}
    if (channel == SCANNER3_CHANNEL) {pio_sm_put_blocking(pio0, 2, 1000000/freq/2);}
    if (channel == SCANNER4_CHANNEL) {pio_sm_put_blocking(pio0, 3, 1000000/freq/2);}
}

int note_to_hz(float note) {
    // Translate a midi note value to hertz
    return (int)(440.0f * pow(2.0f, ((note - 69.0f) / 12.0f)));
}

float pitch_to_note(uint16_t pitch) {
    // Translate the midi pitchbend value to midi note value
    return ((float) (((signed int) pitch - (signed int) 8192)) / 4096.0f);
}

void run_command(uint channel, uint command, uint data1, uint data2) {
    switch (command) {
        case 0: // Note Off
            // Stop playing our note
            if ((int) (data1) == (int) (channels[channel].note)) {
                stop_playing(channel);
                channels[channel].velocity = 0;
            }
            break;

        case 1: // Note On
            // Start playing a note on a specified scanner if velocity > 0
            // Otherwise, stop playing
            if (data2 == 0) {
                // Jump to the "note_off" section
                run_command(channel, 0, data1, data2);
            } else {
                // Play the note + the current pitchbend value
                channels[channel].note = data1;
                set_frequency(channel, 
                    note_to_hz(
                        ((float) (channels[channel].note) +
                        (float) (pitch_to_note(channels[channel].pitchwheel)))
                    )
                );
                start_playing(channel);
                channels[channel].velocity = data2;
            }
            break;

        case 2: // Polyphonic Pressure
            break;

        case 3: // Control Change
            // Check for All Notes Off or All Sounds Off messages
            if (data1 == 120 || data1 == 123) {
                stop_playing(channel); // Stop playing notes
            }
            break;

        case 4: // Program Change
            break;

        case 5: // Channel Pressure
            break;

        case 6: // Pitch Bend
            // The pitchwheel value of data1 and data2 combined
            channels[channel].pitchwheel = (uint16_t) (((uint16_t) data2 << 7) | ((uint8_t) data1));
            // Set the new frequency
            set_frequency(channel,
                note_to_hz(
                    ((float) (channels[channel].note) +
                    (float) (pitch_to_note(channels[channel].pitchwheel)))
                )
            );
            break;
    }
}

int main() {
    // Call all startup functions
    stdio_usb_init(); // Only because the ability of updating software without entering BOOTSEL mode manually
    init_uart();
    init_pio();
    init_sio();
    init_data();
    init_core1();
    
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