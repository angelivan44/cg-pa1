// =============================================================
//  Camera.hpp - Camara orbital del Viewport y proyecciones
//
//  La camara gira alrededor de un punto objetivo usando coordenadas
//  esfericas (yaw, pitch, distancia) y gluLookAt. El objetivo se puede
//  desplazar (paneo) para recorrer la escena.
//
//  Proyeccion:
//    perspectiva -> gluPerspective: lo lejano se ve mas pequeno
//    ortogonal   -> glOrtho: sin perspectiva, las paralelas no se juntan
// =============================================================
#pragma once

#include "Math.hpp"

class Camera {
public:
    bool  ortogonal;
    float yaw;          // giro horizontal (grados)
    float pitch;        // elevacion (grados)
    float distancia;    // distancia al objetivo
    Vec3  objetivo;     // punto que mira la camara
    float fov;          // campo de vision vertical (grados)
    float cerca, lejos; // planos de recorte

    Vec3  ojo;          // posicion calculada del observador (mundo)

    Camera();
    void reset();

    // Envian a OpenGL la proyeccion y la vista para el rectangulo del Viewport
    void apply(int x, int y, int ancho, int alto);

    void rotate(float dYaw, float dPitch);
    void zoom(float factor);
    void pan(float dx, float dy);                 // en pixeles de pantalla
    void focus(const Vec3& punto, float radio);   // encuadra un objeto
    void setView(float nuevoYaw, float nuevoPitch);

    // Ejes de la camara en el mundo
    Vec3 right() const;
    Vec3 up() const;

    // Conversion pantalla <-> mundo con las matrices del ultimo apply().
    // Las coordenadas de pantalla son las de GLUT (origen arriba a la izquierda).
    bool project(const Vec3& mundo, float& sx, float& sy) const;
    void screenRay(int mx, int my, Vec3& origen, Vec3& direccion) const;
    bool insideViewport(int mx, int my) const;
    // Unidades de mundo que mide un pixel a la distancia de "punto"
    float worldPerPixel(const Vec3& punto) const;

private:
    double modelo[16], proyeccion[16];
    int    viewport[4];
    int    altoVentana;
};
