include(FetchContent)

#- pico-vfs -----------------------------------------------------------------

message("Fetching pico-vfs")

FetchContent_Declare(
    pico-vfs
    GIT_REPOSITORY https://github.com/oyama/pico-vfs.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
    GIT_SUBMODULE  ""
)

FetchContent_MakeAvailable(pico-vfs)
