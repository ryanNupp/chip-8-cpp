option(BUILD_SHARED_LIBS "Build using static libraries" off)

# SDL3
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST OFF)

add_subdirectory(SDL)
add_library(external INTERFACE)
target_link_libraries(external INTERFACE SDL3::SDL3)

# tinyfiledialogs
add_subdirectory(tinyfiledialogs)
target_link_libraries(external INTERFACE tinyfiledialogs)
