// =============================================================
//  Gizmo.hpp - Manipulador del objeto seleccionado en el Viewport
//
//  Dibuja tres ejes (X rojo, Y verde, Z azul) sobre el objeto
//  seleccionado. Arrastrar un eje con el mouse lo mueve, lo gira o
//  lo escala segun el modo activo.
//
//  Para saber cuanto mover, el eje se proyecta a la pantalla y el
//  movimiento del mouse se proyecta sobre esa direccion (producto
//  punto): asi el objeto sigue al cursor a lo largo del eje.
// =============================================================
#pragma once

#include "Camera.hpp"
#include "Scene.hpp"

class Gizmo {
public:
    enum Mode { MOVER = 0, ROTAR, ESCALAR };

    Mode modo;
    int  ejeActivo;      // eje que se esta arrastrando (-1 = ninguno)

    Gizmo();
    void draw(const SceneObject& o, const Camera& camara) const;
    // Eje bajo el cursor (0 X, 1 Y, 2 Z) o -1
    int  hitTest(const SceneObject& o, const Camera& camara, int mx, int my) const;
    // Aplica un arrastre de (dx, dy) pixeles sobre el eje dado
    void drag(const Scene& escena, SceneObject& o, const Camera& camara, int eje, float dx, float dy) const;

private:
    Vec3  axisDirection(const SceneObject& o, int eje) const;
    float length(const SceneObject& o, const Camera& camara) const;
};
