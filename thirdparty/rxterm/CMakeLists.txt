add_library(rxterm
    components/component.hpp
    components/flowlayout.hpp
    components/maxwidth.hpp
    components/progress.hpp
    components/stacklayout.hpp
    components/text.hpp
    image.hpp
    nothing.cpp
    pixel.hpp
    reflow.hpp
    style.hpp
    terminal.hpp
    utils.hpp)


target_include_directories(rxterm
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)