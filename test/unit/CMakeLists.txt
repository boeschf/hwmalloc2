

add_executable(unit resources.cpp)
target_link_libraries(unit PUBLIC hwmalloc2)
target_link_libraries(unit PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)

catch_discover_tests(unit)
