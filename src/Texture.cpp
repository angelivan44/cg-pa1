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
    // Filtro trilineal: evita el parpadeo cuando el objeto se ve lejos
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
            p[0] = p[1] = p[2] = 255;                           // el color lo pone glColor
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

// ------------------------------------------------------------------
//  Texturas procedurales
// ------------------------------------------------------------------
// Numero pseudoaleatorio 0..1 a partir de dos enteros (siempre el
// mismo para la misma celda, asi la textura sale igual cada vez).
static float ruido(int x, int y) {
    unsigned int n = (unsigned int)x * 374761393u + (unsigned int)y * 668265263u;
    n = (n ^ (n >> 13)) * 1274126177u;
    return ((n ^ (n >> 16)) & 0xffff) / 65535.0f;
}

static unsigned char aByte(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return (unsigned char)(v * 255.0f);
}

GLuint Texture::createProcedural(int tipo, int tamano) {
    std::vector<unsigned char> datos(tamano * tamano * 3);
    for (int y = 0; y < tamano; y++) {
        for (int x = 0; x < tamano; x++) {
            float u = (x + 0.5f) / tamano, v = (y + 0.5f) / tamano;
            float r, g, b;
            if (tipo == 0) {
                // Tablero de 8 x 8 casillas
                bool claro = (((int)(u * 8.0f) + (int)(v * 8.0f)) % 2) == 0;
                r = g = b = claro ? 0.85f : 0.30f;
            } else if (tipo == 1) {
                // Ladrillos: 4 hiladas, cada una desplazada medio ladrillo
                int   fila = (int)(v * 8.0f);
                float fu = u * 4.0f + (fila % 2) * 0.5f;
                float lu = fu - floorf(fu), lv = v * 8.0f - fila;
                bool junta = lu < 0.04f || lv < 0.08f;
                float tono = 0.85f + 0.3f * ruido((int)floorf(fu) + 7 * (fila % 2), fila);
                float grano = 0.92f + 0.16f * ruido(x, y);
                if (junta) { r = g = b = 0.72f * grano; }
                else       { r = 0.62f * tono * grano; g = 0.24f * tono * grano; b = 0.18f * tono * grano; }
            } else if (tipo == 2) {
                // Madera: vetas onduladas a lo largo de U
                float onda = sinf(v * 40.0f + sinf(u * 6.0f) * 3.0f + ruido(x / 6, y / 2) * 1.5f);
                float veta = 0.75f + 0.25f * onda;
                r = 0.62f * veta; g = 0.42f * veta; b = 0.22f * veta;
            } else {
                // Metal cepillado: rayas finas horizontales
                float raya = 0.62f + 0.22f * ruido(x / 24, y) + 0.08f * ruido(x, y);
                r = raya; g = raya; b = raya * 1.04f;
            }
            unsigned char* p = &datos[(y * tamano + x) * 3];
            p[0] = aByte(r); p[1] = aByte(g); p[2] = aByte(b);
        }
    }
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, tamano, tamano, GL_RGB, GL_UNSIGNED_BYTE, &datos[0]);
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
