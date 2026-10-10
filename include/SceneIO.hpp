// =============================================================
//  SceneIO.hpp - Guardar y cargar escenas
//
//  La escena se guarda en un archivo de texto (.escena), una
//  propiedad por linea:
//
//      objeto
//        id 3
//        nombre Cubo_01
//        tipo cubo
//        padre 0
//        pos 2 0.5 0
//        ...
//      fin
//
//  Se guarda todo lo necesario para reconstruirla: tipo, jerarquia,
//  transformaciones, material, textura, modelo, luces, emisores,
//  fisica y animaciones, ademas del entorno y la camara. Al ser texto
//  se puede leer, comparar y versionar en el repositorio.
// =============================================================
#pragma once

#include "Camera.hpp"
#include "Scene.hpp"

#include <string>

namespace SceneIO {
    bool save(const Scene& escena, const Camera& camara, const std::string& ruta);
    // Reemplaza el contenido de la escena. Devuelve false si no se pudo abrir.
    bool load(Scene& escena, Camera& camara, const std::string& ruta);
}
