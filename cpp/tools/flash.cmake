if (NOT PICOTOOL_EXECUTABLE OR NOT EXISTS "${PICOTOOL_EXECUTABLE}")
  message(FATAL_ERROR "picotool not found. Install it or set -DPICOTOOL_EXECUTABLE=/path/to/picotool")
endif()

execute_process(
  COMMAND "${PICOTOOL_EXECUTABLE}" load "${TARGET_FILE}" -f
  RESULT_VARIABLE PICOTOOL_RESULT
)

if (NOT PICOTOOL_RESULT EQUAL 0)
  message(FATAL_ERROR "picotool failed with code ${PICOTOOL_RESULT}")
endif()
