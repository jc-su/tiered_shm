add_custom_command(
    OUTPUT mmap_monitor.bpf.o
    COMMAND cc -O2 -target bpf -c ${CMAKE_CURRENT_SOURCE_DIR}/mmap_monitor.bpf.c -o ${CMAKE_CURRENT_SOURCE_DIR}/mmap_monitor.bpf.o
    DEPENDS mmap_monitor.bpf.c
)

add_custom_command(
    OUTPUT mmap_monitor.skel.h
    COMMAND bpftool gen skeleton ${CMAKE_CURRENT_SOURCE_DIR}/mmap_monitor.bpf.o > ${CMAKE_CURRENT_SOURCE_DIR}/mmap_monitor.skel.h
    DEPENDS mmap_monitor.bpf.o
)

add_library(utils)
# add_library(shm_wrapper)

target_sources(utils
        PUBLIC
        FILE_SET CXX_MODULES FILES
        utils.cppm
)

target_link_directories(
        utils
        PRIVATE
        bpf
)

# target_sources(shm_wrapper
#         PUBLIC
#         FILE_SET CXX_MODULES FILES
#         shm_wrapper.cppm
# )

# target_include_directories(shm_wrapper
#         PRIVATE
#         ${cxlshm_SOURCE_DIR}/include
#         ${lightning_SOURCE_DIR}/inc
# )

# target_link_libraries(shm_wrapper
#         PRIVATE
#         utils
#         lightning
#         cxlmalloc
# )