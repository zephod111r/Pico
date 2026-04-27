# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_robot_http C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_robot_http
  src/main.cpp
  src/http_server.cpp
  src/pico_robotics.cpp
  src/quaternion_robot.cpp
  src/ble_provision.cpp
  src/wifi_config.cpp
)

target_include_directories(pico_robot_http PRIVATE
  include
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/generated/pico_robot_http_gatt_header
)

target_link_libraries(pico_robot_http
  pico_stdlib
  hardware_i2c
  hardware_flash
  hardware_sync
  pico_btstack_base
  pico_btstack_ble
  pico_btstack_cyw43
  pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(pico_robot_http 1)
pico_enable_stdio_uart(pico_robot_http 0)

pico_btstack_make_gatt_header(pico_robot_http PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/wifi_provision.gatt)

pico_add_extra_outputs(pico_robot_http)

find_program(PICOTOOL_EXECUTABLE picotool)
set(PICOTOOL_EXECUTABLE "${PICOTOOL_EXECUTABLE}" CACHE FILEPATH "Path to picotool")

add_custom_target(flash
  COMMAND ${CMAKE_COMMAND}
          -DPICOTOOL_EXECUTABLE=${PICOTOOL_EXECUTABLE}
          -DTARGET_FILE=$<TARGET_FILE:pico_robot_http>
          -P ${CMAKE_CURRENT_LIST_DIR}/tools/flash.cmake
  DEPENDS pico_robot_http
  COMMENT "Flashing Pico via picotool"
  VERBATIM
)
