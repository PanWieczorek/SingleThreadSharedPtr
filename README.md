## About

A NON THREAD SAFE, crippled (no week_ptr for now, no way of storing a custom dtor) c++17 (but can be easily changes to be 1++14) version of std::shared_ptr
design to be as compiler friendly as possible (a lot of use cases are optimized out)

## Performance

Benchmarks for create / copy / move operations show 2x increase in performance

## Install

SingleThreadSharedPtr is a header only library, so installation can be performed as a simple copy of the include file.

Another possibility is to use [FetchContent]
(https://cmake.org/cmake/help/latest/module/FetchContent.html):
```cmake
Include(FetchContent)

FetchContent_Declare(
  SingleThreadSharedPtr
  GIT_REPOSITORY https://github.com/PanWieczorek/SingleThreadSharedPtr.git
  GIT_TAG main
)

FetchContent_MakeAvailable(SingleThreadSharedPtr)

add_executable(app myapp.cpp)
target_link_libraries(app PRIVATE SingleThreadSharedPtr::SingleThreadSharedPtr)
```
