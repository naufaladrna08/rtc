cmake_minimum_required(VERSION 3.10.0)
project(rtc VERSION 0.1.0 LANGUAGES C)

add_executable(rtc main.c)

target_link_libraries(rtc -lm)