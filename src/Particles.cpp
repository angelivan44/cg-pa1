// =============================================================
//  Particles.cpp - Sistema de particulas (funcionalidad avanzada)
// =============================================================
#include "Particles.hpp"

#include <cstdlib>

static const size_t MAX_POR_EMISOR = 4000;

static float azar() {                 // 0..1
    return (float)rand() / (float)RAND_MAX;
}

void ParticleSystem::clear() {
    estados.clear();
}

int ParticleSystem::count() const {
    int total = 0;
    for (std::map<int, EmitterState>::const_iterator it = estados.begin(); it != estados.end(); ++it)
        total += (int)it->second.particulas.size();
    return total;
}

void ParticleSystem::update(const Scene& escena, float dt) {
    // Se olvidan los emisores que ya no existen en la escena
    for (std::map<int, EmitterState>::iterator it = estados.begin(); it != estados.end();) {
        SceneObject* o = escena.find(it->first);
        if (!o || o->tipo != OBJ_EMISOR) estados.erase(it++);
        else ++it;
    }

    for (size_t i = 0; i < escena.objetos.size(); i++) {
        const SceneObject* o = escena.objetos[i];
        if (o->tipo != OBJ_EMISOR) continue;
        const EmitterProps& e = o->emisor;
        EmitterState& estado = estados[o->id];
        std::vector<Particle>& ps = estado.particulas;

        // 1. Avanzar las particulas vivas y quitar las que murieron
        for (size_t k = 0; k < ps.size();) {
            Particle& p = ps[k];
            p.edad += dt;
            if (p.edad >= p.vida) {
                ps[k] = ps.back();
                ps.pop_back();
                continue;
            }
            p.velocidad.y -= e.gravedad * dt;
            p.posicion += p.velocidad * dt;
            k++;
        }

        // 2. Emitir las nuevas. La tasa es por segundo, asi que en un
        //    cuadro salen tasa * dt (la fraccion sobrante se acumula).
        if (!o->visible) continue;
        estado.pendiente += e.tasa * dt;
        Vec3 origen = o->mundo.origin();
        Vec3 ejeX = o->mundo.axis(0).normalized();
        Vec3 ejeY = o->mundo.axis(1).normalized();
        Vec3 ejeZ = o->mundo.axis(2).normalized();
        while (estado.pendiente >= 1.0f) {
            estado.pendiente -= 1.0f;
            if (ps.size() >= MAX_POR_EMISOR) continue;
            // Direccion al azar dentro del cono de dispersion
            float apertura = radians(e.dispersion) * azar();
            float giro = 2.0f * PI * azar();
            Vec3 dir = ejeY * cosf(apertura) +
                       (ejeX * cosf(giro) + ejeZ * sinf(giro)) * sinf(apertura);
            Particle p;
            p.posicion = origen;
            p.velocidad = dir * (e.velocidad * (0.75f + 0.5f * azar()));
            p.edad = 0.0f;
            p.vida = e.vida * (0.7f + 0.6f * azar());
            ps.push_back(p);
        }
    }
}

void ParticleSystem::draw(const Scene& escena, const Camera& camara, GLuint textura) const {
    if (estados.empty()) return;
    Vec3 der = camara.right(), arr = camara.up();

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);            // se prueba la profundidad pero no se escribe
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textura);

    for (std::map<int, EmitterState>::const_iterator it = estados.begin(); it != estados.end(); ++it) {
        const SceneObject* o = escena.find(it->first);
        if (!o) continue;
        const EmitterProps& e = o->emisor;
        // Aditiva: suma luz sin tapar lo de atras (fuego). Alfa: humo, polvo.
        glBlendFunc(GL_SRC_ALPHA, e.aditivo ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);

        const std::vector<Particle>& ps = it->second.particulas;
        glBegin(GL_QUADS);
        for (size_t k = 0; k < ps.size(); k++) {
            const Particle& p = ps[k];
            float u = p.edad / p.vida;
            float t = (e.tamInicio + (e.tamFin - e.tamInicio) * u) * 0.5f;
            glColor4f(e.colorInicio[0] + (e.colorFin[0] - e.colorInicio[0]) * u,
                      e.colorInicio[1] + (e.colorFin[1] - e.colorInicio[1]) * u,
                      e.colorInicio[2] + (e.colorFin[2] - e.colorInicio[2]) * u,
                      e.colorInicio[3] + (e.colorFin[3] - e.colorInicio[3]) * u);
            Vec3 a = p.posicion - der * t - arr * t;
            Vec3 b = p.posicion + der * t - arr * t;
            Vec3 c = p.posicion + der * t + arr * t;
            Vec3 d = p.posicion - der * t + arr * t;
            glTexCoord2f(0, 0); glVertex3f(a.x, a.y, a.z);
            glTexCoord2f(1, 0); glVertex3f(b.x, b.y, b.z);
            glTexCoord2f(1, 1); glVertex3f(c.x, c.y, c.z);
            glTexCoord2f(0, 1); glVertex3f(d.x, d.y, d.z);
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
