include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: cxlshm")

FetchContent_Declare(
        cxlshm
        GIT_REPOSITORY https://github.com/madsys-dev/sosp-paper19-ae.git
        GIT_TAG main
)

FetchContent_MakeAvailable(cxlshm)