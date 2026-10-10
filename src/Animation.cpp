// =============================================================
//  Animation.cpp - Animacion por interpolacion en el tiempo
// =============================================================
#include "Animation.hpp"

#include <cmath>

AnimClip::AnimClip()
    : propiedad(ANIM_ROT_Y), inicio(0.0f), fin(360.0f), duracion(3.0f), retardo(0.0f),
      repeticion(ANIM_REPETIR), curva(ANIM_LINEAL), activa(true) {}

// Curvas de interpolacion: reciben el avance u en 0..1 y devuelven
// cuanto se ha recorrido del camino entre inicio y fin.
static float applyCurve(int curva, float u) {
    switch (curva) {
        case ANIM_SUAVE:    return u * u * (3.0f - 2.0f * u);            // arranca y frena suave
        case ANIM_ACELERAR: return u * u;
        case ANIM_FRENAR:   return 1.0f - (1.0f - u) * (1.0f - u);
        case ANIM_REBOTE: {                                              // rebota al llegar
            const float n = 7.5625f, d = 2.75f;
            if (u < 1.0f / d)  return n * u * u;
            if (u < 2.0f / d)  { u -= 1.5f / d;   return n * u * u + 0.75f; }
            if (u < 2.5f / d)  { u -= 2.25f / d;  return n * u * u + 0.9375f; }
            u -= 2.625f / d;
            return n * u * u + 0.984375f;
        }
        default:            return u;                                    // lineal
    }
}

float AnimClip::evaluate(float tiempo) const {
    float t = tiempo - retardo;
    if (t <= 0.0f) return inicio;
    float u = duracion > 1e-5f ? t / duracion : 1.0f;

    switch (repeticion) {
        case ANIM_REPETIR:                  // 0..1, 0..1, 0..1 ...
            u = u - floorf(u);
            break;
        case ANIM_PING_PONG:                // 0..1..0..1 ...
            u = fmodf(u, 2.0f);
            if (u > 1.0f) u = 2.0f - u;
            break;
        default:                            // una vez: se queda en el valor final
            if (u > 1.0f) u = 1.0f;
            break;
    }
    return inicio + (fin - inicio) * applyCurve(curva, u);
}

const char* AnimClip::propertyName(int p) {
    static const char* NOMBRES[ANIM_NUM_PROPIEDADES] = {
        "Posición X", "Posición Y", "Posición Z",
        "Rotación X", "Rotación Y", "Rotación Z",
        "Escala X",   "Escala Y",   "Escala Z",
        "Escala uniforme"
    };
    return (p >= 0 && p < ANIM_NUM_PROPIEDADES) ? NOMBRES[p] : "?";
}

const char* AnimClip::loopName(int r) {
    static const char* NOMBRES[ANIM_NUM_REPETICIONES] = { "Una vez", "Repetir", "Ida y vuelta" };
    return (r >= 0 && r < ANIM_NUM_REPETICIONES) ? NOMBRES[r] : "?";
}

const char* AnimClip::curveName(int c) {
    static const char* NOMBRES[ANIM_NUM_CURVAS] = { "Lineal", "Suave", "Acelerar", "Frenar", "Rebote" };
    return (c >= 0 && c < ANIM_NUM_CURVAS) ? NOMBRES[c] : "?";
}

// ------------------------------------------------------------------
//  Reloj de la escena
// ------------------------------------------------------------------
Animator::Animator() : estado(DETENIDO), tiempo(0.0f), velocidad(1.0f) {}

float Animator::update(float dtReal) {
    if (estado != REPRODUCIENDO) return 0.0f;
    float dt = dtReal * velocidad;
    tiempo += dt;
    return dt;
}
