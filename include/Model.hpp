// =============================================================
//  Model.hpp - Modelo 3D leido de un archivo Wavefront OBJ + MTL
//
//  Lee cualquier modelo OBJ (por ejemplo los de NASA 3D Resources
//  convertidos de GLB con tools/glb_a_obj.py). El modelo se centra y
//  se escala para que quepa en una esfera de radio 1; asi en el editor
//  se comporta como una primitiva mas: se mueve, gira y escala igual.
// =============================================================
#pragma once

#include "GLHeaders.hpp"
#include "Math.hpp"

class Model {
public:
    enum DrawMode {
        CON_TEXTURAS = 0,   // materiales y texturas del archivo MTL
        SIN_TEXTURAS,       // materiales del MTL, solo colores Kd
        GEOMETRIA,          // solo la malla: el material lo pone el objeto del editor
        NUM_MODOS
    };

    Model();
    bool load(const std::string& ruta);
    void draw(DrawMode modo) const;
    bool isLoaded() const { return cargado; }
    int  triangleCount() const { return triangulos; }

    Vec3 minimo, maximo;      // caja envolvente despues de normalizar

private:
    GLuint listas;            // primera de las NUM_MODOS display lists
    int    triangulos;
    bool   cargado;
};
