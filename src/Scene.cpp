// =============================================================
//  Scene.cpp - Construccion y renderizado del sistema solar
// =============================================================
#include "Scene.hpp"
#include "Material.hpp"
#include "Texture.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>

static const float PI = 3.14159265f;

// Indices de los cuerpos (mismo orden que la tabla del constructor)
enum {
    SOL, MERCURIO, VENUS, TIERRA, LUNA, ISS, HUBBLE, MARTE, BENNU,
    JUPITER, SATURNO, CASSINI, URANO, NEPTUNO, VOYAGER, NUM_CUERPOS
};

// Anillos de Saturno (radio interno y externo, en radios de Saturno)
static const float ANILLO_INTERNO = 1.25f;
static const float ANILLO_EXTERNO = 2.35f;

Scene::Scene() : texAnillos(0), texHalo(0), cuadrica(0), tiempo(0.0f) {
    const Body tabla[NUM_CUERPOS] = {
    //   nombre      padre    dist  vOrb  fase  radio  vRot   eje    color                 textura
        { "Sol",      -1,     0.0f,  0.0f,   0, 3.00f,  4.0f,   7.3f, {1.0f, 0.8f, 0.3f}, "textures/2k_sun.jpg", 0, 0 },
        { "Mercurio", -1,     5.6f, 24.0f,  40, 0.35f, 12.0f,   0.0f, {0.6f, 0.6f, 0.6f}, "textures/2k_mercury.jpg", 0, 0 },
        { "Venus",    -1,     8.2f, 16.0f, 160, 0.60f, -6.0f,   2.6f, {0.9f, 0.7f, 0.4f}, "textures/2k_venus_atmosphere.jpg", 0, 0 },
        { "Tierra",   -1,    11.5f, 10.0f, 250, 0.65f, 45.0f,  23.4f, {0.2f, 0.4f, 0.9f}, "textures/2k_earth_daymap.jpg", 0, 0 },
        { "Luna",     TIERRA, 1.8f, 22.0f,   0, 0.18f,  0.0f,   6.7f, {0.7f, 0.7f, 0.7f}, "textures/2k_moon.jpg", 0, 0 },
        { "ISS",      TIERRA, 0.95f,70.0f,  90, 0.14f,  0.0f,   0.0f, {0.8f, 0.8f, 0.8f}, 0, &modeloISS, 0 },
        { "Hubble",   TIERRA, 1.25f,-45.0f,200, 0.11f,  0.0f,   0.0f, {0.8f, 0.8f, 0.8f}, 0, &modeloHubble, 0 },
        { "Marte",    -1,    14.5f,  7.0f, 320, 0.45f, 40.0f,  25.2f, {0.8f, 0.35f, 0.2f}, "textures/2k_mars.jpg", 0, 0 },
        { "Bennu",    -1,    18.0f,  4.0f, 210, 0.30f, 30.0f,  10.0f, {0.6f, 0.55f, 0.5f}, 0, &modeloBennu, 0 },
        { "Jupiter",  -1,    24.0f,  3.5f,  60, 1.70f, 70.0f,   3.1f, {0.8f, 0.7f, 0.55f}, "textures/2k_jupiter.jpg", 0, 0 },
        { "Saturno",  -1,    31.0f,  2.4f, 145, 1.40f, 65.0f,  26.7f, {0.9f, 0.8f, 0.55f}, "textures/2k_saturn.jpg", 0, 0 },
        { "Cassini",  SATURNO,3.6f, 18.0f,  30, 0.30f,  0.0f,   0.0f, {0.8f, 0.8f, 0.8f}, 0, &modeloCassini, 0 },
        { "Urano",    -1,    37.5f,  1.6f, 280, 0.95f,-45.0f,  97.8f, {0.6f, 0.85f, 0.9f}, "textures/2k_uranus.jpg", 0, 0 },
        { "Neptuno",  -1,    43.0f,  1.2f,  20, 0.90f, 42.0f,  28.3f, {0.3f, 0.45f, 0.9f}, "textures/2k_neptune.jpg", 0, 0 },
        { "Voyager",  -1,    49.0f,  0.6f, 100, 0.45f,  6.0f,   0.0f, {0.8f, 0.8f, 0.8f}, 0, &modeloVoyager, 0 },
    };
    cuerpos.assign(tabla, tabla + NUM_CUERPOS);
}

Scene::~Scene() {
    if (cuadrica) gluDeleteQuadric(cuadrica);
}

// Numero pseudoaleatorio reproducible (siempre el mismo cinturon)
static unsigned int semilla = 12345u;
static float aleatorio(float a, float b) {
    semilla = semilla * 1103515245u + 12345u;
    return a + (b - a) * ((semilla >> 8) & 0xFFFF) / 65535.0f;
}

