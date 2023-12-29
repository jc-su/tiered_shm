include(FetchContent)
set(FETCHCONTENT_QUIET ON)

message(STATUS "Cloning External Project: cxlshm")

FetchContent_Declare(
        lightning
        GIT_REPOSITORY https://github.com/danyangz/lightning.git
        GIT_TAG master
)

FetchContent_MakeAvailable(lightning)