// =============================================================
//  Material.cpp - Propiedades de material (modelo de Phong)
// =============================================================
#include "Material.hpp"
#include "GLHeaders.hpp"

void Material::apply() const {
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   difuso);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  especular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  emision);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, brillo);
}

// El difuso es blanco porque el color real lo pone la textura:
// con GL_MODULATE el color final = iluminacion * color de la textura.
Material Material::planet(bool brillante) {
    Material mate = {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        1.0f
    };
    Material brillo = {
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.7f, 0.7f, 0.7f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        40.0f
    };
    return brillante ? brillo : mate;
}

Material Material::sun() {
    Material m = {
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        0.0f
    };
    return m;
}

Material Material::rings() {
    Material m = {
        { 0.6f, 0.6f, 0.6f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { 0.1f, 0.1f, 0.1f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        8.0f
    };
    return m;
}
