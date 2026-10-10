# =============================================================
#  Editor de Escenas 3D - PA4 Computacion Grafica
#
#    cmake -S . -B build          configura (una sola vez)
#    cmake --build build          compila
#    ./build/editor3d             ejecuta
#    cmake --build build --target modelos    reconvierte los .glb de NASA a .obj
#
#  Windows: usar la terminal "MSYS2 MINGW64" e instalar una vez
#    pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
#              mingw-w64-x86_64-ninja mingw-w64-x86_64-freeglut
#  y compilar con:
#    cmake -S . -B build -G Ninja
#    cmake --build build
#    ./build/editor3d.exe
#  Linux: sudo apt install build-essential cmake freeglut3-dev
# =============================================================
cmake_minimum_required(VERSION 3.10)
project(Editor3D CXX)

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

# --- Dear ImGui (interfaz grafica) -----------------------------------
# Se compila como biblioteca aparte: asi sus avisos no se mezclan con
# los del editor y solo se recompila cuando cambia.
set(IMGUI ${CMAKE_SOURCE_DIR}/third_party/imgui)
add_library(imgui STATIC
    ${IMGUI}/imgui.cpp
    ${IMGUI}/imgui_draw.cpp
    ${IMGUI}/imgui_tables.cpp
    ${IMGUI}/imgui_widgets.cpp
    ${IMGUI}/imgui_impl_glut.cpp
    ${IMGUI}/imgui_impl_opengl2.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI})

# --- Editor ----------------------------------------------------------
add_executable(editor3d
    src/main.cpp
    src/Application.cpp     # ventana, entrada y bucle principal
    src/Editor.cpp          # estado y operaciones del editor
    src/EditorUI.cpp        # paneles de la interfaz
    src/Scene.cpp           # lista de objetos y jerarquia
    src/SceneObject.cpp     # objeto: transformacion y propiedades
    src/SceneIO.cpp         # guardar y cargar escenas
    src/Examples.cpp        # escenas de ejemplo
    src/Math.cpp            # vectores y matrices
    src/Camera.cpp          # camara y proyecciones
    src/Renderer.cpp        # dibujo de la escena
    src/Primitives.cpp      # geometria de las primitivas
    src/Lighting.cpp        # fuentes de luz
    src/Material.cpp        # materiales
    src/Texture.cpp         # carga de imagenes
    src/Model.cpp           # modelos OBJ
    src/Resources.cpp       # biblioteca de texturas y modelos
    src/Animation.cpp       # animacion por interpolacion
    src/Physics.cpp         # fisica y colisiones
    src/Particles.cpp       # sistema de particulas
    src/Gizmo.cpp           # manipulador en el Viewport
)

target_include_directories(editor3d PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
)

# Rutas absolutas: el programa encuentra assets/ y escenas/ aunque se
# ejecute desde build/
target_compile_definitions(editor3d PRIVATE
    ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets/"
    SCENES_DIR="${CMAKE_SOURCE_DIR}/escenas/"
)

if(WIN32)
    # Windows con MSYS2 MinGW64:
    #   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-freeglut
    # Se enlaza todo estatico (-static) para que el .exe funcione sin DLLs.
    # freeglut necesita ademas estas librerias del sistema de Windows.
    target_compile_definitions(imgui PUBLIC FREEGLUT_STATIC)
    target_compile_options(editor3d PRIVATE -Wall)
    target_link_libraries(editor3d PRIVATE imgui
        -static freeglut opengl32 glu32 winmm gdi32 user32 ole32 advapi32 imm32)
elseif(APPLE)
    # GLUT y OpenGL estan marcados como obsoletos en macOS, pero funcionan
    target_compile_options(imgui PRIVATE -Wno-deprecated-declarations)
    target_compile_options(editor3d PRIVATE -Wall -Wno-deprecated-declarations)
    target_link_libraries(imgui PUBLIC OpenGL::GL GLUT::GLUT)
    target_link_libraries(editor3d PRIVATE imgui OpenGL::GL OpenGL::GLU GLUT::GLUT)
else()
    # Linux: sudo apt install freeglut3-dev
    target_compile_options(editor3d PRIVATE -Wall)
    target_link_libraries(imgui PUBLIC OpenGL::GL GLUT::GLUT)
    target_link_libraries(editor3d PRIVATE imgui OpenGL::GL OpenGL::GLU GLUT::GLUT)
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
    COMMAND ${PY} ${CONVERSOR} "${GLB}/Voyager Probe (A).glb" ${MODELOS}/voyager.obj
    VERBATIM
)
