// =============================================================
//  Resources.hpp - Biblioteca de recursos del proyecto
//
//  Lee las carpetas del proyecto y ofrece al editor lo que encuentra:
//    assets/textures/   imagenes JPG / PNG / BMP / TGA
//    assets/models/     modelos OBJ
//    escenas/           escenas guardadas (.escena)
//
//  Las texturas y los modelos se piden por NOMBRE de archivo; asi se
//  puede cambiar la textura de un objeto desde el editor sin tocar el
//  codigo. Cada recurso se carga la primera vez que se usa y queda en
//  memoria para los demas objetos que lo compartan.
// =============================================================
#pragma once

#include "GLHeaders.hpp"
#include "Model.hpp"

#include <map>
#include <string>
#include <vector>

class Resources {
public:
    struct TextureEntry {
        std::string nombre;     // nombre de archivo o "(procedural)"
        GLuint      id;
        bool        intentada;  // ya se intento cargar (aunque fallara)
    };

    std::vector<TextureEntry> texturas;
    std::vector<std::string>  modelos;     // archivos .obj
    std::vector<std::string>  escenas;     // nombres sin la extension
    GLuint texParticula;                   // degradado radial de las particulas

    Resources();
    ~Resources();

    void init();        // con el contexto de OpenGL creado
    void scan();        // vuelve a leer las carpetas

    GLuint texture(const std::string& nombre);   // 0 si no existe
    Model* model(const std::string& nombre);     // 0 si no se pudo cargar

private:
    std::map<std::string, Model*> cacheModelos;

    Resources(const Resources&);
    Resources& operator=(const Resources&);
};
