// =============================================================
//  Particles.hpp - Sistema de particulas (funcionalidad avanzada)
//
//  Cada objeto de tipo Emisor lanza particulas desde su posicion, en
//  un cono alrededor de su eje +Y. Cada particula tiene posicion,
//  velocidad y edad; en cada paso:
//
//      velocidad.y -= gravedad * dt
//      posicion    += velocidad * dt        (integracion de Euler)
//
//  y al superar su tiempo de vida desaparece. El tamano y el color se
//  interpolan entre el valor inicial y el final segun la edad.
//  Se dibujan como cuadrados que siempre miran a la camara (billboards).
// =============================================================
#pragma once

#include "Camera.hpp"
#include "GLHeaders.hpp"
#include "Scene.hpp"

#include <map>
#include <vector>

class ParticleSystem {
public:
    void update(const Scene& escena, float dt);
    void draw(const Scene& escena, const Camera& camara, GLuint textura) const;
    void clear();
    int  count() const;

private:
    struct Particle {
        Vec3  posicion, velocidad;
        float edad, vida;
    };
    struct EmitterState {
        std::vector<Particle> particulas;
        float pendiente;      // fraccion de particula acumulada entre cuadros
        EmitterState() : pendiente(0.0f) {}
    };
    std::map<int, EmitterState> estados;     // por id de objeto
};
