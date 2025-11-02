file(GLOB_RECURSE SOURCES "*.c" "*.cpp" "*.h" "*.hpp")
add_executable(loader ${SOURCES})
set_target_properties(loader PROPERTIES OUTPUT_NAME "Chantage")
