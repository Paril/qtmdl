cmake_minimum_required(VERSION 3.16)
# Qt6 requires 3.16, see: https://doc.qt.io/qt-6/cmake-get-started.html

project(qtmdl VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools OpenGL OpenGLWidgets)

set(TS_FILES qtmdl_en_US.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        qmdlrenderer.cpp
        qmdlrenderer.h
        qlessannoyingmenu.h
        camera.h
        camera.cpp
        glad.c
        glad.h
        khrplatform.h
        quvpainter.cpp
        quvpainter.h
        qtutils.h
        settings.cpp
        settings.h
        editortypes.h
        modelloader.cpp
        modelloader.h
        uveditor.h
        uveditor.cpp
        uveditor.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qtmdl
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qtmdl APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(qtmdl SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qtmdl
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(qtmdl PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::OpenGLWidgets)

file(GLOB SHADER_SOURCE_FILES "${CMAKE_SOURCE_DIR}/res/shaders/*")
file(GLOB RESOURCE_SOURCE_FILES "${CMAKE_SOURCE_DIR}/res/*")
file(GLOB MINE_SOURCE_FILES "${CMAKE_SOURCE_DIR}/mime/packages/*")

add_custom_command(
    TARGET qtmdl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:qtmdl>/res
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RESOURCE_SOURCE_FILES} $<TARGET_FILE_DIR:qtmdl>/res

    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:qtmdl>/res/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_SOURCE_FILES} $<TARGET_FILE_DIR:qtmdl>/res/shaders

    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:qtmdl>/mime/packages
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MINE_SOURCE_FILES} $<TARGET_FILE_DIR:qtmdl>/mime/packages
)

if (WIN32)
    add_custom_command(
        TARGET qtmdl POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Core> $<TARGET_FILE_DIR:qtmdl>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Widgets> $<TARGET_FILE_DIR:qtmdl>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::Gui> $<TARGET_FILE_DIR:qtmdl>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::OpenGLWidgets> $<TARGET_FILE_DIR:qtmdl>
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:qtmdl>/platforms
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:qtmdl>/imageformats
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::QWindowsIntegrationPlugin> $<TARGET_FILE_DIR:qtmdl>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt6::QJpegPlugin> $<TARGET_FILE_DIR:qtmdl>/imageformats
    )
endif ()

target_compile_definitions(qtmdl PUBLIC QT_NO_DEPRECATED)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qtmdl)
endif()
set_target_properties(qtmdl PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qtmdl
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (WIN32)
    install(FILES "$<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>" DESTINATION bin/platforms)
endif ()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qtmdl)
endif()