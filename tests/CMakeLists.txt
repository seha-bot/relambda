include(FetchContent)

message(STATUS "Fetching Catch2...")
FetchContent_Declare(Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG 914aeecfe23b1e16af6ea675a4fb5dbd5a5b8d0a
	SOURCE_DIR Catch2
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests tests_main.cpp)
target_link_libraries(tests PRIVATE mliti_core Catch2::Catch2WithMain)

add_test(NAME tests COMMAND tests)
