// =============================================================
//  Primitives.hpp - Geometria de las primitivas del editor
//
//  Cada funcion dibuja la primitiva centrada en el origen y dentro
//  de un cubo de lado 1, con normales (para la iluminacion) y
//  coordenadas de textura. "divisiones" controla el detalle de la
//  malla: mas divisiones = curvas mas suaves y mejor iluminacion
//  (OpenGL clasico calcula la luz solo en los vertices).
// =============================================================
#pragma once

namespace Primitives {
    void cube(int divisiones);
    void sphere(int divisiones);
    void cylinder(int divisiones);
    void cone(int divisiones);
    void plane(int divisiones);        // en el plano XZ, normal +Y
    void torus(int divisiones);        // acostado en el plano XZ
    void disc(int divisiones);         // corona circular en el plano XZ

    // Dibuja la primitiva que corresponde a un ObjectType
    void draw(int tipo, int divisiones);

    // Caja de alambre entre dos esquinas (seleccion)
    void wireBox(const float minimo[3], const float maximo[3]);
}
