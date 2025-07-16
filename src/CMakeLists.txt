add_library(mountkit STATIC mountkit.cpp)
target_include_directories(mountkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})