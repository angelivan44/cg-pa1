# =============================================================
#  Sistema Solar - PA3 Computacion Grafica
#
#    cmake -S . -B build          configura (una sola vez)
#    cmake --build build          compila
#    ./build/sistema_solar        ejecuta
#    cmake --build build --target capturas   genera las figuras en capturas/
#    cmake --build build --target modelos    reconvierte los .glb de NASA a .obj
# =============================================================
cmake_minimum_required(VERSION 3.10)
project(SistemaSolar CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

add_executable(sistema_solar
    src/main.cpp
    src/Application.cpp
    src/Scene.cpp
    src/Camera.cpp
    src/Lighting.cpp
    src/Material.cpp
    src/Texture.cpp
    src/Model.cpp
    src/Screenshot.cpp
)

target_include_directories(sistema_solar PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Rutas absolutas: el programa encuentra assets/ y guarda capturas/
# en la carpeta del proyecto aunque se ejecute desde build/.
# CAPTURES_DIR se puede cambiar (el preview usa -DCAPTURES_DIR=preview/capturas).
# CMake quita la barra final de las rutas, por eso se agrega abajo.
set(CAPTURES_DIR "${CMAKE_SOURCE_DIR}/capturas" CACHE PATH "Carpeta donde se guardan las capturas")
target_compile_definitions(sistema_solar PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/"
    CAPTURES_DIR="${CAPTURES_DIR}/"
)

target_link_libraries(sistema_solar PRIVATE OpenGL::GL OpenGL::GLU GLUT::GLUT)

if(APPLE)
    # GLUT y OpenGL estan marcados como obsoletos en macOS, pero funcionan
    target_compile_options(sistema_solar PRIVATE -Wall -Wno-deprecated-declarations)
elseif(WIN32)
    target_compile_definitions(sistema_solar PRIVATE FREEGLUT_STATIC)
else()
    target_compile_options(sistema_solar PRIVATE -Wall)
endif()

# --- Figuras para el informe ---------------------------------------
add_custom_target(capturas
    COMMAND sistema_solar --capturas
    COMMAND sh ${CMAKE_SOURCE_DIR}/tools/bmp_a_png.sh
    DEPENDS sistema_solar
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generando las figuras en capturas/"
)

# --- Conversion de los modelos de NASA (GLB -> OBJ) -----------------
set(GLB ${CMAKE_SOURCE_DIR}/assets/models/glb)
set(MODELOS ${CMAKE_SOURCE_DIR}/assets/models)
set(PY ${CMAKE_SOURCE_DIR}/tools/.venv/bin/python)
set(CONVERSOR ${CMAKE_SOURCE_DIR}/tools/glb_a_obj.py)
add_custom_target(modelos
    COMMAND ${PY} ${CONVERSOR} "${GLB}/International Space Station (ISS) (A).glb" ${MODELOS}/iss.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Hubble Space Telescope (A).glb" ${MODELOS}/hubble.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Cassini-Huygens (A).glb" ${MODELOS}/cassini.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Voyager Probe (A).glb" ${MODELOS}/voyager.obj
    COMMAND ${PY} ${CONVERSOR} "${GLB}/1999 RQ36 asteroid.glb" ${MODELOS}/asteroide_bennu.obj
    VERBATIM
)
