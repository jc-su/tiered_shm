set(EBPF_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/mmap_monitor.bpf.o)
set(EBPF_SKELETON ${CMAKE_CURRENT_BINARY_DIR}/mmap_monitor.skel.h)
set(VMLINUX_HEADER ${CMAKE_CURRENT_BINARY_DIR}/vmlinux.h)

# Generate vmlinux.h
add_custom_command(
    OUTPUT ${VMLINUX_HEADER}
    COMMAND echo "Generating vmlinux.h"
    COMMAND bpftool btf dump file /sys/kernel/btf/vmlinux format c > ${VMLINUX_HEADER}
    COMMAND echo "vmlinux.h is generated at ${CMAKE_CURRENT_BINARY_DIR}"
)

# Compile eBPF program
add_custom_command(
    OUTPUT ${EBPF_OBJECT}
    COMMAND echo "Compiling mmap_monitor.bpf.c"
    COMMAND cc -O2 -target bpf -I${CMAKE_CURRENT_BINARY_DIR} -c ${CMAKE_SOURCE_DIR}/src/mmap_monitor.bpf.c -o ${EBPF_OBJECT}
    COMMAND echo "mmap_monitor.bpf.o is generated at ${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ${CMAKE_SOURCE_DIR}/src/mmap_monitor.bpf.c ${VMLINUX_HEADER}
)

# Generate BPF skeleton
add_custom_command(
    OUTPUT ${EBPF_SKELETON}
    COMMAND echo "Generating mmap_monitor.skel.h from mmap_monitor.bpf.o"
    COMMAND bpftool gen skeleton ${EBPF_OBJECT} > ${EBPF_SKELETON}
    COMMAND echo "mmap_monitor.skel.h is generated at ${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS ${EBPF_OBJECT}
)

# Custom target that depends on the eBPF object and skeleton
add_custom_target(mmap_monitor ALL DEPENDS ${EBPF_OBJECT} ${EBPF_SKELETON} ${VMLINUX_HEADER})

add_library(utils STATIC)
# add_library(shm_wrapper)

target_sources(utils
        PUBLIC
        FILE_SET CXX_MODULES FILES
        utils.cppm
)

target_include_directories(utils PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
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