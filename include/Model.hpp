// =============================================================
//  Model.hpp - Modelo 3D leido de un archivo Wavefront OBJ + MTL
//
//  Lee los modelos descargados de NASA 3D Resources (convertidos de
//  GLB a OBJ con tools/glb_a_obj.py). El modelo se centra y se escala
//  para que quepa en una esfera de radio 1; asi en la escena se usa
//  glScalef con el tamano deseado, igual que con una primitiva.
// =============================================================
#pragma once

#include "GLHeaders.hpp"

class Model {
public:
    Model();
    bool load(const std::string& ruta);
    void draw(bool conTexturas) const;
    bool isLoaded() const { return cargado; }

private:
    GLuint listaConTextura;   // display list con las texturas del MTL
    GLuint listaSinTextura;   // la misma geometria solo con colores Kd
    int    triangulos;
    bool   cargado;
};
