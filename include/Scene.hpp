// =============================================================
//  Scene.hpp - Construccion y renderizado del sistema solar
//
//  Escala: los tamanos, distancias y velocidades NO son reales; se
//  eligieron para que todo quepa en pantalla y se note el movimiento,
//  manteniendo el orden de los planetas y sus proporciones aproximadas.
// =============================================================
#pragma once

#include "GLHeaders.hpp"
#include "Model.hpp"
#include "RenderOptions.hpp"

#include <vector>

class Scene {
public:
    Scene();
    ~Scene();

    void init();                       // carga texturas y modelos (con contexto GL creado)
    void update(float segundos);       // avanza la animacion
    void setTime(float segundos);
    void draw(const RenderOptions& opciones) const;

    // Objetos que puede seguir la camara (modo SEGUIR)
    int         bodyCount() const;
    const char* bodyName(int i) const;
    int         findBody(const char* nombre) const;
    void        bodyPosition(int i, float salida[3]) const;
    float       bodyRadius(int i) const;

private:
    // Cada cuerpo orbita alrededor de su padre (o del Sol si padre = -1)
    // en el plano XZ. Si modelo es 0 se dibuja una esfera texturizada.
    struct Body {
        const char* nombre;
        int         padre;
        float       distancia;      // radio de la orbita
        float       velOrbita;      // grados por segundo
        float       fase;           // angulo inicial en la orbita (grados)
        float       radio;          // radio de la esfera o escala del modelo
        float       velRotacion;    // giro sobre su eje (grados por segundo)
        float       ejeInclinado;   // inclinacion del eje (grados)
        float       color[3];       // color del material cuando no hay textura
        const char* archivo;        // textura de la esfera
        Model*      modelo;
        GLuint      textura;
    };

    // Cinturon de asteroides: copias del modelo de Bennu con tamano,
    // posicion y giro distintos (instancias de un mismo modelo)
    struct Asteroid {
        float distancia, fase, altura, escala, velOrbita, eje[3], velGiro;
    };

    std::vector<Body>     cuerpos;
    std::vector<Asteroid> cinturon;
    Model  modeloISS, modeloHubble, modeloCassini, modeloVoyager, modeloBennu;
    GLuint texAnillos, texHalo;
    GLUquadric* cuadrica;
    float  tiempo;                     // segundos de animacion transcurridos

    float orbitAngle(int i) const;
    void  applyOrbit(int i) const;     // transformacion jerarquica de un cuerpo

    void drawSphere(float radio, const RenderOptions& o) const;
    void drawOrbits(const RenderOptions& o) const;
    void drawBody(int i, const RenderOptions& o) const;
    void drawAsteroidBelt(const RenderOptions& o) const;
    void drawRings(const RenderOptions& o) const;
    void drawSunGlow(const RenderOptions& o) const;
};
