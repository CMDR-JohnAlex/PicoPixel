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

add_executable(PicoPixel
    src/PicoPixel.cpp
    #src/fs_init.c
    src/ili9341.cpp
    src/fonts/RobotoMono-SemiBold.c
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
    ${CMAKE_CURRENT_LIST_DIR}
)

# External libraries
add_subdirectory(lib)

# pico-vfs filesystem
pico_enable_filesystem(${CMAKE_PROJECT_NAME} FS_INIT src/fs_init.c)

# Add any user requested libraries
target_link_libraries(PicoPixel
    pico_cyw43_arch_none        # We need Wifi to access the GPIO?
    hardware_timer
    hardware_watchdog
    hardware_clocks
    hardware_rtc                # Needed for timing?
    filesystem_default
    blockdevice_flash
    hardware_spi                # Screen
    hardware_pwm                # Screen
) # TODO: Clean up un-needed, or comment their use.

pico_add_extra_outputs(PicoPixel)



add_compile_definitions(WAIT_ON_STARTUP=1)