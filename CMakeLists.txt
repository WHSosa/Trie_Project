cmake_minimum_required(VERSION 3.10)
project(Trie_Project)

set(CMAKE_CXX_STANDARD 17)

# PostgreSQL + libpqxx
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${PQXX_INCLUDE_DIRS})
link_directories(${PQXX_LIBRARY_DIRS})

add_executable(Trie_Project
    cpp/Trie_Project.cpp
)

target_link_libraries(Trie_Project
    ${PQXX_LIBRARIES}
)
