// =============================================================
//  Material.cpp - Propiedades de material (modelo de Phong)
// =============================================================
#include "Material.hpp"
#include "GLHeaders.hpp"

Material::Material()
    : ambiente(0.6f), difuso(0.8f), especular(0.3f), brillo(32.0f),
      emision(0.0f), opacidad(1.0f), mezcla(false) {
    color[0] = color[1] = color[2] = 0.8f;
}

// Con GL_MODULATE el color final = iluminacion * color de la textura,
// por eso el color del material tambien tine a la textura.
void Material::apply(bool iluminacion) const {
    if (!iluminacion) {
        glColor4f(color[0], color[1], color[2], opacidad);
        return;
    }
    float a[4], d[4], s[4], e[4];
    for (int k = 0; k < 3; k++) {
        a[k] = color[k] * ambiente;
        d[k] = color[k] * difuso;
        s[k] = especular;
        e[k] = color[k] * emision;
    }
    a[3] = s[3] = e[3] = 1.0f;
    d[3] = opacidad;                 // el alfa del difuso es la opacidad
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   a);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   d);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  s);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  e);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, brillo);
}

// ------------------------------------------------------------------
//  Materiales predefinidos
// ------------------------------------------------------------------
struct Preajuste {
    const char* nombre;
    bool  cambiaColor;
    float color[3];
    float ambiente, difuso, especular, brillo, emision, opacidad;
};

static const Preajuste PREAJUSTES[] = {
    { "Por defecto",   false, { 0.80f, 0.80f, 0.80f }, 0.6f, 0.8f, 0.3f,  32.0f, 0.0f, 1.0f },
    { "Mate (yeso)",   false, { 0.90f, 0.90f, 0.88f }, 0.6f, 0.9f, 0.0f,   1.0f, 0.0f, 1.0f },
    { "Plastico",      false, { 0.80f, 0.10f, 0.10f }, 0.5f, 0.8f, 0.6f,  48.0f, 0.0f, 1.0f },
    { "Metal pulido",  true,  { 0.75f, 0.77f, 0.80f }, 0.4f, 0.5f, 1.0f, 100.0f, 0.0f, 1.0f },
    { "Oro",           true,  { 0.90f, 0.70f, 0.20f }, 0.5f, 0.7f, 0.9f,  60.0f, 0.0f, 1.0f },
    { "Goma",          false, { 0.15f, 0.15f, 0.15f }, 0.5f, 0.7f, 0.1f,   8.0f, 0.0f, 1.0f },
    { "Emisivo",       false, { 1.00f, 0.90f, 0.60f }, 0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f },
    { "Vidrio",        true,  { 0.70f, 0.85f, 0.95f }, 0.5f, 0.6f, 1.0f,  90.0f, 0.0f, 0.35f },
};

int Material::presetCount() {
    return (int)(sizeof(PREAJUSTES) / sizeof(PREAJUSTES[0]));
}

const char* Material::presetName(int i) {
    return PREAJUSTES[i].nombre;
}

Material Material::preset(int i, const Material& actual) {
    const Preajuste& p = PREAJUSTES[i];
    Material m = actual;
    if (p.cambiaColor)
        for (int k = 0; k < 3; k++) m.color[k] = p.color[k];
    m.ambiente  = p.ambiente;
    m.difuso    = p.difuso;
    m.especular = p.especular;
    m.brillo    = p.brillo;
    m.emision   = p.emision;
    m.opacidad  = p.opacidad;
    m.mezcla    = p.opacidad < 1.0f;
    return m;
}
