cmake_minimum_required(VERSION 3.24)

project("tinyfiledialogs"
    VERSION "3.19.1"
    LANGUAGES C
)

set(TFD_SRC ./tinyfiledialogs.c)

add_library("tinyfiledialogs" STATIC ${TFD_SRC})
target_include_directories("tinyfiledialogs" PUBLIC ./)