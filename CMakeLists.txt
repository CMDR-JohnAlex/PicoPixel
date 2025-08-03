cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoPixel C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Options
set(STARTUP_DELAY_MS "3000" CACHE STRING "Startup delay for serial monitor attachment (milliseconds). Set to 0 to disable.")

if(STARTUP_DELAY_MS GREATER 0)
    add_compile_definitions(STARTUP_DELAY_MS=${STARTUP_DELAY_MS})
endif()

add_executable(PicoPixel
    src/main.cpp
    src/drivers/display/ili9341.cpp
    src/firmware/boot.cpp
    src/firmware/menu.cpp
    src/graphics/graphics.cpp
    src/utils/color.cpp
    src/utils/random.cpp
)

pico_set_program_name(PicoPixel "PicoPixel")
pico_set_program_version(PicoPixel "0.1")

# Enable usb output, disable uart output
pico_enable_stdio_uart(PicoPixel 0)
pico_enable_stdio_usb(PicoPixel 1)

# Add the standard library to the build
target_link_libraries(PicoPixel
    pico_stdlib
)

# Add the standard include files to the build
target_include_directories(PicoPixel PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# External libraries
add_subdirectory(lib)

# pico-vfs filesystem
pico_enable_filesystem(${CMAKE_PROJECT_NAME} FS_INIT src/firmware/fs_init.c)

# Add any user requested libraries
target_link_libraries(PicoPixel
    pico_cyw43_arch_none        # To access the on-board LED

    hardware_watchdog
    hardware_clocks             # To get system clock and USB clock speeds
    hardware_rtc                # Needed for timing?
    hardware_spi                # Hardware SPI API to communicate with the ILI9341 screen
    hardware_pwm                # Hardware PWM API to power the ILI9341 screen
    hardware_adc                # Hardware ADC API to get internal temperature, and for ADC entropy, for random numbers

    filesystem_default          # pico-vfs filesystem
    blockdevice_flash           # pico-vfs filesystem
)

pico_add_extra_outputs(PicoPixel)
