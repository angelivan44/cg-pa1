# =============================================================
#  Sistema Solar - PA3 Computacion Grafica
#
#    cmake -S . -B build          configura (una sola vez)
#    cmake --build build          compila
#    ./build/sistema_solar        ejecuta
#    cmake --build build --target modelos    reconvierte los .glb de NASA a .obj
#
#  Windows: usar la terminal "MSYS2 MINGW64" e instalar una vez
#    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
#              mingw-w64-x86_64-ninja mingw-w64-x86_64-freeglut
#  y compilar con:
#    cmake -S . -B build -G Ninja
#    cmake --build build
#    ./build/sistema_solar.exe
#  Linux: sudo apt install build-essential cmake freeglut3-dev
# =============================================================
cmake_minimum_required(VERSION 3.10)
project(SistemaSolar CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# En Windows (MSYS2 MinGW64) freeglut se enlaza estatico a mano, igual
# que en el PA1; en macOS y Linux lo encuentra CMake.
if(NOT WIN32)
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
endif()

add_executable(sistema_solar
    src/main.cpp
    src/Application.cpp
    src/Scene.cpp
    src/Camera.cpp
    src/Lighting.cpp
    src/Material.cpp
    src/Texture.cpp
    src/Model.cpp
)

target_include_directories(sistema_solar PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Ruta absoluta: el programa encuentra assets/ aunque se ejecute desde build/
target_compile_definitions(sistema_solar PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/"
)

if(WIN32)
    # Windows con MSYS2 MinGW64:
    #   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-freeglut
    # Se enlaza todo estatico (-static) para que el .exe funcione sin DLLs.
    # freeglut necesita ademas estas librerias del sistema de Windows.
    target_compile_definitions(sistema_solar PRIVATE FREEGLUT_STATIC)
    target_compile_options(sistema_solar PRIVATE -Wall)
    target_link_libraries(sistema_solar PRIVATE
        -static freeglut opengl32 glu32 winmm gdi32 user32 ole32 advapi32)
elseif(APPLE)
    # GLUT y OpenGL estan marcados como obsoletos en macOS, pero funcionan
    target_compile_options(sistema_solar PRIVATE -Wall -Wno-deprecated-declarations)
    target_link_libraries(sistema_solar PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
else()
    # Linux: sudo apt install freeglut3-dev
    target_compile_options(sistema_solar PRIVATE -Wall)
    target_link_libraries(sistema_solar PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)
endif()

# --- Conversion de los modelos de NASA (GLB -> OBJ) -----------------
set(GLB ${CMAKE_SOURCE_DIR}/assets/models/glb)
set(MODELOS ${CMAKE_SOURCE_DIR}/assets/models)
if(WIN32)
    set(PY ${CMAKE_SOURCE_DIR}/tools/.venv/Scripts/python.exe)
else()
    set(PY ${CMAKE_SOURCE_DIR}/tools/.venv/bin/python)
endif()
set(CONVERSOR ${CMAKE_SOURCE_DIR}/tools/glb_a_obj.py)
add_custom_target(modelos
    COMMAND ${PY} ${CONVERSOR} "${GLB}/International Space Station (ISS) (A).glb" ${MODELOS}/iss.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Hubble Space Telescope (A).glb" ${MODELOS}/hubble.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Cassini-Huygens (A).glb" ${MODELOS}/cassini.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Voyager Probe (A).glb" ${MODELOS}/voyager.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/1999 RQ36 asteroid.glb" ${MODELOS}/asteroide_bennu.obj
    VERBATIM
)
