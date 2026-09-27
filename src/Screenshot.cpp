// =============================================================
//  Screenshot.cpp - Guarda el buffer trasero en un archivo BMP
// =============================================================
#include "Screenshot.hpp"
#include "GLHeaders.hpp"

#include <cstdio>
#include <vector>

static void escribir32(unsigned char* p, int v) {
    p[0] = (unsigned char)(v);
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}

bool Screenshot::saveBMP(const std::string& ruta) {
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int ancho = vp[2], alto = vp[3];
    int filaBytes = (ancho * 3 + 3) & ~3;              // cada fila alineada a 4 bytes
    std::vector<unsigned char> pixeles(filaBytes * alto);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    // BMP guarda las filas de abajo hacia arriba y en orden BGR, igual
    // que devuelve glReadPixels con GL_BGR: no hay que reordenar nada.
    glReadPixels(0, 0, ancho, alto, GL_BGR, GL_UNSIGNED_BYTE, &pixeles[0]);

    unsigned char cab[54] = { 'B', 'M' };
    escribir32(cab + 2, 54 + (int)pixeles.size());   // tamano del archivo
    escribir32(cab + 10, 54);                        // inicio de los pixeles
    escribir32(cab + 14, 40);                        // tamano de la cabecera DIB
    escribir32(cab + 18, ancho);
    escribir32(cab + 22, alto);
    cab[26] = 1;                                     // planos
    cab[28] = 24;                                    // bits por pixel
    escribir32(cab + 34, (int)pixeles.size());

    FILE* f = fopen(ruta.c_str(), "wb");
    if (!f) {
        printf("No se pudo escribir %s\n", ruta.c_str());
        return false;
    }
    fwrite(cab, 1, 54, f);
    fwrite(&pixeles[0], 1, pixeles.size(), f);
    fclose(f);
    printf("Captura guardada: %s\n", ruta.c_str());
    return true;
}
