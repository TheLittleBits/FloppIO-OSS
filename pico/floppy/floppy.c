#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "program.pio.h"
#include "hardware/clocks.h"

// Set UART's baudrate
#define BAUD_RATE 31250

// All floppy drive channels
// (duplicates are not allowed)
#define FDD1_CHANNEL 2
#define FDD2_CHANNEL 3
#define FDD3_CHANNEL 4
#define FDD4_CHANNEL 5
#define FDD5_CHANNEL 6
#define FDD6_CHANNEL 7
#define FDD7_CHANNEL 8
#define FDD8_CHANNEL 10


// Make a struct which contains all the drive's data
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
    // Initialise UART
    uart_init(uart0, BAUD_RATE);
    gpio_set_function(1, GPIO_FUNC_UART);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
}

void fdd_program_init(PIO pio, uint sm, uint pin) {
    // Init one fdd program
    uint offset = pio_add_program(pio, &fdd_program);
    pio_gpio_init(pio, pin);
    pio_gpio_init(pio, pin + 1);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 2, true);
    pio_sm_config config = fdd_program_get_default_config(offset);
    sm_config_set_set_pins(&config, pin, 2);
    float div = (float)clock_get_hz(clk_sys) / 1e6f; // The clock speed has to be 1 MHz for frequency calculations
    sm_config_set_clkdiv(&config, div);
    pio_sm_init(pio, sm, offset, &config);
}

void init_pio() {
    // Init the PIO programs so they can enabled
    fdd_program_init(pio0, 0, 2);
    fdd_program_init(pio0, 1, 4);
    fdd_program_init(pio0, 2, 6);
    fdd_program_init(pio0, 3, 8);
    fdd_program_init(pio1, 0, 10);
    fdd_program_init(pio1, 1, 12);
    fdd_program_init(pio1, 2, 14);
    fdd_program_init(pio1, 3, 16);
}

void enable_pio() {
    // Enable all the PIOs.
    pio_sm_set_enabled(pio0, 0, 1);
    pio_sm_set_enabled(pio0, 1, 1);
    pio_sm_set_enabled(pio0, 2, 1);
    pio_sm_set_enabled(pio0, 3, 1);
    pio_sm_set_enabled(pio1, 0, 1);
    pio_sm_set_enabled(pio1, 1, 1);
    pio_sm_set_enabled(pio1, 2, 1);
    pio_sm_set_enabled(pio1, 3, 1);
}

void init_sio() {
    // Init all ENABLE pins of the FDDs
    gpio_init(18); gpio_set_dir(18, true);
    gpio_init(19); gpio_set_dir(19, true);
    gpio_init(20); gpio_set_dir(20, true);
    gpio_init(21); gpio_set_dir(21, true);
    gpio_init(22); gpio_set_dir(22, true);
    gpio_init(26); gpio_set_dir(26, true);
    gpio_init(27); gpio_set_dir(27, true);
    gpio_init(28); gpio_set_dir(28, true);
    // Init and turn on the onboard led
    gpio_init(25); gpio_set_dir(25, true);
    gpio_put(25, 1);
}

void stop_playing(int channel) {
    // Turn off the according FDD
    if (channel == FDD1_CHANNEL) {gpio_put(18, 0);}
    if (channel == FDD2_CHANNEL) {gpio_put(19, 0);}
    if (channel == FDD3_CHANNEL) {gpio_put(20, 0);}
    if (channel == FDD4_CHANNEL) {gpio_put(21, 0);}
    if (channel == FDD5_CHANNEL) {gpio_put(22, 0);}
    if (channel == FDD6_CHANNEL) {gpio_put(26, 0);}
    if (channel == FDD7_CHANNEL) {gpio_put(27, 0);}
    if (channel == FDD8_CHANNEL) {gpio_put(28, 0);}
}

void start_playing(int channel) {
    // Turn on the according FDD
    if (channel == FDD1_CHANNEL) {gpio_put(18, 1);}
    if (channel == FDD2_CHANNEL) {gpio_put(19, 1);}
    if (channel == FDD3_CHANNEL) {gpio_put(20, 1);}
    if (channel == FDD4_CHANNEL) {gpio_put(21, 1);}
    if (channel == FDD5_CHANNEL) {gpio_put(22, 1);}
    if (channel == FDD6_CHANNEL) {gpio_put(26, 1);}
    if (channel == FDD7_CHANNEL) {gpio_put(27, 1);}
    if (channel == FDD8_CHANNEL) {gpio_put(28, 1);}
}

void set_frequency(int channel, int freq) {
    /* Load the delay value into the according TX FIFO.
    The PIO delay code part has a "SET" instruction and a
    "JMP" instruction, so one delay loop cycle takes
    two microsecond. That's why the delay value is split in
    half. */
    if (channel == FDD1_CHANNEL) {pio_sm_put_blocking(pio0, 0, 1000000/freq/2);}
    if (channel == FDD2_CHANNEL) {pio_sm_put_blocking(pio0, 1, 1000000/freq/2);}
    if (channel == FDD3_CHANNEL) {pio_sm_put_blocking(pio0, 2, 1000000/freq/2);}
    if (channel == FDD4_CHANNEL) {pio_sm_put_blocking(pio0, 3, 1000000/freq/2);}
    if (channel == FDD5_CHANNEL) {pio_sm_put_blocking(pio1, 0, 1000000/freq/2);}
    if (channel == FDD6_CHANNEL) {pio_sm_put_blocking(pio1, 1, 1000000/freq/2);}
    if (channel == FDD7_CHANNEL) {pio_sm_put_blocking(pio1, 2, 1000000/freq/2);}
    if (channel == FDD8_CHANNEL) {pio_sm_put_blocking(pio1, 3, 1000000/freq/2);}
}

int reset() {
    #define DIRECTION_MASK 174760    // Binary: 0b00000000000101010101010101000
    #define STEP_MASK      87380     // Binary: 0b00000000000010101010101010100
    #define LED_MASK       477888512 // Binary: 0b11100011111000000000000000000
    #define ALL_MASK       478150652 // Binary: 0b11100011111111111111111111100

    // Init all used pins
    gpio_init_mask(ALL_MASK);
    gpio_set_dir_out_masked(ALL_MASK);

    // Activate all FDDs
    gpio_set_mask(LED_MASK);

    // Move the head to max position
    gpio_clr_mask(DIRECTION_MASK);

    for (int i = 0; i < 85; i++) {
        gpio_set_mask(STEP_MASK);
        sleep_ms(10);
        gpio_clr_mask(STEP_MASK);
        sleep_ms(10);
    }

    // Change direction and move to the centre
    gpio_set_mask(DIRECTION_MASK);

    for (int i = 0; i < 42; i++) {
        gpio_set_mask(STEP_MASK);
        sleep_ms(10);
        gpio_clr_mask(STEP_MASK);
        sleep_ms(10);
    }

    gpio_clr_mask(ALL_MASK);
    gpio_init_mask(ALL_MASK); // "Deinit" all the used pins
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
            // Start playing a note on a specified drive if velocity > 0
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
    reset();
    init_uart();
    init_pio();
    enable_pio();
    init_sio();
    init_data();
    
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