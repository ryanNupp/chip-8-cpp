cmake_minimum_required(VERSION 3.24)

# add source to this project's executable
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    window.cc
    chip8.cc
    main.cc
)

# copy assets directory to build directory
# add_custom_target(copy_assets
#     COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
# )
# add_dependencies(${PROJECT_NAME} copy_assets)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME} PRIVATE external)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall)