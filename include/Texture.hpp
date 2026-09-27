// =============================================================
//  Texture.hpp - Carga de imagenes y manejo de texturas
// =============================================================
#pragma once

#include "GLHeaders.hpp"

class Texture {
public:
    // Carga una imagen JPG/PNG (con stb_image) y crea una textura con
    // mipmaps. "repetir" elige GL_REPEAT (planetas) o GL_CLAMP_TO_EDGE
    // (anillos). Devuelve 0 si el archivo no se pudo leer.
    static GLuint load(const std::string& ruta, bool repetir);

    // Degradado radial generado por codigo (centro opaco, borde
    // transparente); se usa para el halo del Sol.
    static GLuint createGlow(int tamano);

    // Activa la textura si "activadas" es true y el id es valido;
    // en otro caso deshabilita GL_TEXTURE_2D.
    static void bind(GLuint id, bool activadas);
};
