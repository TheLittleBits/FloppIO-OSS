# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hdd C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(hdd hdd.c)

pico_set_program_name(hdd "hdd")
pico_set_program_version(hdd "0.1")

# no_flash means the target is to run from RAM
#pico_set_binary_type(hdd no_flash)

# Generate PIO header
pico_generate_pio_header(hdd ${CMAKE_CURRENT_LIST_DIR}/program.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(hdd 1)
pico_enable_stdio_usb(hdd 1)

# Add the standard include files to the build
target_include_directories(hdd PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(hdd
        pico_stdlib
        hardware_pio
        )

pico_add_extra_outputs(hdd)

