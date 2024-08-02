# CMakeList.txt : CMake project for r-tree visualization, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (rtree "main.cpp")


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET rtree PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
  set_property(TARGET rtree PROPERTY WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(rtree PRIVATE wx::base wx::core wx::aui wx::stc)
# TODO: Add tests and install targets if needed.

add_subdirectory("back")
add_subdirectory("front")
