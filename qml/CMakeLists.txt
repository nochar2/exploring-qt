# copypasted from https://doc.qt.io/qt-6/cmake-build-qml-application.html,
# (minor changes)
cmake_minimum_required(VERSION 3.19)

project(hello VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 6.2 COMPONENTS Quick Gui REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

qt_add_executable(myapp
    main.cpp
)

qt_add_qml_module(myapp
    URI hello
    QML_FILES
        main.qml
)

target_link_libraries(myapp PRIVATE Qt6::Gui Qt6::Quick)
