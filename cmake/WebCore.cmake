# Macro for web builds

macro(hydra_web_library)
    if (EMSCRIPTEN)
        set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS TRUE)
        set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-s SIDE_MODULE=1")
        set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "-s SIDE_MODULE=1")
        set(CMAKE_SHARED_LIBRARY_SUFFIX_CXX ".wasm")
        set(CMAKE_STRIP FALSE)
    endif()
endmacro()