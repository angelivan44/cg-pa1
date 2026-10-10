// =============================================================
//  Material.hpp - Propiedades de material (modelo de Phong)
//
//  Cada objeto del editor tiene su propio material. Define como
//  refleja la luz:
//    color     -> color base del objeto
//    ambiente  -> cuanto refleja la luz indirecta (ka)
//    difuso    -> reflexion Lambert, depende del angulo N.L (kd)
//    especular -> brillo, depende del angulo R.V y del exponente (ks)
//    brillo    -> exponente especular (0..128, mas alto = brillo mas chico)
//    emision   -> luz propia (el objeto se ve aunque no haya luces)
//    opacidad  -> 1 = opaco; menos de 1 se mezcla con lo que hay detras
// =============================================================
#pragma once

struct Material {
    float color[3];
    float ambiente;
    float difuso;
    float especular;
    float brillo;
    float emision;
    float opacidad;
    bool  mezcla;       // dibujar con mezcla alfa (transparencias, texturas RGBA)

    Material();

    // Envia el material a OpenGL. Sin iluminacion solo se usa el color.
    void apply(bool iluminacion) const;
    bool isTransparent() const { return mezcla || opacidad < 0.999f; }

    // Materiales predefinidos para elegir desde el editor
    static int         presetCount();
    static const char* presetName(int i);
    static Material    preset(int i, const Material& actual);
};
