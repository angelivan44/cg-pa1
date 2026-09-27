// =============================================================
//  Application.hpp - Ventana, OpenGL, entrada y bucle principal
//
//  Une los modulos: crea la ventana con GLUT, configura OpenGL,
//  recibe teclado y mouse, y en cada cuadro ordena el render:
//  camara -> luces -> escena -> panel (HUD).
//  GLUT solo acepta funciones sueltas como callbacks, por eso hay
//  funciones estaticas que reenvian a la unica instancia.
// =============================================================
#pragma once

#include "Camera.hpp"
#include "Lighting.hpp"
#include "RenderOptions.hpp"
#include "Scene.hpp"

class Application {
public:
    Application();
    int run(int argc, char** argv);

private:
    Scene         escena;
    Camera        camara;
    Lighting      luces;
    RenderOptions opciones;

    int   anchoVentana, altoVentana;
    bool  pausado, mostrarHUD;
    float velocidad;          // multiplicador del tiempo de animacion
    int   ultimoTiempo;       // ms, para medir el tiempo entre cuadros
    int   numCaptura;
    int   botonMouse, mouseX, mouseY;
    bool  modoCapturas;       // --capturas: genera las figuras y sale
    int   pasoCaptura;

    void initOpenGL();
    void render();
    void drawHUD();
    void resetAll();
    void setCameraMode(Camera::Mode modo);
    void followBody(const char* nombre, float distancia, float yaw, float pitch);
    void setupCapture(int paso);
    void printHelp() const;

    // Manejo de eventos
    void onDisplay();
    void onReshape(int ancho, int alto);
    void onTimer();
    void onKey(unsigned char tecla);
    void onSpecialKey(int tecla);
    void onMouse(int boton, int estado, int x, int y);
    void onMotion(int x, int y);

    // Puentes de GLUT -> instancia
    static Application* instancia;
    static void displayCallback();
    static void reshapeCallback(int ancho, int alto);
    static void timerCallback(int);
    static void keyboardCallback(unsigned char tecla, int x, int y);
    static void specialCallback(int tecla, int x, int y);
    static void mouseCallback(int boton, int estado, int x, int y);
    static void motionCallback(int x, int y);
};
