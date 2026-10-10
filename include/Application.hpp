// =============================================================
//  Application.hpp - Ventana, entrada y bucle principal
//
//  Crea la ventana con GLUT, inicia Dear ImGui y reparte los eventos:
//  primero los recibe la interfaz y, si no los usa (el cursor esta
//  sobre el Viewport), se convierten en acciones del editor: orbitar
//  la camara, seleccionar, arrastrar el gizmo o un atajo de teclado.
//  GLUT solo acepta funciones sueltas como callbacks, por eso hay
//  funciones estaticas que reenvian a la unica instancia.
// =============================================================
#pragma once

#include "Editor.hpp"
#include "EditorUI.hpp"

class Application {
public:
    Application();
    int run(int argc, char** argv);

private:
    enum Drag { NADA = 0, ORBITAR, DESPLAZAR, ZOOM, GIZMO };

    Editor   editor;
    EditorUI ui;

    int  ultimoTiempo;         // ms, para medir el tiempo entre cuadros
    Drag arrastre;             // que hace el mouse mientras se arrastra
    int  botonMouse;
    int  mouseX, mouseY;       // ultima posicion
    int  inicioX, inicioY;     // donde se presiono el boton
    bool seMovio;              // el arrastre supero el umbral (no es un clic)
    bool quitarFoco;           // clic en el Viewport: soltar los campos de texto

    // Manejo de eventos
    void onDisplay();
    void onReshape(int ancho, int alto);
    void onTimer();
    void onKey(unsigned char tecla, int x, int y);
    void onSpecialKey(int tecla, int x, int y);
    void onMouse(int boton, int estado, int x, int y);
    void onMotion(int x, int y);
    void onWheel(int direccion);

    // Puentes de GLUT -> instancia
    static Application* instancia;
    static void displayCallback();
    static void reshapeCallback(int ancho, int alto);
    static void timerCallback(int);
    static void keyboardCallback(unsigned char tecla, int x, int y);
    static void specialCallback(int tecla, int x, int y);
    static void mouseCallback(int boton, int estado, int x, int y);
    static void motionCallback(int x, int y);
    static void passiveMotionCallback(int x, int y);
    static void wheelCallback(int rueda, int direccion, int x, int y);
};
