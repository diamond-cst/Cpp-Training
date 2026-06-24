cmake_minimum_required(VERSION 3.10)
project(MagicTrick VERSION 1.0 LANGUAGES CXX)

# 设置C++标准 (Set C++ standard)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含目录 (Include directories)
include_directories(${PROJECT_SOURCE_DIR}/include)

# 源文件 (Source files)
set(SOURCES
    src/Card.cpp
    src/ThreePileCardTrick.cpp
    src/TwentyOneCardTrick.cpp
    src/TwentySevenCardTrick.cpp
    src/ConfigurableCardTrick.cpp
    src/Utils.cpp
    src/PileChoice.cpp
    src/ReplayManager.cpp
    src/NetworkGame.cpp
    src/Leaderboard.cpp
)

# 控制台版本已合并到增强版 (Console version merged into enhanced version)
option(BUILD_CONSOLE "Build console version" ON)

if(BUILD_CONSOLE)
    add_executable(magic_trick_console
        ${SOURCES}
        src/main_enhanced.cpp
    )

    # 设置输出目录 (Set output directory)
    set_target_properties(magic_trick_console PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endif()

# GUI版本 (GUI version)
option(BUILD_GUI "Build GUI version" OFF)

if(BUILD_GUI)
    set(GUI_SOURCES
        gui/main.cpp
        gui/GuiUtils.cpp
        gui/CardWidgets.cpp
        gui/MagicTrickWindow.cpp
        gui/LocalGameController.cpp
        gui/InfoPages.cpp
        gui/NetworkGamePage.cpp
        gui/resources.qrc
    )

    set(CMAKE_AUTORCC ON)

    find_package(Qt6 COMPONENTS Widgets Network QUIET)
    if(Qt6_FOUND)
        add_executable(magic_trick_gui ${GUI_SOURCES} ${SOURCES})
        target_link_libraries(magic_trick_gui Qt6::Widgets Qt6::Network)
        if(APPLE)
            target_link_libraries(magic_trick_gui "-framework AudioToolbox" "-framework CoreFoundation")
        endif()
    else()
        find_package(Qt5 COMPONENTS Widgets Network QUIET)
        if(Qt5_FOUND)
            add_executable(magic_trick_gui ${GUI_SOURCES} ${SOURCES})
            target_link_libraries(magic_trick_gui Qt5::Widgets Qt5::Network)
            if(APPLE)
                target_link_libraries(magic_trick_gui "-framework AudioToolbox" "-framework CoreFoundation")
            endif()
        else()
            message(FATAL_ERROR "Qt Widgets not found. Install Qt, then rebuild with -DBUILD_GUI=ON.")
        endif()
    endif()
endif()

# 编译选项 (Compile options)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 创建saves目录 (Create saves directory)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/saves)

message(STATUS "Build configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Console: ${BUILD_CONSOLE}")
message(STATUS "  Build GUI: ${BUILD_GUI}")
