// =============================================================
//  Lighting.hpp - Fuentes de luz de la escena
//
//  GL_LIGHT0  luz PUNTUAL en el centro del Sol (w = 1). Ilumina cada
//             planeta desde el Sol, asi la cara que mira al Sol es de
//             dia y la opuesta de noche. Tiene atenuacion cuadratica:
//             los planetas lejanos reciben menos luz.
//  GL_LIGHT1  luz DIRECCIONAL de relleno (w = 0), tenue y azulada.
//             Simula la luz reflejada / de las estrellas para que el
//             lado nocturno no quede totalmente negro.
//  Ambiente   luz ambiental global (glLightModel), igual para todo.
// =============================================================
#pragma once

class Lighting {
public:
    bool  solActiva;        // GL_LIGHT0 encendida
    bool  rellenoActivo;    // GL_LIGHT1 encendida
    int   nivelAmbiente;    // 0 = nada, 1 = bajo, 2 = alto
    bool  atenuacion;       // atenuacion con la distancia en la luz del Sol
    float anguloRelleno;    // direccion horizontal de la luz de relleno (grados)

    Lighting();
    void  reset();
    void  init() const;      // una vez, al iniciar OpenGL
    void  update() const;    // cada cuadro, DESPUES de cargar la matriz de vista
    float ambientValue() const;
};
