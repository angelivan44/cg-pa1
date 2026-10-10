// =============================================================
//  Lighting.hpp - Iluminacion de la escena
//
//  Las luces son objetos de la escena (tipo Luz): se crean, mueven,
//  giran y animan como cualquier otro, y pueden colgar de un padre.
//  En cada cuadro este modulo las recorre y configura GL_LIGHT0..7:
//
//    puntual      w = 1, emite en todas direcciones desde su posicion
//    direccional  w = 0, rayos paralelos (como el Sol visto de lejos)
//    foco         puntual + cono (GL_SPOT_CUTOFF / GL_SPOT_EXPONENT)
//
//  Las direccionales y los focos apuntan hacia el -Y local del objeto.
//  Cada luz aporta componente ambiental, difusa y especular; ademas
//  hay una luz ambiental global (glLightModel).
// =============================================================
#pragma once

#include "Scene.hpp"

class Lighting {
public:
    enum { MAX_LUCES = 8 };

    void init() const;      // una vez, al iniciar OpenGL
    // Cada cuadro, DESPUES de cargar la matriz de vista.
    // Devuelve cuantas luces quedaron encendidas.
    int  apply(const Scene& escena) const;
};
