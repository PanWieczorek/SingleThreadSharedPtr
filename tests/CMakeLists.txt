Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview3
)

FetchContent_MakeAvailable(Catch2)

add_executable(single_thread_shared_ptr_tests
    single_thread_shared_ptr_counter_test.cpp
    assign.cpp
    cons.cpp
    comparisions.cpp
    modifiers.cpp
    observers.cpp
    hash.cpp
)

target_link_libraries(single_thread_shared_ptr_tests
    PRIVATE
        SingleThreadSharedPtr::SingleThreadSharedPtr
        Catch2::Catch2WithMain
)
