// =============================================================
//  EditorUI.hpp - Interfaz grafica del editor (Dear ImGui)
//
//  La interfaz es solo el medio para usar el editor: cada control
//  lee o cambia una propiedad de la escena o llama a una operacion
//  de Editor. Paneles:
//
//    barra de menu    Archivo, Crear, Editar, Ver, Animacion, Ayuda
//    Escena           arbol de objetos (jerarquia padre-hijo)
//    Propiedades      transformacion, material, textura, luz, fisica...
//    Recursos         texturas, modelos y escenas del proyecto
//    Animacion        reproduccion y clips del objeto seleccionado
//    Viewport         la zona central que queda libre para la escena 3D
// =============================================================
#pragma once

#include "Editor.hpp"

class EditorUI {
public:
    bool salir;             // el usuario eligio Archivo > Salir

    EditorUI();
    void init();            // estilo y fuente, una vez
    void draw(Editor& editor);     // entre ImGui::NewFrame() y ImGui::Render()

    // Rectangulo del Viewport en coordenadas de OpenGL (origen abajo a la izquierda)
    void viewport(int& x, int& y, int& ancho, int& alto) const;

    // Atajos de teclado que abren dialogos
    void requestSave(Editor& editor);
    void requestSaveAs(Editor& editor);
    void requestOpen();
    void requestImport();

private:
    enum Dialog { DLG_NINGUNO = 0, DLG_GUARDAR, DLG_ABRIR, DLG_IMPORTAR, DLG_CONTROLES };

    float anchoVentana, altoVentana, altoMenu;
    int   dialogoPendiente;
    char  nombreEscena[128];
    char  rutaModelo[512];
    bool  escalaUniforme;
    int   clipActual;           // clip seleccionado en el panel Animacion
    int   objetoDeClip;         // objeto al que pertenece clipActual

    // Operaciones sobre el arbol que se aplican al terminar de recorrerlo
    int eliminarId, duplicarId, moverHijo, moverPadre;

    void drawMenu(Editor& ed);
    void drawHierarchy(Editor& ed);
    void drawNode(Editor& ed, SceneObject* o);
    void drawCreateItems(Editor& ed, int padre);
    void drawProperties(Editor& ed);
    void drawObjectTab(Editor& ed);
    void drawEnvironmentTab(Editor& ed);
    void drawViewTab(Editor& ed);
    void drawResources(Editor& ed);
    void drawAnimation(Editor& ed);
    void drawToolbar(Editor& ed);
    void drawDialogs(Editor& ed);
};
