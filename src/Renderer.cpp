// =============================================================
//  Renderer.cpp - Dibujo de la escena en el Viewport
// =============================================================
#include "Renderer.hpp"
#include "Primitives.hpp"
#include "Texture.hpp"

#include <algorithm>

void Renderer::init() const {
    glEnable(GL_DEPTH_TEST);          // visibilidad por Z-buffer
    glDepthFunc(GL_LEQUAL);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);   // textura * iluminacion
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

// ------------------------------------------------------------------
//  Cuadricula del suelo y ejes del mundo (X rojo, Z azul)
// ------------------------------------------------------------------
void Renderer::drawGrid() const {
    const int   MITAD = 20;
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -MITAD; i <= MITAD; i++) {
        if (i == 0) continue;
        float alfa = (i % 5 == 0) ? 0.35f : 0.16f;
        glColor4f(0.6f, 0.65f, 0.75f, alfa);
        glVertex3f((float)i, 0.0f, (float)-MITAD); glVertex3f((float)i, 0.0f, (float)MITAD);
        glVertex3f((float)-MITAD, 0.0f, (float)i); glVertex3f((float)MITAD, 0.0f, (float)i);
    }
    glColor4f(0.85f, 0.25f, 0.25f, 0.8f);
    glVertex3f((float)-MITAD, 0.0f, 0.0f); glVertex3f((float)MITAD, 0.0f, 0.0f);
    glColor4f(0.25f, 0.45f, 0.95f, 0.8f);
    glVertex3f(0.0f, 0.0f, (float)-MITAD); glVertex3f(0.0f, 0.0f, (float)MITAD);
    glEnd();
    glDisable(GL_BLEND);
}

