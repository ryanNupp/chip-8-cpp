# static link
option(BUILD_SHARED_LIBS "Build using static libraries" off)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST OFF)

add_subdirectory(SDL)

add_library(external INTERFACE)
target_link_libraries(external INTERFACE SDL3::SDL3)
