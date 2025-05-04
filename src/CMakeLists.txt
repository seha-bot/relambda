# Common compiler options
add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_23)
target_compile_options(compiler_flags INTERFACE -Wall -Wextra -Wpedantic -Wunused -Wconversion -Wsign-conversion)

# Core library for linking with all targets
add_library(relambda_core INTERFACE)
target_sources(relambda_core
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS include
)
target_link_libraries(relambda_core INTERFACE compiler_flags)

include(FetchContent)

# FetchContent_Declare(argparse
#     GIT_REPOSITORY https://github.com/p-ranav/argparse.git
#     GIT_TAG d924b84eba1f0f0adf38b20b7b4829f6f65b6570
# )
# FetchContent_MakeAvailable(argparse)

FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
FetchContent_MakeAvailable(lexy)

# Parsing library for ease of linking with tests
add_library(relambda_parsing INTERFACE)
target_link_libraries(relambda_parsing INTERFACE relambda_core foonathan::lexy)
target_sources(relambda_parsing INTERFACE parser.cpp)

add_executable(relambda main.cpp)
target_link_libraries(relambda PRIVATE relambda_parsing)
