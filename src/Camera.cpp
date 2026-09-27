// =============================================================
//  Camera.cpp - Camara orbital y proyeccion perspectiva
// =============================================================
#include "Camera.hpp"
#include "GLHeaders.hpp"

#include <cmath>

static const float FOV_Y          = 50.0f;     // campo de vision vertical (grados)
static const float PLANO_LEJANO   = 1500.0f;
static const float PLANOS_CERCA[] = { 0.1f, 20.0f, 60.0f };

Camera::Camera()
    : modo(GENERAL), yaw(0), pitch(0), distancia(1), objetivo(0), nivelCerca(0) {
    ojo[0] = ojo[1] = ojo[2] = 0.0f;
    setMode(GENERAL, 1.0f);
}

float Camera::nearPlane() const {
    return PLANOS_CERCA[nivelCerca];
}

void Camera::setMode(Mode m, float radioObjetivo) {
    modo = m;
    switch (m) {
        case GENERAL:  yaw = 30.0f; pitch = 32.0f; distancia = 36.0f; break;
        case SUPERIOR: yaw = 0.0f;  pitch = 89.0f; distancia = 58.0f; break;
        case SEGUIR:   yaw = 20.0f; pitch = 15.0f; distancia = radioObjetivo * 5.0f; break;
        case RASANTE:  yaw = 10.0f; pitch = 4.0f;  distancia = 34.0f; break;
    }
}

void Camera::rotate(float dYaw, float dPitch) {
    yaw += dYaw;
    pitch += dPitch;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::zoom(float factor) {
    distancia *= factor;
    if (distancia < 0.5f) distancia = 0.5f;
}

void Camera::applyProjection(int ancho, int alto) const {
    if (alto == 0) alto = 1;
    glViewport(0, 0, ancho, alto);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV_Y, (double)ancho / alto, nearPlane(), PLANO_LEJANO);
    glMatrixMode(GL_MODELVIEW);
}

void Camera::applyView(const float o[3]) {
    // Coordenadas esfericas -> cartesianas alrededor del objetivo
    float y = yaw * 3.14159265f / 180.0f;
    float p = pitch * 3.14159265f / 180.0f;
    ojo[0] = o[0] + distancia * cosf(p) * sinf(y);
    ojo[1] = o[1] + distancia * sinf(p);
    ojo[2] = o[2] + distancia * cosf(p) * cosf(y);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(ojo[0], ojo[1], ojo[2],
              o[0], o[1], o[2],
              0.0, 1.0, 0.0);
}
