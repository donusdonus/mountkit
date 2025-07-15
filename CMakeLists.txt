add_executable(app main.cpp)

target_sources(app PRIVATE
  ${freertos_SOURCE_DIR}/list.c
  ${freertos_SOURCE_DIR}/queue.c
  ${freertos_SOURCE_DIR}/tasks.c
  ${freertos_SOURCE_DIR}/timers.c
  ${freertos_SOURCE_DIR}/event_groups.c
  ${freertos_SOURCE_DIR}/portable/MemMang/heap_4.c
  ${freertos_SOURCE_DIR}/portable/MSVC-MingW/port.c
)

target_include_directories(app PRIVATE
  ${freertos_SOURCE_DIR}/include
  ${freertos_SOURCE_DIR}/portable/MSVC-MingW
)

target_link_libraries(app
  PRIVATE
  Mountkit
  freertos_kernel
)
