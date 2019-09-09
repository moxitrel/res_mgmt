cmake_minimum_required(VERSION 3.13)

project(res_mgmt)

# ignore test when referenced as package
if (PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME)
    include(CTest)
    include(FindGTest)
    include(GoogleTest)

    if (NOT GTEST_FOUND)
        message(FATAL_ERROR "googletest doesn't exist!")
    endif ()

    add_executable(res_mgmt_test)
    target_sources(res_mgmt_test
            PRIVATE res_mgmt_test.cpp)
    target_compile_features(res_mgmt_test
            PRIVATE cxx_std_11)
    target_link_libraries(res_mgmt_test
            PRIVATE GTest::Main)

    gtest_discover_tests(res_mgmt_test)
endif ()