void Scene::init() {
    printf("Cargando texturas...\n");
    for (size_t i = 0; i < cuerpos.size(); i++)
        if (cuerpos[i].archivo) cuerpos[i].textura = Texture::load(assetPath(cuerpos[i].archivo), true);
    texAnillos   = Texture::load(assetPath("textures/2k_saturn_ring_alpha.png"), false);
    texHalo      = Texture::createGlow(128);

    printf("Cargando modelos 3D (NASA 3D Resources, convertidos a OBJ)...\n");
    modeloISS.load(assetPath("models/iss.obj"));
    modeloHubble.load(assetPath("models/hubble.obj"));
    modeloCassini.load(assetPath("models/cassini.obj"));
    modeloVoyager.load(assetPath("models/voyager.obj"));
    modeloBennu.load(assetPath("models/asteroide_bennu.obj"));

    cuadrica = gluNewQuadric();
    gluQuadricTexture(cuadrica, GL_TRUE);    // genera coordenadas (s,t) en la esfera

    for (int i = 0; i < 90; i++) {
        Asteroid a;
        a.distancia = aleatorio(16.3f, 20.0f);
        a.fase      = aleatorio(0.0f, 360.0f);
        a.altura    = aleatorio(-0.45f, 0.45f);
        a.escala    = aleatorio(0.05f, 0.15f);
        a.velOrbita = 4.0f * powf(18.0f / a.distancia, 1.5f);   // mas lejos, mas lento
        a.eje[0] = aleatorio(-1, 1); a.eje[1] = aleatorio(-1, 1); a.eje[2] = aleatorio(-1, 1);
        a.velGiro   = aleatorio(20.0f, 90.0f);
        cinturon.push_back(a);
    }
}

void Scene::update(float segundos)  { tiempo += segundos; }
void Scene::setTime(float segundos) { tiempo = segundos; }

// ------------------------------------------------------------------
//  Transformaciones jerarquicas
//  Para un cuerpo con padre se aplica primero la orbita del padre y
//  luego la propia:  M = Orbita(padre) * Orbita(cuerpo)
//  Orbita = R(angulo) * T(distancia) * R(-angulo)
//  La rotacion de vuelta R(-angulo) mantiene el eje inclinado siempre
//  apuntando al mismo lado del espacio (como el eje real de la Tierra).
// ------------------------------------------------------------------
float Scene::orbitAngle(int i) const {
    return cuerpos[i].fase + cuerpos[i].velOrbita * tiempo;
}

void Scene::applyOrbit(int i) const {
    if (cuerpos[i].padre >= 0) applyOrbit(cuerpos[i].padre);
    float a = orbitAngle(i);
    glRotatef(a, 0.0f, 1.0f, 0.0f);
    glTranslatef(cuerpos[i].distancia, 0.0f, 0.0f);
    glRotatef(-a, 0.0f, 1.0f, 0.0f);
}

// Misma cuenta que applyOrbit pero en la CPU (para la camara):
// girar (d, 0, 0) un angulo a sobre Y da (d cos a, 0, -d sin a)
void Scene::bodyPosition(int i, float p[3]) const {
    p[0] = p[1] = p[2] = 0.0f;
    if (cuerpos[i].padre >= 0) bodyPosition(cuerpos[i].padre, p);
    float a = orbitAngle(i) * PI / 180.0f;
    p[0] += cuerpos[i].distancia * cosf(a);
    p[2] -= cuerpos[i].distancia * sinf(a);
}

int         Scene::bodyCount() const       { return (int)cuerpos.size(); }
const char* Scene::bodyName(int i) const   { return cuerpos[i].nombre; }
float       Scene::bodyRadius(int i) const { return cuerpos[i].radio < 0.3f ? 0.3f : cuerpos[i].radio; }

int Scene::findBody(const char* nombre) const {
    for (size_t i = 0; i < cuerpos.size(); i++)
        if (strcmp(cuerpos[i].nombre, nombre) == 0) return (int)i;
    return 0;
}

// ------------------------------------------------------------------
//  Dibujo de cada parte
// ------------------------------------------------------------------
void Scene::drawSphere(float radio, const RenderOptions& o) const {
    int n = o.sphereDivisions();
    // Normales por vertice (suave) o una por cara (plano)
    gluQuadricNormals(cuadrica, o.gouraud ? GLU_SMOOTH : GLU_FLAT);
    glPushMatrix();
    // gluSphere tiene los polos en Z; se giran al eje Y (norte arriba)
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluSphere(cuadrica, radio, n, n / 2);
    glPopMatrix();
}

