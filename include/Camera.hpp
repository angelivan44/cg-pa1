// =============================================================
//  Camera.hpp - Camara orbital y proyeccion perspectiva
//
//  La camara gira alrededor de un punto objetivo usando coordenadas
//  esfericas (yaw, pitch, distancia) y gluLookAt. La proyeccion es
//  perspectiva con gluPerspective; los planos cercano y lejano
//  definen el volumen de vision (lo que queda fuera se recorta).
// =============================================================
#pragma once

class Camera {
public:
    enum Mode {
        GENERAL = 0,   // todo el sistema, vista en diagonal
        SUPERIOR,      // desde arriba, se ven las orbitas completas
        SEGUIR,        // sigue a un objeto (TAB cambia de objeto)
        RASANTE        // casi al nivel del plano orbital
    };

    Mode  modo;
    float yaw;         // giro horizontal (grados)
    float pitch;       // elevacion (grados)
    float distancia;   // distancia al objetivo
    int   objetivo;    // objeto seguido en modo SEGUIR
    int   nivelCerca;  // indice del plano cercano (prueba de clipping)
    float ojo[3];      // posicion calculada del observador (mundo)

    Camera();

    // radioObjetivo: tamano del objeto seguido, para elegir la distancia
    void  setMode(Mode m, float radioObjetivo);
    void  applyProjection(int ancho, int alto) const;
    void  applyView(const float objetivoMundo[3]);   // gluLookAt
    void  rotate(float dYaw, float dPitch);
    void  zoom(float factor);
    float nearPlane() const;
    const char* modeName() const;
};
