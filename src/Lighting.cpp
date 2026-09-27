// =============================================================
//  Lighting.cpp - Fuentes de luz de la escena
// =============================================================
#include "Lighting.hpp"
#include "GLHeaders.hpp"

#include <cmath>

static const float NIVELES_AMBIENTE[] = { 0.0f, 0.06f, 0.30f };

Lighting::Lighting() {
    reset();
}

void Lighting::reset() {
    solActiva     = true;
    rellenoActivo = true;
    nivelAmbiente = 1;
    atenuacion    = true;
    anguloRelleno = 35.0f;
}

float Lighting::ambientValue() const {
    return NIVELES_AMBIENTE[nivelAmbiente];
}

void Lighting::init() const {
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);   // los objetos se escalan: hay que renormalizar N

    // Luz del Sol: blanca con un poco de amarillo
    float solDifusa[]    = { 1.00f, 0.95f, 0.85f, 1.0f };
    float solEspecular[] = { 1.00f, 1.00f, 1.00f, 1.0f };
    float sinAmbiente[]  = { 0.00f, 0.00f, 0.00f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  solDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, solEspecular);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  sinAmbiente);

    // Luz de relleno: azulada, debil y sin brillo especular
    float rellenoDifusa[]    = { 0.16f, 0.19f, 0.30f, 1.0f };
    float rellenoEspecular[] = { 0.00f, 0.00f, 0.00f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  rellenoDifusa);
    glLightfv(GL_LIGHT1, GL_SPECULAR, rellenoEspecular);
    glLightfv(GL_LIGHT1, GL_AMBIENT,  sinAmbiente);

    // El especular se suma DESPUES de la textura; si no, la textura
    // oscura de un planeta tambien apagaria su brillo.
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    // Calcula el especular con la posicion real del observador
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

void Lighting::update() const {
    // Las posiciones se envian con la matriz de vista ya cargada, asi
    // quedan fijas en el mundo y no se mueven con la camara.
    float posSol[] = { 0.0f, 0.0f, 0.0f, 1.0f };         // w = 1: puntual
    glLightfv(GL_LIGHT0, GL_POSITION, posSol);
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.0f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, atenuacion ? 0.00035f : 0.0f);

    float rad = anguloRelleno * 3.14159265f / 180.0f;
    float dirRelleno[] = { cosf(rad), 0.55f, sinf(rad), 0.0f };  // w = 0: direccional
    glLightfv(GL_LIGHT1, GL_POSITION, dirRelleno);

    if (solActiva)     glEnable(GL_LIGHT0); else glDisable(GL_LIGHT0);
    if (rellenoActivo) glEnable(GL_LIGHT1); else glDisable(GL_LIGHT1);

    float a = ambientValue();
    float ambiente[] = { a, a, a, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambiente);
}