void Scene::drawOrbits(const RenderOptions& o) const {
    if (!o.orbitas) return;
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.30f, 0.40f);
    for (size_t i = 1; i < cuerpos.size(); i++) {
        if (cuerpos[i].padre != -1) continue;
        glBegin(GL_LINE_LOOP);
        for (int k = 0; k < 180; k++) {
            float a = k * 2.0f * PI / 180.0f;
            glVertex3f(cuerpos[i].distancia * cosf(a), 0.0f, cuerpos[i].distancia * sinf(a));
        }
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

void Scene::drawBody(int i, const RenderOptions& o) const {
    const Body& c = cuerpos[i];
    glPushMatrix();
    applyOrbit(i);
    glRotatef(c.ejeInclinado, 0.0f, 0.0f, 1.0f);           // inclinacion del eje
    glRotatef(c.velRotacion * tiempo, 0.0f, 1.0f, 0.0f);   // giro sobre su eje

    if (c.modelo) {
        glScalef(c.radio, c.radio, c.radio);
        c.modelo->draw(o.texturas);
    } else {
        Material m = (i == SOL) ? Material::sun() : Material::planet(o.materialBrillante);
        if (!o.texturas || c.textura == 0) {
            // Sin textura cada cuerpo usa su propio color de material
            for (int k = 0; k < 3; k++) {
                m.ambiente[k] = c.color[k];
                m.difuso[k] = c.color[k];
                if (i == SOL) m.emision[k] = c.color[k];
            }
        }
        m.apply();
        Texture::bind(c.textura, o.texturas);
        drawSphere(c.radio, o);
        glDisable(GL_TEXTURE_2D);
    }
    glPopMatrix();
}

void Scene::drawAsteroidBelt(const RenderOptions& o) const {
    if (!modeloBennu.isLoaded()) return;
    for (size_t i = 0; i < cinturon.size(); i++) {
        const Asteroid& a = cinturon[i];
        glPushMatrix();
        glRotatef(a.fase + a.velOrbita * tiempo, 0.0f, 1.0f, 0.0f);
        glTranslatef(a.distancia, a.altura, 0.0f);
        glRotatef(a.velGiro * tiempo, a.eje[0], a.eje[1], a.eje[2]);
        glScalef(a.escala, a.escala, a.escala);
        modeloBennu.draw(o.texturas);
        glPopMatrix();
    }
}

// Anillos: corona circular con la textura RGBA de Solar System Scope.
// La coordenada s recorre el radio (del borde interno al externo),
// asi cada franja de la imagen se convierte en un anillo.
void Scene::drawRings(const RenderOptions& o) const {
    const Body& s = cuerpos[SATURNO];
    glPushMatrix();
    applyOrbit(SATURNO);
    glRotatef(s.ejeInclinado, 0.0f, 0.0f, 1.0f);

    if (o.transparencias) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);    // se prueba la profundidad pero no se escribe
    }
    Material m = Material::rings();
    if (!o.texturas)
        for (int k = 0; k < 3; k++) { m.ambiente[k] = 0.5f; m.difuso[k] = 0.85f - 0.1f * k; }
    m.apply();
    Texture::bind(texAnillos, o.texturas);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Primero el borde externo y luego el interno: asi, visto desde +Y,
    // cada cuadrilatero va en sentido antihorario (cara frontal) y
    // coincide con la normal (0,1,0). Con el orden al reves la cara
    // iluminada por el Sol quedaba oscura.
    float rin = s.radio * ANILLO_INTERNO, rout = s.radio * ANILLO_EXTERNO;
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUAD_STRIP);
    for (int k = 0; k <= 128; k++) {
        float a = k * 2.0f * PI / 128.0f;
        float ca = cosf(a), sa = sinf(a);
        glTexCoord2f(1.0f, 0.5f); glVertex3f(rout * ca, 0.0f, rout * sa);
        glTexCoord2f(0.0f, 0.5f); glVertex3f(rin * ca, 0.0f, rin * sa);
    }
    glEnd();

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPopMatrix();
}

// Halo del Sol: un cuadrado siempre de frente a la camara (billboard)
// con mezcla aditiva: suma luz sin tapar lo que hay detras.
void Scene::drawSunGlow(const RenderOptions& o) const {
    if (!o.transparencias || !o.texturas) return;
    float mv[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    // Filas de la parte 3x3 de la vista = ejes derecha y arriba de la camara
    float der[3] = { mv[0], mv[4], mv[8] };
    float arr[3] = { mv[1], mv[5], mv[9] };
    float t = cuerpos[SOL].radio * 2.6f;

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texHalo);
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-t * der[0] - t * arr[0], -t * der[1] - t * arr[1], -t * der[2] - t * arr[2]);
    glTexCoord2f(1, 0); glVertex3f( t * der[0] - t * arr[0],  t * der[1] - t * arr[1],  t * der[2] - t * arr[2]);
    glTexCoord2f(1, 1); glVertex3f( t * der[0] + t * arr[0],  t * der[1] + t * arr[1],  t * der[2] + t * arr[2]);
    glTexCoord2f(0, 1); glVertex3f(-t * der[0] + t * arr[0], -t * der[1] + t * arr[1], -t * der[2] + t * arr[2]);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// ------------------------------------------------------------------
//  Orden de dibujo:
//   1. objetos opacos, en cualquier orden: el depth buffer resuelve
//      que se ve delante (el fondo es el negro de glClearColor)
//   2. objetos transparentes al final, para que se mezclen con lo que
//      ya esta dibujado detras
// ------------------------------------------------------------------
void Scene::draw(const RenderOptions& o) const {
    if (o.depthBuffer) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glShadeModel(o.gouraud ? GL_SMOOTH : GL_FLAT);

    drawOrbits(o);
    for (size_t i = 0; i < cuerpos.size(); i++) drawBody((int)i, o);
    drawAsteroidBelt(o);

    drawRings(o);
    drawSunGlow(o);
}
