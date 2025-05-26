add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE timeduration::timeduration)

add_executable(advanced_usage advanced_usage.cpp)
target_link_libraries(advanced_usage PRIVATE timeduration::timeduration)
