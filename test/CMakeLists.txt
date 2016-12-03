set(TESTS
    test_linklist.cpp
)

add_executable(wayward-tests ${TESTS})
target_include_directories(wayward-tests PRIVATE ${GOOGLETEST_INCLUDE_DIR})
link_directories(wayward-tests ${GOOGLETEST_LIBRARY_PATH})
target_link_libraries(wayward-tests ${GOOGLETEST_MAIN_LIBRARY} ${GOOGLETEST_LIBRARY})
target_link_libraries(wayward-tests wayward)

add_test(NAME wayward-tests COMMAND ${CMAKE_BINARY_DIR}/test/wayward-tests)

add_executable(wayward-test-server test.cpp)
target_link_libraries(wayward-test-server wayward)
