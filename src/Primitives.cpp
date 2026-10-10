// =============================================================
//  Primitives.cpp - Geometria de las primitivas del editor
// =============================================================
#include "Primitives.hpp"
#include "GLHeaders.hpp"
#include "Math.hpp"
#include "SceneObject.hpp"

// Radio interno del disco respecto al externo (proporcion de los
// anillos de Saturno del PA3)
static const float DISCO_INTERNO = 0.53f;

// Cara rectangular subdividida: parte de "origen" y avanza por los
// lados u y v. Se subdivide porque la luz se calcula por vertice.
static void patch(const Vec3& origen, const Vec3& u, const Vec3& v, const Vec3& normal, int n) {
    glNormal3f(normal.x, normal.y, normal.z);
    for (int j = 0; j < n; j++) {
        float t0 = (float)j / n, t1 = (float)(j + 1) / n;
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= n; i++) {
            float s = (float)i / n;
            Vec3 a = origen + u * s + v * t1;
            Vec3 b = origen + u * s + v * t0;
            glTexCoord2f(s, t1); glVertex3f(a.x, a.y, a.z);
            glTexCoord2f(s, t0); glVertex3f(b.x, b.y, b.z);
        }
        glEnd();
    }
}

void Primitives::cube(int divisiones) {
    int n = divisiones / 6;
    if (n < 1) n = 1;
    const float h = 0.5f;
    patch(Vec3(-h, -h,  h), Vec3( 1, 0, 0), Vec3(0, 1, 0), Vec3( 0,  0,  1), n);   // frente
    patch(Vec3( h, -h, -h), Vec3(-1, 0, 0), Vec3(0, 1, 0), Vec3( 0,  0, -1), n);   // atras
    patch(Vec3( h, -h,  h), Vec3( 0, 0,-1), Vec3(0, 1, 0), Vec3( 1,  0,  0), n);   // derecha
    patch(Vec3(-h, -h, -h), Vec3( 0, 0, 1), Vec3(0, 1, 0), Vec3(-1,  0,  0), n);   // izquierda
    patch(Vec3(-h,  h,  h), Vec3( 1, 0, 0), Vec3(0, 0,-1), Vec3( 0,  1,  0), n);   // arriba
    patch(Vec3(-h, -h, -h), Vec3( 1, 0, 0), Vec3(0, 0, 1), Vec3( 0, -1,  0), n);   // abajo
}

// Esfera por paralelos y meridianos. La normal de una esfera centrada
// en el origen es el mismo punto normalizado. La textura se envuelve
// con s = longitud y t = latitud (proyeccion equirrectangular).
void Primitives::sphere(int divisiones) {
    int meridianos = divisiones, paralelos = divisiones / 2;
    const float r = 0.5f;
    for (int i = 0; i < paralelos; i++) {
        float lat0 = -PI * 0.5f + PI * i / paralelos;
        float lat1 = -PI * 0.5f + PI * (i + 1) / paralelos;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= meridianos; j++) {
            float lon = 2.0f * PI * j / meridianos;
            float s = (float)j / meridianos;
            float x = cosf(lon), z = -sinf(lon);
            glNormal3f(x * cosf(lat1), sinf(lat1), z * cosf(lat1));
            glTexCoord2f(s, (float)(i + 1) / paralelos);
            glVertex3f(r * x * cosf(lat1), r * sinf(lat1), r * z * cosf(lat1));
            glNormal3f(x * cosf(lat0), sinf(lat0), z * cosf(lat0));
            glTexCoord2f(s, (float)i / paralelos);
            glVertex3f(r * x * cosf(lat0), r * sinf(lat0), r * z * cosf(lat0));
        }
        glEnd();
    }
}

// Tapa circular (abanico de triangulos) a la altura y, mirando a +Y o -Y
static void cap(float y, float radio, bool arriba, int divisiones) {
    glNormal3f(0.0f, arriba ? 1.0f : -1.0f, 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.5f, 0.5f);
    glVertex3f(0.0f, y, 0.0f);
    for (int j = 0; j <= divisiones; j++) {
        float a = 2.0f * PI * j / divisiones * (arriba ? 1.0f : -1.0f);
        float x = cosf(a), z = -sinf(a);
        glTexCoord2f(0.5f + 0.5f * x, 0.5f - 0.5f * z);
        glVertex3f(radio * x, y, radio * z);
    }
    glEnd();
}

void Primitives::cylinder(int divisiones) {
    const float r = 0.5f, h = 0.5f;
    glBegin(GL_QUAD_STRIP);
    for (int j = 0; j <= divisiones; j++) {
        float a = 2.0f * PI * j / divisiones;
        float x = cosf(a), z = -sinf(a);
        float s = (float)j / divisiones;
        glNormal3f(x, 0.0f, z);
        glTexCoord2f(s, 1.0f); glVertex3f(r * x,  h, r * z);
        glTexCoord2f(s, 0.0f); glVertex3f(r * x, -h, r * z);
    }
    glEnd();
    cap( h, r, true, divisiones);
    cap(-h, r, false, divisiones);
}

