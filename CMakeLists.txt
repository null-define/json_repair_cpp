cmake_minimum_required(VERSION 3.12)
project(json_parser)

add_library(json_parser
    json_repair/json_parser.cpp
    json_repair/json_context.cpp
    json_repair/parse_array.cpp
    json_repair/parse_object.cpp
    json_repair/parse_number.cpp
    json_repair/parse_string.cpp
    json_repair/parse_comment.cpp
    json_repair/string_file_wrapper.cpp
)
target_include_directories(json_parser PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_features(json_parser PUBLIC cxx_std_17)


add_executable(json_repair_cli test/cli/json_repair_cli.cpp)
target_link_libraries(json_repair_cli json_parser)