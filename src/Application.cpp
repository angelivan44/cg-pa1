// =============================================================
//  Application.cpp - Ventana, entrada y bucle principal
// =============================================================
#include "Application.hpp"
#include "GLHeaders.hpp"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

#include <cstdio>
#include <cstdlib>

static const int UMBRAL_CLIC = 4;      // pixeles: menos que esto es un clic, no un arrastre

Application* Application::instancia = 0;

Application::Application()
    : ultimoTiempo(0), arrastre(NADA), botonMouse(-1), mouseX(0), mouseY(0),
      inicioX(0), inicioY(0), seMovio(false), quitarFoco(false) {
    instancia = this;
}

// ------------------------------------------------------------------
//  Inicio
// ------------------------------------------------------------------
int Application::run(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1440, 860);
    glutCreateWindow("Editor 3D - Proyecto PA4");

    // Dear ImGui sobre GLUT + OpenGL clasico
    ImGui::CreateContext();
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL2_Init();
    ui.init();

    editor.init();

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(ImGui_ImplGLUT_KeyboardUpFunc);
    glutSpecialFunc(specialCallback);
    glutSpecialUpFunc(ImGui_ImplGLUT_SpecialUpFunc);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutPassiveMotionFunc(passiveMotionCallback);
#ifdef __FREEGLUT_EXT_H__
    glutMouseWheelFunc(wheelCallback);
#endif
    ultimoTiempo = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timerCallback, 0);

    printf("Editor 3D listo. Ayuda > Controles muestra los atajos.\n");
    glutMainLoop();
    return 0;
}

// ------------------------------------------------------------------
//  Un cuadro: interfaz -> escena 3D -> interfaz encima
// ------------------------------------------------------------------
void Application::onDisplay() {
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();
    if (quitarFoco) {
        ImGui::SetWindowFocus(0);
        quitarFoco = false;
    }

    // 1. Se arma la interfaz: los cambios del usuario se aplican a la
    //    escena antes de dibujarla, asi se ven en el mismo cuadro.
    ui.draw(editor);

    // 2. La escena 3D, solo en el rectangulo del Viewport
    int x, y, ancho, alto;
    ui.viewport(x, y, ancho, alto);
    editor.render(x, y, ancho, alto);

    // 3. Los paneles de la interfaz, encima
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    glutSwapBuffers();

    if (ui.salir) exit(0);
}

void Application::onReshape(int ancho, int alto) {
    ImGui_ImplGLUT_ReshapeFunc(ancho, alto);
}

void Application::onTimer() {
    int ahora = glutGet(GLUT_ELAPSED_TIME);
    float dt = (ahora - ultimoTiempo) / 1000.0f;
    ultimoTiempo = ahora;
    if (dt > 0.1f) dt = 0.1f;               // tras una pausa larga no se da un salto
    editor.update(dt);
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);    // ~60 cuadros por segundo
}

// ------------------------------------------------------------------
//  Teclado
// ------------------------------------------------------------------
void Application::onKey(unsigned char tecla, int x, int y) {
    ImGui_ImplGLUT_KeyboardFunc(tecla, x, y);
    ImGuiIO& io = ImGui::GetIO();
    // Si se esta escribiendo en un campo o hay un dialogo abierto, la
    // tecla es para la interfaz y no un atajo del editor.
    if (io.WantTextInput || io.WantCaptureKeyboard) return;

    Camera& camara = editor.camara;
    switch (tecla) {
        // Con Ctrl, GLUT entrega el numero de la letra (Ctrl+A = 1 ... Ctrl+Z = 26)
        case 14: editor.newScene(); break;                 // Ctrl+N
        case 15: ui.requestOpen(); break;                  // Ctrl+O
        case 19: ui.requestSave(editor); break;            // Ctrl+S
        case 4:  editor.duplicateSelected(); break;        // Ctrl+D

        case 27: editor.escena.seleccion = 0; break;       // Esc
        case 127: case 'x': case 'X': editor.deleteSelected(); break;
        case 'w': case 'W': editor.gizmo.modo = Gizmo::MOVER; break;
        case 'e': case 'E': editor.gizmo.modo = Gizmo::ROTAR; break;
        case 'r': case 'R': editor.gizmo.modo = Gizmo::ESCALAR; break;
        case 'f': case 'F': editor.focusSelected(); break;
        case 'p': case 'P': camara.ortogonal = !camara.ortogonal; break;
        case 'z': case 'Z': editor.opciones.wireframe = !editor.opciones.wireframe; break;
        case 't': case 'T': editor.opciones.texturas = !editor.opciones.texturas; break;
        case 'l': case 'L': editor.opciones.iluminacion = !editor.opciones.iluminacion; break;
        case 'g': case 'G': editor.escena.cuadricula = !editor.escena.cuadricula; break;
        case '1': camara.setView(0.0f, 0.0f); break;
        case '3': camara.setView(90.0f, 0.0f); break;
        case '7': camara.setView(0.0f, 89.0f); break;
        case '0': camara.reset(); break;
        case '+': case '=': camara.zoom(0.9f); break;
        case '-': case '_': camara.zoom(1.1f); break;
        case ' ': editor.togglePlay(); break;
    }
}

