cmake_minimum_required(VERSION 3.28)

project(ddos_simulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

find_package(CURL REQUIRED)

set(SOURCES
        src/main.cpp
        src/simulator.cpp
        src/attacker.cpp
        src/proxy.cpp

)

set(HEADERS
        src/simulator.h
        src/attacker.h
        src/proxy.h
        src/curl_utils.cpp
        src/curl_utils.h
        src/webscanner.cpp
        src/webscanner.h
        src/webscanner_tab.cpp
        src/webscanner_tab.h
        src/include/cptl.h
        src/style.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        ${CURL_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src/include)

set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)