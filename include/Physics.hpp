// =============================================================
//  Physics.hpp - Simulacion fisica basica y colisiones
//                (funcionalidad avanzada)
//
//  Un objeto puede ser:
//    dinamico  -> le afecta la gravedad, cae, rebota y choca
//    estatico  -> no se mueve, pero los dinamicos chocan con el
//
//  Integracion de Euler en pasos pequenos:
//      v += g * dt ;  p += v * dt
//  Colisiones:
//    dinamico - suelo      plano horizontal (altura configurable)
//    dinamico - dinamico   esfera contra esfera, con intercambio de
//                          impulso segun las masas
//    dinamico - estatico   esfera contra la caja envolvente (AABB)
//  En cada choque la velocidad normal se invierte y se multiplica por
//  el coeficiente de rebote.
//
//  La simulacion corre mientras la animacion se reproduce; al detener
//  los objetos vuelven a la posicion que tenian.
// =============================================================
#pragma once

#include "Scene.hpp"

class Physics {
public:
    int contactos;              // choques detectados en el ultimo paso

    Physics();
    void begin(Scene& escena);  // guarda las posiciones iniciales
    void end(Scene& escena);    // las restaura
    void step(Scene& escena, float dt);
    bool isActive() const { return activa; }

private:
    bool activa;
};
