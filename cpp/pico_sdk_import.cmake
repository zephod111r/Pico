# Import the Raspberry Pi Pico SDK (requires PICO_SDK_PATH).
if (DEFINED ENV{PICO_SDK_PATH})
  set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if (NOT PICO_SDK_PATH)
  message(FATAL_ERROR "PICO_SDK_PATH not set")
endif()

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
