cmake_minimum_required(VERSION 3.16)
project(poco_template_server VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PostgreSQL REQUIRED)

if(TARGET PostgreSQL::PostgreSQL AND NOT TARGET PostgreSQL::client)
    add_library(PostgreSQL::client INTERFACE IMPORTED)
    set_property(TARGET PostgreSQL::client PROPERTY
        INTERFACE_LINK_LIBRARIES PostgreSQL::PostgreSQL)
endif()

find_package(Poco REQUIRED COMPONENTS
    Net
    Foundation
    Util
    JWT
    Data
    DataPostgreSQL
)

add_executable(poco_template_server
    src/main.cpp
    src/handler/storage/db_session.cpp
    src/handler/storage/postgres_storage.cpp
)

target_include_directories(poco_template_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(poco_template_server
    PRIVATE
        Poco::Net
        Poco::Foundation
        Poco::Util
        Poco::JWT
        Poco::Data
        Poco::DataPostgreSQL
        PostgreSQL::PostgreSQL
)

install(TARGETS poco_template_server DESTINATION bin)