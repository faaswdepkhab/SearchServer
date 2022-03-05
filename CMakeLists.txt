cmake_minimum_required(VERSION 3.1)

project(SearchServer)


include_directories(include)
set(SRC "src/main.cpp")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -Wextra -Werror")

find_package(Threads REQUIRED)

add_executable(SearchServer 
    src/main.cpp
    src/document.cpp
    src/read_input_functions.cpp
    src/search_server.cpp
    src/remove_duplicates.cpp
    src/string_processing.cpp
    src/process_queries.cpp
    src/request_queue.cpp
    src/test_example_functions.cpp
)

target_link_libraries( SearchServer Threads::Threads tbb)
