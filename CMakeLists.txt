cmake_minimum_required(VERSION 3.16)
project(uukOJ)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(REDIS_PLUS_PLUS_INCLUDE_DIR sw/redis++/redis++.h)
find_library(REDIS_PLUS_PLUS_LIBRARY redis++)
find_library(HIREDIS_LIBRARY hiredis)
find_package(Threads REQUIRED)

# 指定源码（显式列出，避免把测试/临时 cpp 误编进主程序）
add_executable(uukOJ
    src/main.cpp
)

# 评测机
add_executable(uuk_judger src/uuk-judger/src/main.cpp)

# CRUD 冒烟测试（最简单验证 Mapper 是否能正常工作）
add_executable(crud_smoke 
    src/crud_smoke.cpp
    )
add_executable(crudtest 
    src/crudtest.cpp
)

add_executable(api_checker
    src/api_checker.cpp
)


# MariaDB Connector/C++ 库路径
set(MARIADB_CPP_INCLUDE_DIR "/usr/local/include/mariadb")
set(MARIADB_CPP_LIBRARY_DIR "/usr/local/lib/mariadb")

# 包含头文件
set(UUKOJ_INCLUDE_DIRS
    ${MARIADB_CPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/mapper
    ${CMAKE_CURRENT_SOURCE_DIR}/Services
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/uuk-judger/include
)

target_include_directories(uukOJ PRIVATE ${UUKOJ_INCLUDE_DIRS})
target_include_directories(crud_smoke PRIVATE ${UUKOJ_INCLUDE_DIRS})
target_include_directories(crudtest PRIVATE ${UUKOJ_INCLUDE_DIRS})
target_include_directories(api_checker PRIVATE ${UUKOJ_INCLUDE_DIRS})



# 链接库
target_link_directories(uukOJ PRIVATE ${MARIADB_CPP_LIBRARY_DIR})
target_link_libraries(uukOJ PRIVATE mariadbcpp)

target_link_directories(crud_smoke PRIVATE ${MARIADB_CPP_LIBRARY_DIR})
target_link_libraries(crud_smoke PRIVATE mariadbcpp)


target_link_directories(crudtest PRIVATE ${MARIADB_CPP_LIBRARY_DIR})
target_link_libraries(crudtest PRIVATE mariadbcpp)

set_target_properties(uukOJ PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${MARIADB_CPP_LIBRARY_DIR}"
)


# 评测机部分

target_include_directories(uuk_judger PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uuk-judger/src 
    ${REDIS_PLUS_PLUS_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接评测机需要的库: redis++, hiredis, pthread
target_link_libraries(uuk_judger 
    PRIVATE
    ${REDIS_PLUS_PLUS_LIBRARY}
    ${HIREDIS_LIBRARY}
    Threads::Threads
    seccomp
)

# 为了方便，让它也输出到 build 根目录
set_target_properties(uuk_judger PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
