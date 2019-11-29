cmake_minimum_required(VERSION 3.7)
set(TARGET_NAME libqtwin)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LIB_INCLUDE_DIR}
    )

set(libqtwin_MOC_HEADERS RenderWidget.h MainWindow.h)

qt4_wrap_cpp(libqtwin_MOC_FILES ${libqtwin_MOC_HEADERS})
file_to_cpp(libqtwin_DATA shaders::vertex shaders/vertex.glsl)

file(GLOB SOURCES *.cpp)
add_library(${TARGET_NAME} ${SOURCES} ${libqtwin_DATA} ${libqtwin_MOC_FILES})