// ------------------------------------------------------------------
//  Un objeto con geometria
// ------------------------------------------------------------------
void Renderer::drawObject(const SceneObject& o, Resources& recursos, const RenderOptions& op) const {
    glPushMatrix();
    glMultMatrixf(o.mundo.m);          // transformacion jerarquica ya resuelta en la CPU

    Model* modelo = (o.tipo == OBJ_MODELO) ? recursos.model(o.modelo) : 0;
    if (modelo && o.usarMaterialModelo && op.iluminacion) {
        // Modelo con los materiales y texturas de su archivo MTL
        modelo->draw(op.texturas ? Model::CON_TEXTURAS : Model::SIN_TEXTURAS);
        glPopMatrix();
        return;
    }

    o.material.apply(op.iluminacion);
    // En wireframe no se aplica la textura: las lineas se ven con el color del material
    GLuint textura = recursos.texture(o.textura);
    bool conTextura = op.texturas && !op.wireframe && textura != 0;
    Texture::bind(textura, conTextura);
    if (conTextura) {
        // La matriz de textura repite la imagen sobre la superficie
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glScalef(o.texRepetir[0], o.texRepetir[1], 1.0f);
        glMatrixMode(GL_MODELVIEW);
    }

    if (o.tipo == OBJ_MODELO) {
        if (modelo) {
            modelo->draw(Model::GEOMETRIA);
        } else {
            // El archivo no se pudo cargar: se muestra una caja en su lugar
            const float a[3] = { -0.5f, -0.5f, -0.5f }, b[3] = { 0.5f, 0.5f, 0.5f };
            Primitives::wireBox(a, b);
        }
    } else {
        // El plano y el disco no tienen grosor: se iluminan por los dos lados
        bool dosCaras = o.tipo == OBJ_PLANO || o.tipo == OBJ_DISCO;
        if (dosCaras) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        Primitives::draw(o.tipo, op.divisions());
        if (dosCaras) glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    }

    if (conTextura) {
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

struct PorDistancia {
    Vec3 ojo;
    bool operator()(const SceneObject* a, const SceneObject* b) const {
        return (a->mundo.origin() - ojo).length() > (b->mundo.origin() - ojo).length();
    }
};

void Renderer::drawObjects(const Scene& escena, Resources& recursos,
                           const RenderOptions& op, const Vec3& ojo) const {
    if (op.depthBuffer) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (op.iluminacion) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
    glShadeModel(op.gouraud ? GL_SMOOTH : GL_FLAT);
    glPolygonMode(GL_FRONT_AND_BACK, op.wireframe ? GL_LINE : GL_FILL);
    glLineWidth(1.0f);

    // 1. Opacos
    std::vector<const SceneObject*> transparentes;
    for (size_t i = 0; i < escena.objetos.size(); i++) {
        const SceneObject* o = escena.objetos[i];
        if (!o->visible || !objectHasGeometry(o->tipo)) continue;
        bool usaMTL = o->tipo == OBJ_MODELO && o->usarMaterialModelo;
        if (!usaMTL && o->material.isTransparent()) transparentes.push_back(o);
        else drawObject(*o, recursos, op);
    }

    // 2. Transparentes, del mas lejano al mas cercano
    if (!transparentes.empty()) {
        PorDistancia orden;
        orden.ojo = ojo;
        std::sort(transparentes.begin(), transparentes.end(), orden);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);         // se prueba la profundidad pero no se escribe
        for (size_t i = 0; i < transparentes.size(); i++) drawObject(*transparentes[i], recursos, op);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

// ------------------------------------------------------------------
//  Ayudas del editor (no forman parte de la escena final)
// ------------------------------------------------------------------
static void circle(float radio, int eje) {
    glBegin(GL_LINE_LOOP);
    for (int k = 0; k < 24; k++) {
        float a = k * 2.0f * PI / 24.0f;
        float c = radio * cosf(a), s = radio * sinf(a);
        if (eje == 0)      glVertex3f(0.0f, c, s);
        else if (eje == 1) glVertex3f(c, 0.0f, s);
        else               glVertex3f(c, s, 0.0f);
    }
    glEnd();
}

void Renderer::drawIcon(const SceneObject& o) const {
    // El icono no debe deformarse con la escala del objeto: se usan
    // la posicion y la orientacion de la matriz de mundo, sin la escala.
    Vec3 p = o.mundo.origin();
    Vec3 x = o.mundo.axis(0).normalized(), y = o.mundo.axis(1).normalized(), z = o.mundo.axis(2).normalized();
    float m[16] = { x.x, x.y, x.z, 0,  y.x, y.y, y.z, 0,  z.x, z.y, z.z, 0,  p.x, p.y, p.z, 1 };
    glPushMatrix();
    glMultMatrixf(m);

    if (o.tipo == OBJ_VACIO) {
        glBegin(GL_LINES);
        glColor3f(0.9f, 0.3f, 0.3f); glVertex3f(-0.2f, 0, 0); glVertex3f(0.2f, 0, 0);
        glColor3f(0.4f, 0.9f, 0.4f); glVertex3f(0, -0.2f, 0); glVertex3f(0, 0.2f, 0);
        glColor3f(0.4f, 0.55f, 1.0f); glVertex3f(0, 0, -0.2f); glVertex3f(0, 0, 0.2f);
        glEnd();
    } else if (o.tipo == OBJ_LUZ) {
        const LightProps& l = o.luz;
        float k = l.encendida ? 1.0f : 0.35f;
        glColor3f(l.color[0] * k, l.color[1] * k, l.color[2] * k);
        for (int eje = 0; eje < 3; eje++) circle(0.18f, eje);
        if (l.tipo == LUZ_PUNTUAL) {
            glBegin(GL_LINES);                 // rayos en todas direcciones
            for (int i = 0; i < 6; i++) {
                Vec3 d;
                d[i / 2] = (i % 2) ? -1.0f : 1.0f;
                glVertex3f(d.x * 0.24f, d.y * 0.24f, d.z * 0.24f);
                glVertex3f(d.x * 0.38f, d.y * 0.38f, d.z * 0.38f);
            }
            glEnd();
        } else {
            // Direccional y foco: flecha hacia donde apunta (-Y local)
            float abre = l.tipo == LUZ_FOCO ? tanf(radians(fminf(l.anguloFoco, 80.0f))) * 1.2f : 0.0f;
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0); glVertex3f(0, -1.2f, 0);
            for (int i = 0; i < 4; i++) {
                float a = i * PI * 0.5f;
                float dx = cosf(a), dz = sinf(a);
                if (l.tipo == LUZ_FOCO) {
                    glVertex3f(0, 0, 0); glVertex3f(dx * abre, -1.2f, dz * abre);
                } else {
                    glVertex3f(dx * 0.18f, 0, dz * 0.18f); glVertex3f(dx * 0.18f, -0.9f, dz * 0.18f);
                }
            }
            glEnd();
            if (l.tipo == LUZ_FOCO) {
                glTranslatef(0.0f, -1.2f, 0.0f);
                circle(abre, 1);
            }
        }
    } else if (o.tipo == OBJ_EMISOR) {
        glColor3f(1.0f, 0.6f, 0.2f);
        circle(0.2f, 1);
        float abre = 0.2f + 0.5f * tanf(radians(fminf(o.emisor.dispersion, 80.0f)));
        glBegin(GL_LINES);
        for (int i = 0; i < 4; i++) {
            float a = i * PI * 0.5f;
            glVertex3f(cosf(a) * 0.2f, 0, sinf(a) * 0.2f);
            glVertex3f(cosf(a) * abre, 0.5f, sinf(a) * abre);
        }
        glEnd();
        glTranslatef(0.0f, 0.5f, 0.0f);
        circle(abre, 1);
    }
    glPopMatrix();
}

void Renderer::drawHelpers(const Scene& escena, const RenderOptions& op) const {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.5f);
    if (op.ayudas) {
        for (size_t i = 0; i < escena.objetos.size(); i++) {
            const SceneObject* o = escena.objetos[i];
            if (o->visible && !objectHasGeometry(o->tipo)) drawIcon(*o);
        }
    }

    // Caja envolvente del objeto seleccionado
    const SceneObject* s = escena.selected();
    if (s) {
        Vec3 a, b;
        s->localBounds(a, b);
        glPushMatrix();
        glMultMatrixf(s->mundo.m);
        glColor3f(1.0f, 0.62f, 0.1f);
        glLineWidth(2.0f);
        Primitives::wireBox(&a.x, &b.x);
        glPopMatrix();
    }
    glLineWidth(1.0f);
}