// Cono: la normal del lado esta inclinada hacia arriba segun la
// pendiente (radio / altura). Se divide en franjas para que la luz
// por vertice no quede concentrada en la punta.
void Primitives::cone(int divisiones) {
    const float r = 0.5f, h = 0.5f;
    int franjas = divisiones / 4;
    if (franjas < 1) franjas = 1;
    const float ny = r / (2.0f * h);
    const float largo = sqrtf(1.0f + ny * ny);
    for (int i = 0; i < franjas; i++) {
        float t0 = (float)i / franjas, t1 = (float)(i + 1) / franjas;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= divisiones; j++) {
            float a = 2.0f * PI * j / divisiones;
            float x = cosf(a), z = -sinf(a);
            float s = (float)j / divisiones;
            glNormal3f(x / largo, ny / largo, z / largo);
            glTexCoord2f(s, t1); glVertex3f(r * (1.0f - t1) * x, -h + 2.0f * h * t1, r * (1.0f - t1) * z);
            glTexCoord2f(s, t0); glVertex3f(r * (1.0f - t0) * x, -h + 2.0f * h * t0, r * (1.0f - t0) * z);
        }
        glEnd();
    }
    cap(-h, r, false, divisiones);
}

// Visto desde +Y cada cuadrilatero va en sentido antihorario, asi la
// cara frontal coincide con la normal (0,1,0).
void Primitives::plane(int divisiones) {
    patch(Vec3(-0.5f, 0.0f, 0.5f), Vec3(1, 0, 0), Vec3(0, 0, -1), Vec3(0, 1, 0), divisiones);
}

// Toro: un circulo pequeno (tubo) que gira alrededor del eje Y
void Primitives::torus(int divisiones) {
    const float R = 0.35f, r = 0.15f;
    int lados = divisiones / 2;
    if (lados < 6) lados = 6;
    for (int i = 0; i < divisiones; i++) {
        float a0 = 2.0f * PI * i / divisiones, a1 = 2.0f * PI * (i + 1) / divisiones;
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= lados; j++) {
            float b = 2.0f * PI * j / lados;
            float cb = cosf(b), sb = sinf(b);
            float t = (float)j / lados;
            glNormal3f(cosf(a1) * cb, sb, -sinf(a1) * cb);
            glTexCoord2f((float)(i + 1) / divisiones, t);
            glVertex3f(cosf(a1) * (R + r * cb), r * sb, -sinf(a1) * (R + r * cb));
            glNormal3f(cosf(a0) * cb, sb, -sinf(a0) * cb);
            glTexCoord2f((float)i / divisiones, t);
            glVertex3f(cosf(a0) * (R + r * cb), r * sb, -sinf(a0) * (R + r * cb));
        }
        glEnd();
    }
}

// Corona circular. La coordenada s recorre el radio (del borde interno
// al externo): cada franja de la imagen se convierte en un anillo.
// Primero el borde externo y luego el interno para que, visto desde
// +Y, la cara frontal coincida con la normal (0,1,0).
void Primitives::disc(int divisiones) {
    int n = divisiones * 2;
    const float rout = 0.5f, rin = 0.5f * DISCO_INTERNO;
    glNormal3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_QUAD_STRIP);
    for (int k = 0; k <= n; k++) {
        float a = k * 2.0f * PI / n;
        float ca = cosf(a), sa = sinf(a);
        glTexCoord2f(1.0f, 0.5f); glVertex3f(rout * ca, 0.0f, rout * sa);
        glTexCoord2f(0.0f, 0.5f); glVertex3f(rin * ca, 0.0f, rin * sa);
    }
    glEnd();
}

void Primitives::draw(int tipo, int divisiones) {
    switch (tipo) {
        case OBJ_CUBO:     cube(divisiones);     break;
        case OBJ_ESFERA:   sphere(divisiones);   break;
        case OBJ_CILINDRO: cylinder(divisiones); break;
        case OBJ_CONO:     cone(divisiones);     break;
        case OBJ_PLANO:    plane(divisiones);    break;
        case OBJ_TORO:     torus(divisiones);    break;
        case OBJ_DISCO:    disc(divisiones);     break;
    }
}

void Primitives::wireBox(const float a[3], const float b[3]) {
    glBegin(GL_LINE_LOOP);
    glVertex3f(a[0], a[1], a[2]); glVertex3f(b[0], a[1], a[2]);
    glVertex3f(b[0], a[1], b[2]); glVertex3f(a[0], a[1], b[2]);
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex3f(a[0], b[1], a[2]); glVertex3f(b[0], b[1], a[2]);
    glVertex3f(b[0], b[1], b[2]); glVertex3f(a[0], b[1], b[2]);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(a[0], a[1], a[2]); glVertex3f(a[0], b[1], a[2]);
    glVertex3f(b[0], a[1], a[2]); glVertex3f(b[0], b[1], a[2]);
    glVertex3f(b[0], a[1], b[2]); glVertex3f(b[0], b[1], b[2]);
    glVertex3f(a[0], a[1], b[2]); glVertex3f(a[0], b[1], b[2]);
    glEnd();
}