void Application::onSpecialKey(int tecla, int x, int y) {
    ImGui_ImplGLUT_SpecialFunc(tecla, x, y);
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput || io.WantCaptureKeyboard) return;
    switch (tecla) {
        case GLUT_KEY_LEFT:  editor.camara.rotate(-4.0f, 0.0f); break;
        case GLUT_KEY_RIGHT: editor.camara.rotate( 4.0f, 0.0f); break;
        case GLUT_KEY_UP:    editor.camara.rotate(0.0f,  3.0f); break;
        case GLUT_KEY_DOWN:  editor.camara.rotate(0.0f, -3.0f); break;
    }
}

// ------------------------------------------------------------------
//  Mouse
// ------------------------------------------------------------------
void Application::onWheel(int direccion) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) io.AddMouseWheelEvent(0.0f, direccion > 0 ? 1.0f : -1.0f);
    else editor.camara.zoom(direccion > 0 ? 0.9f : 1.1f);
}

void Application::onMouse(int boton, int estado, int x, int y) {
    // Sin glutMouseWheelFunc la rueda llega como botones 3 y 4
    if (boton == 3 || boton == 4) {
        if (estado == GLUT_DOWN) onWheel(boton == 3 ? 1 : -1);
        return;
    }
    ImGui_ImplGLUT_MouseFunc(boton, estado, x, y);
    mouseX = x;
    mouseY = y;

    if (estado == GLUT_DOWN) {
        if (ImGui::GetIO().WantCaptureMouse) return;      // el clic es de un panel
        int mods = glutGetModifiers();
        botonMouse = boton;
        inicioX = x;
        inicioY = y;
        seMovio = false;
        quitarFoco = true;

        if (boton == GLUT_LEFT_BUTTON) {
            SceneObject* sel = editor.escena.selected();
            int eje = sel ? editor.gizmo.hitTest(*sel, editor.camara, x, y) : -1;
            if (eje >= 0) {
                arrastre = GIZMO;
                editor.gizmo.ejeActivo = eje;
            } else if (mods & GLUT_ACTIVE_SHIFT) {
                arrastre = DESPLAZAR;
            } else if (mods & (GLUT_ACTIVE_CTRL | GLUT_ACTIVE_ALT)) {
                arrastre = ZOOM;
            } else {
                arrastre = ORBITAR;
            }
        } else if (boton == GLUT_RIGHT_BUTTON) {
            arrastre = DESPLAZAR;
        } else {
            arrastre = ZOOM;
        }
    } else {
        // Soltar sin haber arrastrado es un clic: selecciona lo que hay debajo
        if (arrastre == ORBITAR && !seMovio) editor.pick(x, y);
        arrastre = NADA;
        botonMouse = -1;
        editor.gizmo.ejeActivo = -1;
    }
}

void Application::onMotion(int x, int y) {
    ImGui_ImplGLUT_MotionFunc(x, y);
    int dx = x - mouseX, dy = y - mouseY;
    mouseX = x;
    mouseY = y;
    if (arrastre == NADA) return;

    if (!seMovio) {
        if (abs(x - inicioX) + abs(y - inicioY) < UMBRAL_CLIC) return;
        seMovio = true;
    }
    Camera& camara = editor.camara;
    switch (arrastre) {
        case ORBITAR:   camara.rotate(-dx * 0.4f, dy * 0.4f); break;
        case DESPLAZAR: camara.pan((float)dx, (float)dy); break;
        case ZOOM:      camara.zoom(1.0f + dy * 0.01f); break;
        case GIZMO: {
            SceneObject* sel = editor.escena.selected();
            if (sel) editor.gizmo.drag(editor.escena, *sel, camara, editor.gizmo.ejeActivo, (float)dx, (float)dy);
            break;
        }
        default: break;
    }
}

// ------------------------------------------------------------------
//  Puentes de GLUT -> instancia
// ------------------------------------------------------------------
void Application::displayCallback()                             { instancia->onDisplay(); }
void Application::reshapeCallback(int w, int h)                 { instancia->onReshape(w, h); }
void Application::timerCallback(int)                            { instancia->onTimer(); }
void Application::keyboardCallback(unsigned char t, int x, int y) { instancia->onKey(t, x, y); }
void Application::specialCallback(int t, int x, int y)          { instancia->onSpecialKey(t, x, y); }
void Application::mouseCallback(int b, int e, int x, int y)     { instancia->onMouse(b, e, x, y); }
void Application::motionCallback(int x, int y)                  { instancia->onMotion(x, y); }
void Application::passiveMotionCallback(int x, int y)           { ImGui_ImplGLUT_MotionFunc(x, y); }
void Application::wheelCallback(int, int direccion, int, int)   { instancia->onWheel(direccion); }
