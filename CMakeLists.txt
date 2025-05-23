cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
set(FREERTOS_KERNEL_PATH "/home/matheus/FreeRTOS-Kernel")
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(TrafficLight C CXX ASM)
pico_sdk_init()


include_directories(${CMAKE_SOURCE_DIR}/lib)


add_executable(${PROJECT_NAME}
        src/main.c
        lib/button/button.c # Button library
        lib/led/led.c # LED library
        lib/ssd1306/ssd1306.c # SSD1306 library
        lib/ssd1306/display.c # Display library
        lib/ws2812b/ws2812b.c # WS2812B library
        lib/buzzer/buzzer.c # Buzzer library
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})

pico_generate_pio_header(${PROJECT_NAME}  ${CMAKE_CURRENT_LIST_DIR}/lib/ws2812b/pio/ws2812b.pio)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_gpio
        hardware_i2c
        hardware_pio
        hardware_pwm
        hardware_clocks
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})




