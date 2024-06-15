cmake_minimum_required(VERSION 3.5)

project(wx_markdown LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

#find_package(Qt5 COMPONENTS Widgets LinguistTools X11Extras REQUIRED)
#find_package(QHotkey REQUIRED)

#WebEngineWidgets can be installed by qtwebengine5-dev
find_package(Qt5 REQUIRED COMPONENTS Widgets LinguistTools X11Extras WebEngineWidgets )
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake/QHotkey" ${CMAKE_PREFIX_PATH})
find_package(QHotkey REQUIRED)

set(TS_FILES wx_markdown_en_US.ts)

if(ANDROID)
  add_library(wx_markdown SHARED
          main.cpp
          main.ui
          ${TS_FILES}
  )
else()
  add_executable(wx_markdown
          main.cpp
          main.ui
          ${TS_FILES}
  )
endif()

target_include_directories(wx_markdown PRIVATE /home/chaowen/projects/cmark/build/include)
target_link_libraries(wx_markdown PRIVATE Qt5::Widgets Qt5::X11Extras Qt5::WebEngineWidgets qhotkey /home/chaowen/projects/cmark/build/lib/libcmark.a)

qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
