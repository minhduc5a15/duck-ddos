cmake_minimum_required(VERSION 3.28)

project(duck-nettool)

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

if (WIN32)
    find_program(QT_WINDEPLOY windeployqt HINTS "${_Qt5_DIR}/../../../bin")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${QT_WINDEPLOY} --no-translations --no-system-d3d-compiler "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Copying Qt dependencies to output directory"
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/curl/bin/libcurl-x64.dll"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying libcurl DLL to output directory"
    )
endif ()