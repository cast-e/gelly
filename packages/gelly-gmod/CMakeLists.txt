project(gelly-gmod CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(vendor/gmod-module-base)
add_subdirectory(vendor/minhook)

add_subdirectory(../gelly "${CMAKE_CURRENT_BINARY_DIR}/gelly")

add_library(
    gelly-gmod SHARED
    src/main.cpp
    src/Gelly.cpp
    src/Gelly.h
    src/LoggingMacros.h
    src/source/Interface.h
    src/source/IBaseClientDLL.h
    src/source/MathTypes.h
    src/source/CViewSetup.h
    src/source/IBaseClientDLL.cpp
        src/source/IVRenderView.h
        src/source/IVRenderView.cpp
        src/source/D3DDeviceWrapper.h
        src/source/D3DDeviceWrapper.cpp
        src/hooking/Library.h
        src/hooking/Library.cpp
)

target_link_libraries(gelly-gmod
    PUBLIC
        ${GELLY_LIBS}
        gmod-module-base
        minhook
)

target_include_directories(gelly-gmod
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/gmod-module-base/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/minhook/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GELLY_INCLUDE_DIRS}
)

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
    set_target_properties(gelly-gmod PROPERTIES PREFIX "gmcl_")
    set_target_properties(gelly-gmod PROPERTIES SUFFIX "_win32.dll")
else()
    set_target_properties(gelly-gmod PROPERTIES PREFIX "gmcl_")
    set_target_properties(gelly-gmod PROPERTIES SUFFIX "_win64.dll")
endif()