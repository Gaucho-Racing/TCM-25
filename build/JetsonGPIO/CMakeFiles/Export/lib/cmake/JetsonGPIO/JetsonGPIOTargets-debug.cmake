#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "JetsonGPIO::JetsonGPIO" for configuration "Debug"
set_property(TARGET JetsonGPIO::JetsonGPIO APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(JetsonGPIO::JetsonGPIO PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libJetsonGPIO.so.1.2.5"
  IMPORTED_SONAME_DEBUG "libJetsonGPIO.so.1"
  )

list(APPEND _IMPORT_CHECK_TARGETS JetsonGPIO::JetsonGPIO )
list(APPEND _IMPORT_CHECK_FILES_FOR_JetsonGPIO::JetsonGPIO "${_IMPORT_PREFIX}/lib/libJetsonGPIO.so.1.2.5" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
