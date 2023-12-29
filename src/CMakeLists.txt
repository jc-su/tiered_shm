add_library(utils)
add_library(shm_wrapper)

target_sources(utils
        PUBLIC
        FILE_SET CXX_MODULES FILES
        utils.cppm
)

target_sources(shm_wrapper
        PUBLIC
        FILE_SET CXX_MODULES FILES
        shm_wrapper.cppm
)

target_include_directories(shm_wrapper
        PRIVATE
        ${cxlshm_SOURCE_DIR}/include
        ${lightning_SOURCE_DIR}/inc
)

target_link_libraries(shm_wrapper
        PRIVATE
        lightning
        cxlmalloc
)