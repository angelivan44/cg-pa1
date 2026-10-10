// =============================================================
//  Camera.cpp - Camara orbital del Viewport y proyecciones
// =============================================================
#include "Camera.hpp"
#include "GLHeaders.hpp"

Camera::Camera() : altoVentana(1) {
    for (int i = 0; i < 16; i++) modelo[i] = proyeccion[i] = (i % 5 == 0) ? 1.0 : 0.0;
    viewport[0] = viewport[1] = 0;
    viewport[2] = viewport[3] = 1;
    reset();
}

void Camera::reset() {
    ortogonal = false;
    yaw = 35.0f;
    pitch = 25.0f;
    distancia = 12.0f;
    objetivo = Vec3(0.0f, 0.5f, 0.0f);
    fov = 50.0f;
    cerca = 0.1f;
    lejos = 1500.0f;
}

void Camera::rotate(float dYaw, float dPitch) {
    yaw += dYaw;
    pitch += dPitch;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void Camera::setView(float nuevoYaw, float nuevoPitch) {
    yaw = nuevoYaw;
    pitch = nuevoPitch;
}

void Camera::zoom(float factor) {
    distancia *= factor;
    if (distancia < 0.2f) distancia = 0.2f;
    if (distancia > 1000.0f) distancia = 1000.0f;
}

Vec3 Camera::right() const {
    float y = radians(yaw);
    return Vec3(cosf(y), 0.0f, -sinf(y));
}

Vec3 Camera::up() const {
    float y = radians(yaw), p = radians(pitch);
    return Vec3(-sinf(p) * sinf(y), cosf(p), -sinf(p) * cosf(y));
}

// El objetivo se mueve en el plano de la pantalla; el paso depende de
// la distancia para que el arrastre siga al mouse.
void Camera::pan(float dx, float dy) {
    float paso = worldPerPixel(objetivo);
    objetivo -= right() * (dx * paso);
    objetivo += up() * (dy * paso);
}

void Camera::focus(const Vec3& punto, float radio) {
    objetivo = punto;
    distancia = radio * 3.5f;
    if (distancia < 1.0f) distancia = 1.0f;
}

void Camera::apply(int x, int y, int ancho, int alto) {
    if (ancho < 1) ancho = 1;
    if (alto < 1) alto = 1;
    altoVentana = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(x, y, ancho, alto);

    double aspecto = (double)ancho / alto;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (ortogonal) {
        // La mitad del alto se calcula con la distancia para que al
        // cambiar de proyeccion el objetivo se vea del mismo tamano.
        double mitad = distancia * tan(radians(fov) * 0.5);
        glOrtho(-mitad * aspecto, mitad * aspecto, -mitad, mitad, -lejos, lejos);
    } else {
        gluPerspective(fov, aspecto, cerca, lejos);
    }

    // Coordenadas esfericas -> cartesianas alrededor del objetivo
    float ry = radians(yaw), rp = radians(pitch);
    ojo.x = objetivo.x + distancia * cosf(rp) * sinf(ry);
    ojo.y = objetivo.y + distancia * sinf(rp);
    ojo.z = objetivo.z + distancia * cosf(rp) * cosf(ry);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(ojo.x, ojo.y, ojo.z,
              objetivo.x, objetivo.y, objetivo.z,
              0.0, 1.0, 0.0);

    // Se guardan las matrices para convertir entre pantalla y mundo
    glGetDoublev(GL_MODELVIEW_MATRIX, modelo);
    glGetDoublev(GL_PROJECTION_MATRIX, proyeccion);
    glGetIntegerv(GL_VIEWPORT, viewport);
}

bool Camera::project(const Vec3& mundo, float& sx, float& sy) const {
    double x, y, z;
    if (gluProject(mundo.x, mundo.y, mundo.z, modelo, proyeccion, viewport, &x, &y, &z) != GL_TRUE)
        return false;
    sx = (float)x;
    sy = (float)(altoVentana - y);          // OpenGL mide Y desde abajo
    return z > 0.0 && z < 1.0;
}

// Rayo que sale del pixel: se "des-proyecta" el pixel en el plano
// cercano (z = 0) y en el lejano (z = 1) y se unen los dos puntos.
void Camera::screenRay(int mx, int my, Vec3& origen, Vec3& direccion) const {
    double x0, y0, z0, x1, y1, z1;
    double wy = altoVentana - my;
    gluUnProject(mx, wy, 0.0, modelo, proyeccion, viewport, &x0, &y0, &z0);
    gluUnProject(mx, wy, 1.0, modelo, proyeccion, viewport, &x1, &y1, &z1);
    origen = Vec3((float)x0, (float)y0, (float)z0);
    direccion = Vec3((float)(x1 - x0), (float)(y1 - y0), (float)(z1 - z0)).normalized();
}

bool Camera::insideViewport(int mx, int my) const {
    int wy = altoVentana - my;
    return mx >= viewport[0] && mx < viewport[0] + viewport[2] &&
           wy >= viewport[1] && wy < viewport[1] + viewport[3];
}

float Camera::worldPerPixel(const Vec3& punto) const {
    float alto = viewport[3] > 0 ? (float)viewport[3] : 1.0f;
    float d = ortogonal ? distancia : (punto - ojo).length();
    return 2.0f * d * tanf(radians(fov) * 0.5f) / alto;
}
