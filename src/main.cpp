// =============================================================
//  Sistema Solar - Computacion Grafica (PA3)
//  Renderizado de una escena 3D con iluminacion, materiales,
//  sombreado, texturas y visibilidad (OpenGL clasico + GLUT).
//
//  Compilar:  cmake -S . -B build && cmake --build build
//  Ejecutar:  ./build/sistema_solar
// =============================================================
#include "Application.hpp"

int main(int argc, char** argv) {
    Application app;
    return app.run(argc, argv);
}
