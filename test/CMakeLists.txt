set(TESTS
    test_linklist.cpp
)

add_executable(wayward-tests ${TESTS})
target_include_directories(wayward-tests PRIVATE ${GOOGLETEST_INCLUDE_DIR})
link_directories(wayward-tests ${GOOGLETEST_LIBRARY_PATH})
target_link_libraries(wayward-tests gtest_main gtest)
target_link_libraries(wayward-tests wayward)

add_test(NAME wayward-tests COMMAND ${CMAKE_BINARY_DIR}/test/wayward-tests)

add_executable(wayward-test-server test.cpp)
target_link_libraries(wayward-test-server wayward)

if (WIN32)
    add_custom_command(TARGET wayward-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:wayward> $<TARGET_FILE_DIR:wayward-tests>)
    add_custom_command(TARGET wayward-test-server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:wayward> $<TARGET_FILE_DIR:wayward-test-server>)
endif(WIN32)