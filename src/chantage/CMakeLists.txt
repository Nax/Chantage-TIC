file(GLOB_RECURSE SOURCES "*.c" "*.cpp" "*.h" "*.hpp" "*.S")
add_library(chantage SHARED ${SOURCES})
set_target_properties(chantage PROPERTIES OUTPUT_NAME "Chantage")
