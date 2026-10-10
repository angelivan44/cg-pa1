// =============================================================
//  Lighting.cpp - Iluminacion de la escena
// =============================================================
#include "Lighting.hpp"
#include "GLHeaders.hpp"

void Lighting::init() const {
    glEnable(GL_NORMALIZE);   // los objetos se escalan: hay que renormalizar N

    // El especular se suma DESPUES de la textura; si no, una textura
    // oscura tambien apagaria el brillo.
    glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
    // Calcula el especular con la posicion real del observador
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

int Lighting::apply(const Scene& escena) const {
    float ambiente[] = { escena.ambiente[0], escena.ambiente[1], escena.ambiente[2], 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambiente);

    int usadas = 0;
    for (size_t i = 0; i < escena.objetos.size() && usadas < MAX_LUCES; i++) {
        const SceneObject* o = escena.objetos[i];
        if (o->tipo != OBJ_LUZ || !o->visible || !o->luz.encendida) continue;
        const LightProps& l = o->luz;
        GLenum luz = GL_LIGHT0 + usadas;

        float a[4], d[4], s[4];
        for (int k = 0; k < 3; k++) {
            a[k] = l.color[k] * l.ambiente;
            d[k] = l.color[k] * l.intensidad;
            s[k] = l.color[k] * l.intensidad * l.especular;
        }
        a[3] = d[3] = s[3] = 1.0f;
        glLightfv(luz, GL_AMBIENT,  a);
        glLightfv(luz, GL_DIFFUSE,  d);
        glLightfv(luz, GL_SPECULAR, s);

        // La posicion se envia con la matriz de vista * mundo cargada:
        // OpenGL la transforma en ese momento, asi la luz queda en el
        // lugar del objeto y sigue a su padre si lo tiene.
        glPushMatrix();
        glMultMatrixf(o->mundo.m);
        if (l.tipo == LUZ_DIRECCIONAL) {
            float haciaLaLuz[] = { 0.0f, 1.0f, 0.0f, 0.0f };     // w = 0: direccional
            glLightfv(luz, GL_POSITION, haciaLaLuz);
        } else {
            float posicion[] = { 0.0f, 0.0f, 0.0f, 1.0f };       // w = 1: puntual
            glLightfv(luz, GL_POSITION, posicion);
        }
        float abajo[] = { 0.0f, -1.0f, 0.0f };
        glLightfv(luz, GL_SPOT_DIRECTION, abajo);
        glPopMatrix();

        bool foco = l.tipo == LUZ_FOCO;
        glLightf(luz, GL_SPOT_CUTOFF,   foco ? l.anguloFoco : 180.0f);
        glLightf(luz, GL_SPOT_EXPONENT, foco ? l.exponenteFoco : 0.0f);
        glLightf(luz, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(luz, GL_LINEAR_ATTENUATION,    l.atenLineal);
        glLightf(luz, GL_QUADRATIC_ATTENUATION, l.atenCuadratica);
        glEnable(luz);
        usadas++;
    }
    for (int i = usadas; i < MAX_LUCES; i++) glDisable(GL_LIGHT0 + i);
    return usadas;
}
