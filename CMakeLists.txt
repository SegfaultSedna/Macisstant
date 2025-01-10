cmake_minimum_required(VERSION 3.16)

project(MacisstantQML_cmake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets)


qt_standard_project_setup(REQUIRES 6.5)


set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/custom/resources.rc")
qt_add_executable(appMacisstantQML_cmake WIN32
    main.cpp
    AppController.cpp
    ${app_icon_resource_windows}
)

qt_add_qml_module(appMacisstantQML_cmake
    URI MacisstantQML_cmake
    VERSION 1.0
    QML_FILES
        Main.qml
        custom/components/IconButton.qml
        custom/components/KeyMacroInputField.qml
        custom/components/KeyTextInput.qml
        custom/components/MacroItem.qml
        custom/components/MainButton.qml
        custom/components/NumberTextInput.qml
        custom/components/KbMacrosWindow.qml
        custom/components/SimpleTextInput.qml
        custom/components/PopupWindow.qml
    RESOURCES
        custom/images/arrow-down.svg
        custom/images/bg.jpg
        custom/images/close-line.svg
        custom/images/edit-fill.svg
        custom/images/home.svg
        custom/images/save.svg
        custom/images/settings.svg
        custom/images/square-check-big.svg
        custom/images/appicon.ico
    SOURCES
        AppController.h
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appMacisstantQML_cmake PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appMacisstantQML_cmake
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appMacisstantQML_cmake
    PRIVATE Qt6::Quick Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS appMacisstantQML_cmake
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
