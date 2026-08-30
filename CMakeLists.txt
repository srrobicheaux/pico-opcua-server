#https://github.com/raspberrypi/pico-examples/tree/master/pico_w/wifi/freertos

cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w CACHE STRING "Board type")

set(FREERTOS_KERNEL_PATH "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel")
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "")

include(pico_sdk_import.cmake)

project(pico-opcua-server C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS_Kernel_import.cmake")
    include(${CMAKE_CURRENT_LIST_DIR}/FreeRTOS_Kernel_import.cmake)
elseif(EXISTS "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake")
    include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/FreeRTOS_Kernel_import.cmake)
else()
    message(FATAL_ERROR "FreeRTOS_Kernel_import.cmake not found")
endif()

set(PICO_LWIP_OPTS_DIR ${CMAKE_CURRENT_LIST_DIR})

# ---------- open62541 configuration (BEFORE add_subdirectory) ----------
set(UA_ARCHITECTURE "freertosLWIP" CACHE STRING "Use FreeRTOS architecture")
set(UA_ENABLE_AMALGAMATION    OFF        CACHE BOOL   "" FORCE) 
set(UA_MULTITHREADING         "0"        CACHE STRING "" FORCE)
set(UA_BUILD_UNIT_TESTS      OFF        CACHE BOOL   "" FORCE)
set(UA_BUILD_EXAMPLES        OFF        CACHE BOOL   "" FORCE)
set(UA_ENABLE_INSTALLATION   OFF        CACHE BOOL   "" FORCE)
set(UA_ENABLE_ENCRYPTION     OFF        CACHE BOOL   "" FORCE)
set(UA_NAMESPACE_ZERO        "MINIMAL"  CACHE STRING "" FORCE)
set(UA_ENABLE_PUBSUB         OFF        CACHE BOOL   "" FORCE)
set(UA_LWIP_EXTERNAL         ON         CACHE BOOL   "" FORCE)
set(UA_ARCHITECTURE_LWIP     TRUE       CACHE BOOL   "" FORCE)
set(UA_LOGLEVEL              "500"      CACHE STRING "" FORCE)

set(LWIP_INCLUDE_DIRS
    "${PICO_SDK_PATH}/lib/lwip/src/include"
    "${PICO_SDK_PATH}/lib/lwip/contrib/ports/freertos/include"
    "${PICO_SDK_PATH}/src/rp2_common/pico_lwip/include"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    CACHE PATH "" FORCE
)
 
# Global compiler fallback macros
add_compile_definitions(
    UA_HAS_LWIP=1
    EAFNOSUPPORT=106
    CLOCK_MONOTONIC=1
    CLOCK_REALTIME=0
    __STDC_FORMAT_MACROS
    PRIu64="llu" # Forces formatting validation on strict compilers
)

# ---------- open62541 configuration & targets ----------
add_subdirectory(open62541)

 target_link_libraries(open62541-plugins PRIVATE
        pico_stdlib
        pico_cyw43_arch_lwip_sys_freertos
        pico_rand
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
    )
 target_link_libraries(open62541-object PRIVATE
        pico_stdlib
        pico_cyw43_arch_lwip_sys_freertos
        pico_rand
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
    )
    
add_executable(${PROJECT_NAME}
    main.c
    pico_ua_time.c
    pico_lwip_posix_fixes.c
    hardware.c
    opc_handlers.c
    ${PICO_SDK_PATH}/lib/lwip/src/apps/sntp/sntp.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    pico_stdlib
    pico_multicore
    pico_cyw43_arch_lwip_sys_freertos
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    open62541
    hardware_adc
    pico_lwip_mdns
    pico_lwip_netbios
)

target_compile_options(${PROJECT_NAME} PRIVATE -w)
target_compile_options(open62541-plugins PRIVATE -w)
target_compile_options(open62541-object PRIVATE -w)
target_compile_definitions(${PROJECT_NAME} PRIVATE UA_ARCHITECTURE_FREERTOSLWIP)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/open62541
    ${CMAKE_CURRENT_SOURCE_DIR}/open62541/include
    ${CMAKE_CURRENT_SOURCE_DIR}/open62541/plugins/include
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure

)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})