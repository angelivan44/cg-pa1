// =============================================================
//  Physics.cpp - Simulacion fisica basica y colisiones
// =============================================================
#include "Physics.hpp"

#include <vector>

static const float PASO_MAXIMO = 1.0f / 120.0f;    // segundos por subpaso

struct Cuerpo {
    SceneObject* objeto;
    float radio;          // esfera para chocar con otros objetos
    float mitadAlto;      // mitad de la altura, para apoyarse en el suelo
};

struct Caja {
    SceneObject* objeto;
    Vec3 minimo, maximo;
};

Physics::Physics() : contactos(0), activa(false) {}

void Physics::begin(Scene& escena) {
    for (size_t i = 0; i < escena.objetos.size(); i++) {
        PhysicsProps& f = escena.objetos[i]->fisica;
        f.posInicio = escena.objetos[i]->posicion;
        f.velocidad = f.velInicial;
        f.enContacto = false;
    }
    activa = true;
}

void Physics::end(Scene& escena) {
    if (!activa) return;
    for (size_t i = 0; i < escena.objetos.size(); i++) {
        SceneObject* o = escena.objetos[i];
        if (o->fisica.modo == FISICA_DINAMICO) o->posicion = o->fisica.posInicio;
        o->fisica.velocidad = Vec3();
        o->fisica.enContacto = false;
    }
    activa = false;
    contactos = 0;
}

void Physics::step(Scene& escena, float dt) {
    if (!activa || dt <= 0.0f) return;

    // Se arma la lista de cuerpos con las matrices del cuadro actual.
    // Solo se simulan objetos sin padre: su posicion local es la del mundo.
    std::vector<Cuerpo> cuerpos;
    std::vector<Caja> cajas;
    for (size_t i = 0; i < escena.objetos.size(); i++) {
        SceneObject* o = escena.objetos[i];
        if (!o->visible || o->fisica.modo == FISICA_NINGUNA) continue;
        Vec3 minimo, maximo;
        o->worldBounds(minimo, maximo);
        if (o->fisica.modo == FISICA_ESTATICO) {
            Caja c = { o, minimo, maximo };
            cajas.push_back(c);
        } else if (o->padre == 0) {
            Vec3 mitad = (maximo - minimo) * 0.5f;
            Cuerpo c;
            c.objeto = o;
            c.mitadAlto = mitad.y;
            c.radio = (o->tipo == OBJ_ESFERA) ? fmaxf(mitad.x, fmaxf(mitad.y, mitad.z))
                                              : (mitad.x + mitad.y + mitad.z) / 3.0f;
            o->fisica.enContacto = false;
            cuerpos.push_back(c);
        }
    }

    contactos = 0;
    int pasos = (int)ceilf(dt / PASO_MAXIMO);
    if (pasos > 20) pasos = 20;
    float h = dt / pasos;

    for (int paso = 0; paso < pasos; paso++) {
        for (size_t i = 0; i < cuerpos.size(); i++) {
            SceneObject* o = cuerpos[i].objeto;
            PhysicsProps& f = o->fisica;
            Vec3& p = o->posicion;
            Vec3& v = f.velocidad;

            // 1. Gravedad e integracion de Euler
            v.y -= escena.gravedad * h;
            p += v * h;

            // 2. Suelo
            float piso = escena.alturaSuelo + cuerpos[i].mitadAlto;
            if (escena.suelo && p.y < piso) {
                p.y = piso;
                if (v.y < 0.0f) {
                    v.y = -v.y * f.rebote;
                    if (v.y < 0.4f) v.y = 0.0f;          // se queda apoyado
                }
                float freno = 1.0f - f.friccion * 6.0f * h;
                if (freno < 0.0f) freno = 0.0f;
                v.x *= freno;
                v.z *= freno;
                f.enContacto = true;
                contactos++;
            }

            // 3. Colisionadores estaticos: esfera contra caja
            for (size_t k = 0; k < cajas.size(); k++) {
                const Caja& c = cajas[k];
                Vec3 cercano;                             // punto de la caja mas cercano al centro
                for (int e = 0; e < 3; e++)
                    cercano[e] = p[e] < c.minimo[e] ? c.minimo[e] : (p[e] > c.maximo[e] ? c.maximo[e] : p[e]);
                Vec3 d = p - cercano;
                float dist = d.length();
                if (dist >= cuerpos[i].radio) continue;

                Vec3 normal;
                float penetracion;
                if (dist > 1e-5f) {
                    normal = d * (1.0f / dist);
                    penetracion = cuerpos[i].radio - dist;
                } else {
                    // El centro quedo dentro de la caja: sale por la cara mas cercana
                    float menor = 1e30f;
                    for (int e = 0; e < 3; e++) {
                        float aMin = p[e] - c.minimo[e], aMax = c.maximo[e] - p[e];
                        if (aMin < menor) { menor = aMin; normal = Vec3(); normal[e] = -1.0f; }
                        if (aMax < menor) { menor = aMax; normal = Vec3(); normal[e] = 1.0f; }
                    }
                    penetracion = menor + cuerpos[i].radio;
                }
                p += normal * penetracion;
                float vn = v.dot(normal);
                if (vn < 0.0f) v -= normal * ((1.0f + f.rebote) * vn);
                f.enContacto = true;
                c.objeto->fisica.enContacto = true;
                contactos++;
            }
        }

        // 4. Dinamico contra dinamico: esfera contra esfera
        for (size_t i = 0; i < cuerpos.size(); i++) {
            for (size_t j = i + 1; j < cuerpos.size(); j++) {
                SceneObject* a = cuerpos[i].objeto;
                SceneObject* b = cuerpos[j].objeto;
                Vec3 d = b->posicion - a->posicion;
                float dist = d.length();
                float suma = cuerpos[i].radio + cuerpos[j].radio;
                if (dist >= suma || dist < 1e-6f) continue;

                Vec3 normal = d * (1.0f / dist);
                float invA = 1.0f / fmaxf(a->fisica.masa, 0.01f);
                float invB = 1.0f / fmaxf(b->fisica.masa, 0.01f);
                // Se separan segun la masa: el mas liviano se mueve mas
                float penetracion = suma - dist;
                a->posicion -= normal * (penetracion * invA / (invA + invB));
                b->posicion += normal * (penetracion * invB / (invA + invB));

                // Impulso: conserva el momento y aplica el rebote
                float vr = (b->fisica.velocidad - a->fisica.velocidad).dot(normal);
                if (vr < 0.0f) {
                    float e = fminf(a->fisica.rebote, b->fisica.rebote);
                    float impulso = -(1.0f + e) * vr / (invA + invB);
                    a->fisica.velocidad -= normal * (impulso * invA);
                    b->fisica.velocidad += normal * (impulso * invB);
                }
                a->fisica.enContacto = b->fisica.enContacto = true;
                contactos++;
            }
        }
    }
}
