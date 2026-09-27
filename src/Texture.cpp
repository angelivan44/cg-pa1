// =============================================================
//  Texture.cpp - Carga de imagenes y manejo de texturas
// =============================================================
#include "Texture.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint Texture::load(const std::string& ruta, bool repetir) {
    int ancho, alto, canales;
    // Las imagenes guardan la primera fila arriba y OpenGL espera la
    // primera fila abajo (t = 0), por eso se voltean al cargar.
    stbi_set_flip_vertically_on_load(1);
    unsigned char* pixeles = stbi_load(ruta.c_str(), &ancho, &alto, &canales, 0);
    if (!pixeles) {
        printf("  [aviso] no se pudo cargar la textura %s\n", ruta.c_str());
        return 0;
    }
    GLenum formato = (canales == 4) ? GL_RGBA : (canales == 3) ? GL_RGB : GL_LUMINANCE;

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLint envoltura = repetir ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, envoltura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, envoltura);
    // Filtro trilineal: evita el parpadeo cuando el planeta se ve lejos
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, formato, ancho, alto, formato, GL_UNSIGNED_BYTE, pixeles);

    stbi_image_free(pixeles);
    // Solo el nombre del archivo, la ruta completa es muy larga
    size_t barra = ruta.find_last_of("/\\");
    printf("  textura %-32s %4dx%-4d (%d canales)\n",
           ruta.substr(barra == std::string::npos ? 0 : barra + 1).c_str(), ancho, alto, canales);
    return id;
}

GLuint Texture::createGlow(int tamano) {
    std::vector<unsigned char> datos(tamano * tamano * 4);
    for (int y = 0; y < tamano; y++) {
        for (int x = 0; x < tamano; x++) {
            float dx = (x + 0.5f) / tamano * 2.0f - 1.0f;
            float dy = (y + 0.5f) / tamano * 2.0f - 1.0f;
            float d = sqrtf(dx * dx + dy * dy);
            float a = d >= 1.0f ? 0.0f : powf(1.0f - d, 2.2f);   // cae suave hacia el borde
            unsigned char* p = &datos[(y * tamano + x) * 4];
            p[0] = 255; p[1] = 200; p[2] = 110;                 // naranja calido
            p[3] = (unsigned char)(a * 255.0f);
        }
    }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tamano, tamano, 0, GL_RGBA, GL_UNSIGNED_BYTE, &datos[0]);
    return id;
}

void Texture::bind(GLuint id, bool activadas) {
    if (activadas && id != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, id);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
}
