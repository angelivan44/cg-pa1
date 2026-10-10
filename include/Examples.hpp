// =============================================================
//  Examples.hpp - Escenas de ejemplo
//
//  Se construyen con las mismas operaciones que usa el editor (crear
//  objeto, asignar padre, agregar animacion...), por eso quedan
//  totalmente editables y se pueden guardar como cualquier escena.
//  Muestran para que sirve cada parte del editor:
//    Basica           primitivas, materiales, texturas y luces
//    Sistema solar    jerarquia de 4 niveles + animacion (escena del PA3)
//    Brazo robotico   cadena articulada: cinematica directa
//    Fisica           gravedad, rebotes, colisiones y particulas
// =============================================================
#pragma once

#include "Camera.hpp"
#include "Scene.hpp"

namespace Examples {
    int         count();
    const char* name(int i);
    void        build(int i, Scene& escena, Camera& camara);
}
