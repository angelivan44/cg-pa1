// =============================================================
//  Editor de Escenas 3D - Computacion Grafica (PA4)
//  Aplicacion para crear, modificar, visualizar y animar escenas
//  3D en tiempo de ejecucion (OpenGL clasico + GLUT + Dear ImGui).
//
//  Compilar:  cmake -S . -B build && cmake --build build
//  Ejecutar:  ./build/editor3d
// =============================================================
#include "Application.hpp"

int main(int argc, char** argv) {
    Application app;
    return app.run(argc, argv);
}
