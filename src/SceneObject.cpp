// =============================================================
//  SceneObject.cpp - Un objeto de la escena y sus propiedades
// =============================================================
#include "SceneObject.hpp"

#include <cstring>

struct InfoTipo {
    const char* nombre;
    const char* clave;
    bool        geometria;
    float       mitad[3];     // mitad del tamano de la caja envolvente local
};

// Todas las primitivas caben en un cubo de lado 1 centrado en el
// origen, asi "escala 1" significa lo mismo para cualquiera de ellas.
static const InfoTipo TIPOS[OBJ_NUM_TIPOS] = {
    { "Vacío",     "vacio",    false, { 0.20f, 0.20f, 0.20f } },
    { "Cubo",      "cubo",     true,  { 0.50f, 0.50f, 0.50f } },
    { "Esfera",    "esfera",   true,  { 0.50f, 0.50f, 0.50f } },
    { "Cilindro",  "cilindro", true,  { 0.50f, 0.50f, 0.50f } },
    { "Cono",      "cono",     true,  { 0.50f, 0.50f, 0.50f } },
    { "Plano",     "plano",    true,  { 0.50f, 0.01f, 0.50f } },
    { "Toro",      "toro",     true,  { 0.50f, 0.15f, 0.50f } },
    { "Disco",     "disco",    true,  { 0.50f, 0.01f, 0.50f } },
    { "Modelo",    "modelo",   true,  { 1.00f, 1.00f, 1.00f } },
    { "Luz",       "luz",      false, { 0.25f, 0.25f, 0.25f } },
    { "Emisor",    "emisor",   false, { 0.25f, 0.25f, 0.25f } },
};

const char* objectTypeName(int tipo) {
    return (tipo >= 0 && tipo < OBJ_NUM_TIPOS) ? TIPOS[tipo].nombre : "?";
}

const char* objectTypeKey(int tipo) {
    return (tipo >= 0 && tipo < OBJ_NUM_TIPOS) ? TIPOS[tipo].clave : "?";
}

int objectTypeFromKey(const char* clave) {
    for (int i = 0; i < OBJ_NUM_TIPOS; i++)
        if (strcmp(TIPOS[i].clave, clave) == 0) return i;
    return -1;
}

bool objectHasGeometry(int tipo) {
    return tipo >= 0 && tipo < OBJ_NUM_TIPOS && TIPOS[tipo].geometria;
}

LightProps::LightProps()
    : tipo(LUZ_PUNTUAL), encendida(true), intensidad(1.0f), ambiente(0.0f), especular(1.0f),
      atenLineal(0.0f), atenCuadratica(0.0f), anguloFoco(30.0f), exponenteFoco(8.0f) {
    color[0] = color[1] = color[2] = 1.0f;
}

EmitterProps::EmitterProps()
    : tasa(60.0f), vida(1.5f), velocidad(3.0f), dispersion(20.0f), gravedad(2.0f),
      tamInicio(0.35f), tamFin(0.05f), aditivo(true) {
    const float c0[4] = { 1.0f, 0.75f, 0.25f, 1.0f };
    const float c1[4] = { 1.0f, 0.15f, 0.05f, 0.0f };
    for (int k = 0; k < 4; k++) { colorInicio[k] = c0[k]; colorFin[k] = c1[k]; }
}

PhysicsProps::PhysicsProps()
    : modo(FISICA_NINGUNA), masa(1.0f), rebote(0.6f), friccion(0.3f), enContacto(false) {}

SceneObject::SceneObject()
    : id(0), tipo(OBJ_VACIO), padre(0), visible(true), escala(1.0f, 1.0f, 1.0f),
      usarMaterialModelo(true), escAnim(1.0f, 1.0f, 1.0f), sello(-1),
      limMin(-1.0f, -1.0f, -1.0f), limMax(1.0f, 1.0f, 1.0f) {
    texRepetir[0] = texRepetir[1] = 1.0f;
}

void SceneObject::localBounds(Vec3& minimo, Vec3& maximo) const {
    if (tipo == OBJ_MODELO) {
        minimo = limMin;
        maximo = limMax;
        return;
    }
    const float* m = TIPOS[tipo].mitad;
    minimo = Vec3(-m[0], -m[1], -m[2]);
    maximo = Vec3( m[0],  m[1],  m[2]);
}

// Se transforman las 8 esquinas de la caja local y se toma la caja
// alineada a los ejes que las contiene.
void SceneObject::worldBounds(Vec3& minimo, Vec3& maximo) const {
    Vec3 a, b;
    localBounds(a, b);
    minimo = Vec3( 1e30f,  1e30f,  1e30f);
    maximo = Vec3(-1e30f, -1e30f, -1e30f);
    for (int i = 0; i < 8; i++) {
        Vec3 esquina((i & 1) ? b.x : a.x, (i & 2) ? b.y : a.y, (i & 4) ? b.z : a.z);
        Vec3 p = mundo.point(esquina);
        for (int k = 0; k < 3; k++) {
            if (p[k] < minimo[k]) minimo[k] = p[k];
            if (p[k] > maximo[k]) maximo[k] = p[k];
        }
    }
}
