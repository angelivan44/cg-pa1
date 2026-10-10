// =============================================================
//  RenderOptions.hpp - Opciones de visualizacion del Viewport
//
//  Son opciones de la vista, no de la escena: no se guardan en el
//  archivo. Se cambian desde el menu Ver, la barra del Viewport o
//  el panel Vista.
// =============================================================
#pragma once

struct RenderOptions {
    bool wireframe;      // malla de alambre o solido
    bool gouraud;        // sombreado suave (Gouraud) o plano (Flat)
    bool texturas;       // texturas activadas
    bool iluminacion;    // calculo de luces activado
    bool depthBuffer;    // prueba de profundidad
    bool ayudas;         // iconos de luces, vacios y emisores
    int  detalleMalla;   // 0 bajo, 1 medio, 2 alto (divisiones de las primitivas)

    RenderOptions() { reset(); }

    void reset() {
        wireframe    = false;
        gouraud      = true;
        texturas     = true;
        iluminacion  = true;
        depthBuffer  = true;
        ayudas       = true;
        detalleMalla = 2;
    }

    int divisions() const {
        static const int DIVISIONES[] = { 10, 24, 48 };
        return DIVISIONES[detalleMalla];
    }
};
