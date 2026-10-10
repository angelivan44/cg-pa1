// =============================================================
//  Animation.hpp - Animacion por interpolacion en el tiempo
//
//  Cada objeto puede tener varios clips. Un clip anima UNA propiedad
//  (posicion, rotacion o escala en un eje) desde un valor inicial
//  hasta uno final durante cierta duracion:
//
//      u     = (tiempo - retardo) / duracion        avance 0..1
//      valor = inicio + (fin - inicio) * curva(u)   interpolacion
//
//  El Animator lleva el reloj comun de la escena (reproducir, pausar,
//  detener). Como todo depende del tiempo transcurrido y no del numero
//  de cuadros, la animacion va igual de rapido en cualquier equipo.
// =============================================================
#pragma once

enum AnimProperty {
    ANIM_POS_X = 0, ANIM_POS_Y, ANIM_POS_Z,
    ANIM_ROT_X,     ANIM_ROT_Y, ANIM_ROT_Z,
    ANIM_ESC_X,     ANIM_ESC_Y, ANIM_ESC_Z,
    ANIM_ESC_UNIFORME,
    ANIM_NUM_PROPIEDADES
};

enum AnimLoop  { ANIM_UNA_VEZ = 0, ANIM_REPETIR, ANIM_PING_PONG, ANIM_NUM_REPETICIONES };
enum AnimCurve { ANIM_LINEAL = 0, ANIM_SUAVE, ANIM_ACELERAR, ANIM_FRENAR, ANIM_REBOTE, ANIM_NUM_CURVAS };

struct AnimClip {
    int   propiedad;     // AnimProperty
    float inicio, fin;   // valores entre los que se interpola
    float duracion;      // segundos
    float retardo;       // segundos que espera antes de empezar
    int   repeticion;    // AnimLoop
    int   curva;         // AnimCurve
    bool  activa;

    AnimClip();
    float evaluate(float tiempo) const;

    static const char* propertyName(int propiedad);
    static const char* loopName(int repeticion);
    static const char* curveName(int curva);
};

class Animator {
public:
    enum State { DETENIDO = 0, REPRODUCIENDO, PAUSADO };

    State estado;
    float tiempo;       // segundos de animacion transcurridos
    float velocidad;    // multiplicador del tiempo

    Animator();
    // Avanza el reloj; devuelve los segundos de animacion que pasaron
    float update(float dtReal);
    bool  isPlaying() const { return estado == REPRODUCIENDO; }
};
