project(ubuntu_platform_sdk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

include_directories(include/)

add_subdirectory(src/)
add_subdirectory(tests/)

install(
  DIRECTORY include/ubuntu
  DESTINATION include
)