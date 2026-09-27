// =============================================================
//  Material.hpp - Propiedades de material (modelo de Phong)
//
//  Cada material define como refleja la luz un objeto:
//    ambiente  -> luz indirecta que recibe aunque no le de el Sol
//    difuso    -> reflexion Lambert (depende del angulo N.L)
//    especular -> brillo (depende del angulo R.V y del exponente)
//    emision   -> luz propia (el objeto se ve aunque no haya luces)
//    brillo    -> exponente especular (0..128, mas alto = brillo mas chico)
// =============================================================
#pragma once

struct Material {
    float ambiente[4];
    float difuso[4];
    float especular[4];
    float emision[4];
    float brillo;

    void apply() const;

    // Planetas: mate (sin especular) o brillante (especular alto)
    static Material planet(bool brillante);
    // Sol: emisivo, no necesita luces para verse
    static Material sun();
    // Anillos de Saturno: algo de ambiente para que no desaparezcan
    static Material rings();
};
