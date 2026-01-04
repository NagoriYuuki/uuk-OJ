cmake_minimum_required(VERSION 3.16)
project(uukOJ)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 指定源码（显式列出，避免把测试/临时 cpp 误编进主程序）
add_executable(uukOJ
    src/main.cpp
)

# CRUD 冒烟测试（最简单验证 Mapper 是否能正常工作）
add_executable(crud_smoke src/crud_smoke.cpp)
add_executable(crudtest src/crudtest.cpp)


# MariaDB Connector/C++ 库路径
set(MARIADB_CPP_INCLUDE_DIR "/usr/local/include/mariadb")
set(MARIADB_CPP_LIBRARY_DIR "/usr/local/lib/mariadb")

# 包含头文件
set(UUKOJ_INCLUDE_DIRS
    ${MARIADB_CPP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/mapper
)

target_include_directories(uukOJ PRIVATE ${UUKOJ_INCLUDE_DIRS})
target_include_directories(crud_smoke PRIVATE ${UUKOJ_INCLUDE_DIRS})
target_include_directories(crudtest PRIVATE ${UUKOJ_INCLUDE_DIRS})


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
