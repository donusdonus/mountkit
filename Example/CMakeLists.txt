cmake_minimum_required(VERSION 3.14)
project(mountkitExample)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

# ✅ FreeRTOSConfig.h สำหรับ build freertos
#add_library(freertos_config INTERFACE)
#target_include_directories(freertos_config INTERFACE
#  ${CMAKE_CURRENT_SOURCE_DIR}  # FreeRTOSConfig.h อยู่ที่นี่
#)


#include(FetchContent)
#FetchContent_Declare(
#  freertos
#  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
#  GIT_TAG main
#  UPDATE_DISCONNECTED TRUE
#)
#FetchContent_MakeAvailable(freertos)

# ✅ เพิ่มไลบรารี mountkit
add_subdirectory(../src ${CMAKE_CURRENT_BINARY_DIR}/mountkit)

# ✅ สร้าง executable
add_executable(app main.cpp)

# ✅ รวม FreeRTOS kernel source
#target_sources(app PRIVATE
#  ${freertos_SOURCE_DIR}/list.c
#  ${freertos_SOURCE_DIR}/queue.c
#  ${freertos_SOURCE_DIR}/tasks.c
#  ${freertos_SOURCE_DIR}/timers.c
#  ${freertos_SOURCE_DIR}/event_groups.c
#  ${freertos_SOURCE_DIR}/portable/MemMang/heap_4.c
#  ${freertos_SOURCE_DIR}/portable/MSVC-MingW/port.c
#)

# ✅ include path
#target_include_directories(app PRIVATE
#  ${freertos_SOURCE_DIR}/include
#  ${freertos_SOURCE_DIR}/portable/MSVC-MingW
#)

# ✅ link libraries
target_link_libraries(app
  PRIVATE
  mountkit
#  freertos_kernel
)
