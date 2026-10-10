// =============================================================
//  Gizmo.cpp - Manipulador del objeto seleccionado en el Viewport
// =============================================================
#include "Gizmo.hpp"
#include "GLHeaders.hpp"

static const float LARGO_PIXELES = 90.0f;     // tamano del gizmo en pantalla
static const float RADIO_TOQUE   = 9.0f;      // distancia maxima del cursor al eje

Gizmo::Gizmo() : modo(MOVER), ejeActivo(-1) {}

// Mover usa los ejes del mundo; girar y escalar, los ejes locales del
// objeto (las columnas de su matriz de mundo).
Vec3 Gizmo::axisDirection(const SceneObject& o, int eje) const {
    if (modo == MOVER) {
        Vec3 d;
        d[eje] = 1.0f;
        return d;
    }
    Vec3 d = o.mundo.axis(eje).normalized();
    if (d.length() < 0.5f) d[eje] = 1.0f;
    return d;
}

// Largo en unidades de mundo para que el gizmo mida siempre lo mismo en pantalla
float Gizmo::length(const SceneObject& o, const Camera& camara) const {
    return camara.worldPerPixel(o.mundo.origin()) * LARGO_PIXELES;
}

void Gizmo::draw(const SceneObject& o, const Camera& camara) const {
    static const float COLORES[3][3] = { { 0.95f, 0.25f, 0.25f }, { 0.35f, 0.9f, 0.3f }, { 0.3f, 0.5f, 1.0f } };
    Vec3 origen = o.mundo.origin();
    float largo = length(o, camara);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);          // siempre visible, aunque el objeto lo tape
    glLineWidth(3.0f);
    for (int eje = 0; eje < 3; eje++) {
        Vec3 punta = origen + axisDirection(o, eje) * largo;
        if (eje == ejeActivo) glColor3f(1.0f, 0.9f, 0.2f);
        else                  glColor3fv(COLORES[eje]);
        glBegin(GL_LINES);
        glVertex3f(origen.x, origen.y, origen.z);
        glVertex3f(punta.x, punta.y, punta.z);
        glEnd();

        if (modo == MOVER) {
            // Punta de flecha: glutSolidCone apunta a +Z, se gira hacia el eje
            glPushMatrix();
            glTranslatef(punta.x, punta.y, punta.z);
            if (eje == 0) glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            if (eje == 1) glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glutSolidCone(largo * 0.07, largo * 0.22, 12, 1);
            glPopMatrix();
        } else {
            // Girar: punto redondo. Escalar: punto cuadrado.
            if (modo == ROTAR) glEnable(GL_POINT_SMOOTH); else glDisable(GL_POINT_SMOOTH);
            glPointSize(12.0f);
            glBegin(GL_POINTS);
            glVertex3f(punta.x, punta.y, punta.z);
            glEnd();
        }
    }
    glPointSize(1.0f);
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

// Distancia de un punto al segmento a-b, todo en pixeles
static float distanceToSegment(float px, float py, float ax, float ay, float bx, float by) {
    float vx = bx - ax, vy = by - ay;
    float largo2 = vx * vx + vy * vy;
    float t = largo2 > 0.0f ? ((px - ax) * vx + (py - ay) * vy) / largo2 : 0.0f;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.2f) t = 1.2f;            // incluye la punta de flecha
    float dx = px - (ax + vx * t), dy = py - (ay + vy * t);
    return sqrtf(dx * dx + dy * dy);
}

int Gizmo::hitTest(const SceneObject& o, const Camera& camara, int mx, int my) const {
    Vec3 origen = o.mundo.origin();
    float largo = length(o, camara);
    float ox, oy;
    if (!camara.project(origen, ox, oy)) return -1;

    int   mejor = -1;
    float mejorDistancia = RADIO_TOQUE;
    for (int eje = 0; eje < 3; eje++) {
        float px, py;
        camara.project(origen + axisDirection(o, eje) * largo, px, py);
        float d = distanceToSegment((float)mx, (float)my, ox, oy, px, py);
        if (d < mejorDistancia) {
            mejorDistancia = d;
            mejor = eje;
        }
    }
    return mejor;
}

void Gizmo::drag(const Scene& escena, SceneObject& o, const Camera& camara,
                 int eje, float dx, float dy) const {
    if (eje < 0 || eje > 2) return;
    if (modo == ROTAR) {
        o.rotacion[eje] += (dx - dy) * 0.5f;
        return;
    }

    // Direccion del eje en la pantalla (pixeles)
    Vec3 origen = o.mundo.origin();
    Vec3 dir = axisDirection(o, eje);
    float largo = length(o, camara);
    float ox, oy, px, py;
    camara.project(origen, ox, oy);
    camara.project(origen + dir * largo, px, py);
    float ax = px - ox, ay = py - oy;
    float largo2 = ax * ax + ay * ay;
    if (largo2 < 1.0f) return;         // el eje apunta hacia la camara: no se puede arrastrar

    // Proyeccion del movimiento del mouse sobre el eje (0..1 = un largo de gizmo)
    float avance = (dx * ax + dy * ay) / largo2;

    if (modo == ESCALAR) {
        o.escala[eje] *= 1.0f + avance;
        if (o.escala[eje] < 0.01f) o.escala[eje] = 0.01f;
        return;
    }

    // Mover: el desplazamiento esta en el mundo y la posicion del objeto
    // es relativa a su padre, asi que se pasa al espacio del padre.
    Vec3 desplazamiento = dir * (avance * largo);
    SceneObject* padre = escena.find(o.padre);
    Mat4 inversa;
    if (padre && padre->mundo.inverse(inversa)) desplazamiento = inversa.direction(desplazamiento);
    o.posicion += desplazamiento;
}
