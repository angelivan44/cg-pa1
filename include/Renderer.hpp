// =============================================================
//  Renderer.hpp - Dibujo de la escena en el Viewport
//
//  Orden de dibujo de un cuadro:
//   1. cuadricula del suelo (lineas, sin iluminacion)
//   2. objetos opacos, en cualquier orden: el depth buffer resuelve
//      cual queda delante
//   3. objetos transparentes, de atras hacia adelante y sin escribir
//      profundidad, para que se mezclen con lo ya dibujado
//   4. ayudas del editor: iconos de luces, vacios y emisores, y la
//      caja del objeto seleccionado
// =============================================================
#pragma once

#include "RenderOptions.hpp"
#include "Resources.hpp"
#include "Scene.hpp"

class Renderer {
public:
    void init() const;
    void drawGrid() const;
    void drawObjects(const Scene& escena, Resources& recursos,
                     const RenderOptions& opciones, const Vec3& ojo) const;
    void drawHelpers(const Scene& escena, const RenderOptions& opciones) const;

private:
    void drawObject(const SceneObject& o, Resources& recursos, const RenderOptions& opciones) const;
    void drawIcon(const SceneObject& o) const;
};
