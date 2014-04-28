set(jack_keys_SRCS
    jack-piano.cpp
    mainwindow.cpp
    piano.cpp
    octave.cpp
    key.cpp
    jackoutput.cpp
    )

set(jack_keys_HEADERS
    mainwindow.h
    piano.h
    octave.h
    key.h
    jackoutput.h
    )

set(jack_keys_FORMS
    mainwindow.ui
    piano.ui
    )

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
qt5_wrap_cpp(jack_keys_HEADERS_MOC ${jack_keys_HEADERS})
qt5_wrap_ui(jack_keys_FORMS_HEADERS ${jack_keys_FORMS})

find_package(JACK REQUIRED)

link_directories(
    )

include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${JACK_INCLUDE_DIRS}
    )

add_executable(jack-keys ${jack_keys_SRCS} ${jack_keys_HEADERS_MOC} ${jack_keys_FORMS_HEADERS})

target_link_libraries(jack-keys
        ${JACK_LIBRARIES}
	)

qt5_use_modules(jack-keys Widgets)
