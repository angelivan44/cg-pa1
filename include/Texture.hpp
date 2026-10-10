// =============================================================
//  Texture.hpp - Carga de imagenes y manejo de texturas
// =============================================================
#pragma once

#include "GLHeaders.hpp"

class Texture {
public:
    // Carga una imagen JPG/PNG (con stb_image) y crea una textura con
    // mipmaps. "repetir" elige GL_REPEAT o GL_CLAMP_TO_EDGE. Devuelve 0 si el archivo no se pudo leer.
    static GLuint load(const std::string& ruta, bool repetir);

    // Degradado radial blanco generado por codigo (centro opaco, borde
    // transparente); es la imagen de cada particula.
    static GLuint createGlow(int tamano);

    // Texturas generadas por codigo, disponibles aunque la carpeta de
    // recursos este vacia: 0 tablero, 1 ladrillos, 2 madera, 3 metal.
    static GLuint createProcedural(int tipo, int tamano);

    // Activa la textura si "activadas" es true y el id es valido;
    // en otro caso deshabilita GL_TEXTURE_2D.
    static void bind(GLuint id, bool activadas);
};
