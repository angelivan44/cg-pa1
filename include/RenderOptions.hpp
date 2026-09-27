// =============================================================
//  RenderOptions.hpp - Parametros de renderizado para experimentar
//
//  Cada campo se cambia con una tecla (ver Application.cpp) y permite
//  comparar el resultado visual con el parametro activado o no.
// =============================================================
#pragma once

struct RenderOptions {
    bool texturas;           // T: texturas activadas
    bool gouraud;            // G: sombreado suave (Gouraud) o plano (Flat)
    int  detalleMalla;       // V: 0 bajo, 1 medio, 2 alto (divisiones de las esferas)
    bool materialBrillante;  // M: planetas mate o con brillo especular
    bool depthBuffer;        // Z: prueba de profundidad activada
    bool transparencias;     // B: mezcla alfa (anillos) y halo del Sol
    bool orbitas;            // O: lineas de las orbitas

    RenderOptions() { reset(); }

    void reset() {
        texturas          = true;
        gouraud           = true;
        detalleMalla      = 2;
        materialBrillante = false;
        depthBuffer       = true;
        transparencias    = true;
        orbitas           = true;
    }

    int sphereDivisions() const {
        static const int DIVISIONES[] = { 10, 24, 64 };
        return DIVISIONES[detalleMalla];
    }
};
