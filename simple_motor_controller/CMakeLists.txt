cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(simple_motor_controller C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(simple_motor_controller
  src/main.cpp
  src/pico_robotics.cpp
  src/quaternion_robot.cpp
)

target_include_directories(simple_motor_controller PRIVATE include)

target_link_libraries(simple_motor_controller
  pico_stdlib
  hardware_i2c
  hardware_irq
  pico_i2c_slave
)

pico_enable_stdio_usb(simple_motor_controller 1)
pico_enable_stdio_uart(simple_motor_controller 0)

pico_add_extra_outputs(simple_motor_controller)
