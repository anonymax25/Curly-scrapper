cmake_minimum_required(VERSION 3.15)
project(test_curl C)

set(CMAKE_C_STANDARD 99)

add_executable(test_curl main.c)

find_package(CURL)
if (CURL_FOUND)
    message("Curl found")
    target_include_directories(test_curl PUBLIC ${CURL_INCLUDE_DIRS})
    target_link_libraries(test_curl PUBLIC ${CURL_LIBRARIES})
else ()
    message(Curl not found)
endif ()