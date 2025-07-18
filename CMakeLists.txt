# Generated Cmake Pico project file

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

cmake_minimum_required(VERSION 3.13)


# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(freeRTOS_mqtt_conn C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(freeRTOS_mqtt_conn 
freeRTOS_mqtt_conn.c
src/led.c
src/mqtt_comm.c
src/wifi_conn.c
)

target_include_directories(freeRTOS_mqtt_conn PRIVATE ${CMAKE_CURRENT_LIST_DIR})
 
# pull in common dependencies
target_link_libraries(freeRTOS_mqtt_conn
pico_stdlib
FreeRTOS-Kernel
FreeRTOS-Kernel-Heap4
hardware_adc
hardware_i2c
# Oferece suporte Wi-Fi (CYW43) com LwIP (pilha TCP/IP) em um modo thread-safe em segundo plano.
pico_cyw43_arch_lwip_threadsafe_background
# Adiciona suporte a cliente MQTT sobre o LwIP.
pico_lwip_mqtt
# A pilha Lightweight IP (LwIP), que fornece funcionalidades TCP/IP (HTTP, MQTT, UDP, DHCP, etc.).
pico_lwip
# O driver do chip Wi-Fi CYW43 (usado no Pico W).
pico_cyw43_driver
)

pico_enable_stdio_uart(freeRTOS_mqtt_conn 0)
pico_enable_stdio_usb(freeRTOS_mqtt_conn 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(freeRTOS_mqtt_conn)