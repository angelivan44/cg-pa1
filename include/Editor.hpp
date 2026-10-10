// =============================================================
//  Editor.hpp - Estado y operaciones del Editor de Escenas 3D
//
//  Reune los modulos (escena, camara, luces, recursos, animacion,
//  fisica, particulas, gizmo, render) y ofrece las operaciones que
//  ejecuta el usuario: crear, borrar, duplicar, importar, guardar,
//  cargar, reproducir... La interfaz (EditorUI) y la ventana
//  (Application) solo llaman a estas operaciones; no saben como
//  esta hecha la escena por dentro.
// =============================================================
#pragma once

#include "Animation.hpp"
#include "Camera.hpp"
#include "Gizmo.hpp"
#include "Lighting.hpp"
#include "Particles.hpp"
#include "Physics.hpp"
#include "RenderOptions.hpp"
#include "Renderer.hpp"
#include "Resources.hpp"
#include "Scene.hpp"

#include <string>

class Editor {
public:
    Scene          escena;
    Camera         camara;
    Lighting       luces;
    Resources      recursos;
    Renderer       renderer;
    RenderOptions  opciones;
    Animator       animador;
    Physics        fisica;
    ParticleSystem particulas;
    Gizmo          gizmo;

    std::string archivo;        // nombre de la escena actual ("" = sin guardar)
    std::string mensaje;        // ultima accion, se muestra en la barra de menu
    int         lucesActivas;

    Editor();
    void init();                                    // con el contexto de OpenGL creado
    void update(float dt);                          // animacion, fisica y particulas
    void render(int x, int y, int ancho, int alto); // dibuja el Viewport

    // --- Objetos ---
    SceneObject* createObject(int tipo, int padre = 0);
    SceneObject* createLight(int tipoLuz);
    SceneObject* importModel(const std::string& archivoObj);
    void deleteSelected();
    void duplicateSelected();
    void focusSelected();
    void assignTexture(const std::string& nombre);
    void pick(int mx, int my);                      // seleccion con el mouse

    // --- Archivo ---
    void newScene();
    bool saveScene(const std::string& nombre);
    bool loadScene(const std::string& nombre);
    void loadExample(int i);

    // --- Reproduccion ---
    void play();
    void pause();
    void stop();
    void restart();
    void togglePlay();
};
