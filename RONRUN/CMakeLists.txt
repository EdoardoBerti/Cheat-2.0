cmake_minimum_required(VERSION 3.16)
project(RONRUN LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
#  Usermode Cheat (overlay + features)
# ============================================================================

# ImGui source files — clone into vendor/imgui/
# git clone https://github.com/ocornut/imgui.git vendor/imgui
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

# Check if ImGui exists
if(NOT EXISTS ${IMGUI_DIR}/imgui.h)
    message(FATAL_ERROR
        "ImGui not found at ${IMGUI_DIR}\n"
        "Run: git clone https://github.com/ocornut/imgui.git vendor/imgui"
    )
endif()

# ImGui source files
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# Main cheat executable
add_executable(ronrun WIN32
    cheat/main.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(ronrun PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/cheat
    ${CMAKE_SOURCE_DIR}/driver
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(ronrun PRIVATE
    d3d11
    dxgi
    dwmapi
    psapi
)

# Disable console subsystem warning (we use WinMain but also AllocConsole)
set_target_properties(ronrun PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    OUTPUT_NAME "ronrun"
)

# ============================================================================
#  Notes:
#
#  The kernel driver (driver/driver.c) must be built separately with WDK.
#  See README.md for instructions.
#
#  To build the usermode cheat:
#    1. Clone ImGui:  git clone https://github.com/ocornut/imgui.git vendor/imgui
#    2. Create build dir:  mkdir build && cd build
#    3. Generate:  cmake .. -G "Visual Studio 17 2022" -A x64
#    4. Build:  cmake --build . --config Release
#
#  Output: build/Release/ronrun.exe
# ============================================================================
