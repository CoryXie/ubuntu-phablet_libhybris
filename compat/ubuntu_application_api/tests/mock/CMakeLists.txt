add_executable(
  ubuntu_mock_platform_test

  ../test_window_session_service.cpp
)

add_executable(
  ubuntu_mock_platform_c_api_test

  ../test_window_session_service_c_api.cpp
)

target_link_libraries(
  ubuntu_mock_platform_test

  ubuntu_mock_platform
)

target_link_libraries(
  ubuntu_mock_platform_c_api_test

  ubuntu_mock_platform
)