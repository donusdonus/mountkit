add_library(Mountkit STATIC Mountkit.cpp)
target_include_directories(Mountkit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})