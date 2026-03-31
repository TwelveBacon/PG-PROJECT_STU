#Tu nastavit verziu Qt6
set(Qt6_Version 6.10.2)

cmake_minimum_required(VERSION 3.16)

project(IV_playground VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  add_compile_options(/MP /openmp)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt6_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6")
set(Qt6WidgetsTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6WidgetsTools")
set(Qt6CoreTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6CoreTools")
set(Qt6GuiTools_DIR "C:/Qt/${Qt6_Version}/msvc2022_64/lib/cmake/Qt6GuiTools")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

file(GLOB UI_FILES *.ui)
file(GLOB H_FILES *.h)
file(GLOB CPP_FILES *.cpp)
file(GLOB QRC_FILES *.qrc)

set(SOURCE_LIST ${CPP_FILES} ${UI_FILES} ${H_FILES} ${QRC_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_LIST})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND "C:/Qt/${Qt6_Version}/msvc2022_64/bin/windeployqt.exe" "$<TARGET_FILE:${PROJECT_NAME}>"
                   COMMENT "Win deploy Qt6")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
