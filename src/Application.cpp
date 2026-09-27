// =============================================================
//  Application.cpp - Ventana, OpenGL, entrada y bucle principal
// =============================================================
#include "Application.hpp"
#include "GLHeaders.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

Application* Application::instancia = 0;

Application::Application()
    : anchoVentana(1280), altoVentana(720), pausado(false), mostrarHUD(true),
      velocidad(1.0f), ultimoTiempo(0), botonMouse(-1), mouseX(0), mouseY(0) {
    instancia = this;
}

// ------------------------------------------------------------------
//  Inicio
// ------------------------------------------------------------------
int Application::run(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(anchoVentana, altoVentana);
    glutCreateWindow("Sistema Solar - PA3 Computacion Grafica");

    initOpenGL();
    escena.init();
    camara.objetivo = escena.findBody("Tierra");
    setCameraMode(Camera::GENERAL);

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutSpecialFunc(specialCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    ultimoTiempo = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(16, timerCallback, 0);

    printHelp();
    glutMainLoop();
    return 0;
}

void Application::initOpenGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);          // visibilidad por Z-buffer
    glDepthFunc(GL_LEQUAL);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);   // textura * iluminacion
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    luces.init();
}

void Application::setCameraMode(Camera::Mode modo) {
    camara.setMode(modo, escena.bodyRadius(camara.objetivo));
}

void Application::resetAll() {
    opciones.reset();
    luces.reset();
    escena.setTime(0.0f);
    camara.nivelCerca = 0;
    velocidad = 1.0f;
    setCameraMode(Camera::GENERAL);
    camara.applyProjection(anchoVentana, altoVentana);
}

// ------------------------------------------------------------------
//  Render de un cuadro
// ------------------------------------------------------------------
void Application::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float objetivo[3] = { 0.0f, 0.0f, 0.0f };
    if (camara.modo == Camera::SEGUIR) escena.bodyPosition(camara.objetivo, objetivo);
    camara.applyView(objetivo);        // 1. matriz de vista (camara)
    luces.update();                    // 2. luces, con la vista ya cargada
    escena.draw(opciones);     // 3. objetos
    if (mostrarHUD) drawHUD();         // 4. panel 2D encima
}

// ------------------------------------------------------------------
//  HUD: texto 2D encima de la escena con el estado de cada opcion
// ------------------------------------------------------------------
static void texto(float x, float y, const char* s) {
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);
}

static const char* onOff(bool v) { return v ? "ON" : "OFF"; }

void Application::drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, anchoVentana, 0, altoVentana);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    char l[160];
    const char* niveles[] = { "bajo", "medio", "alto" };
    std::vector<std::string> lineas;
    snprintf(l, sizeof(l), "Camara: %s%s%s   (1-4, TAB objetivo, mouse / flechas / +-)",
             camara.modeName(), camara.modo == Camera::SEGUIR ? " -> " : "",
             camara.modo == Camera::SEGUIR ? escena.bodyName(camara.objetivo) : "");
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[L] Luz puntual Sol: %s   [U] Atenuacion: %s", onOff(luces.solActiva), onOff(luces.atenuacion));
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[K] Luz direccional relleno: %s   [ / ] direccion: %.0f grados",
             onOff(luces.rellenoActivo), luces.anguloRelleno);
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[A] Luz ambiental: %.2f", luces.ambientValue());
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[G] Sombreado: %s   [V] Malla: %s (%d div.)",
             opciones.gouraud ? "Gouraud (suave)" : "Flat (plano)",
             niveles[opciones.detalleMalla], opciones.sphereDivisions());
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[M] Material: %s   [T] Texturas: %s",
             opciones.materialBrillante ? "brillante (especular)" : "mate", onOff(opciones.texturas));
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[Z] Depth buffer: %s   [N] Plano cercano: %.1f   [B] Transparencia: %s   [O] Orbitas: %s",
             onOff(opciones.depthBuffer), camara.nearPlane(), onOff(opciones.transparencias), onOff(opciones.orbitas));
    lineas.push_back(l);
    snprintf(l, sizeof(l), "[ESPACIO] %s   [,/.] velocidad x%.2f   [R] reiniciar   [H] ocultar",
             pausado ? "reanudar" : "pausa", velocidad);
    lineas.push_back(l);

    // Fondo semitransparente para que el texto se lea sobre las estrellas
    float alto = 18.0f * lineas.size() + 12.0f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glRectf(6, altoVentana - 6 - alto, 640, altoVentana - 6);
    glDisable(GL_BLEND);

    glColor3f(0.95f, 0.95f, 0.85f);
    for (size_t i = 0; i < lineas.size(); i++)
        texto(14, altoVentana - 24 - 18.0f * i, lineas[i].c_str());

    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Application::printHelp() const {
    printf("\nControles:\n"
           "  1 General  2 Superior  3 Seguir objeto  4 Rasante   TAB siguiente objeto\n"
           "  Mouse: arrastrar = girar, boton derecho = acercar   Flechas = girar   + / - = zoom\n"
           "  L luz puntual del Sol       K luz direccional de relleno   [ ] mover relleno\n"
           "  A luz ambiental (0/bajo/alto) U atenuacion de la luz del Sol\n"
           "  G Flat / Gouraud   V detalle de malla   M material mate / brillante\n"
           "  T texturas   Z depth buffer   N plano cercano (clipping)\n"
           "  B transparencias   O orbitas   ESPACIO pausa   , . velocidad\n"
           "  R reiniciar   H mostrar/ocultar panel   ESC salir\n\n");
}

// ------------------------------------------------------------------
//  Eventos
// ------------------------------------------------------------------
void Application::onDisplay() {
    render();
    glutSwapBuffers();
}

void Application::onReshape(int ancho, int alto) {
    anchoVentana = ancho;
    altoVentana = alto;
    camara.applyProjection(ancho, alto);
}

void Application::onTimer() {
    int ahora = glutGet(GLUT_ELAPSED_TIME);
    float dt = (ahora - ultimoTiempo) / 1000.0f;
    ultimoTiempo = ahora;
    if (!pausado) escena.update(dt * velocidad);
    glutPostRedisplay();
    glutTimerFunc(16, timerCallback, 0);    // ~60 cuadros por segundo
}

void Application::onKey(unsigned char tecla) {
    switch (tecla) {
        case 27: exit(0);
        case '1': setCameraMode(Camera::GENERAL);  break;
        case '2': setCameraMode(Camera::SUPERIOR); break;
        case '3': setCameraMode(Camera::SEGUIR);   break;
        case '4': setCameraMode(Camera::RASANTE);  break;
        case '\t':
            camara.objetivo = (camara.objetivo + 1) % escena.bodyCount();
            setCameraMode(Camera::SEGUIR);
            break;
        case 'l': case 'L': luces.solActiva = !luces.solActiva; break;
        case 'k': case 'K': luces.rellenoActivo = !luces.rellenoActivo; break;
        case 'a': case 'A': luces.nivelAmbiente = (luces.nivelAmbiente + 1) % 3; break;
        case 'u': case 'U': luces.atenuacion = !luces.atenuacion; break;
        case '[': luces.anguloRelleno -= 15.0f; break;
        case ']': luces.anguloRelleno += 15.0f; break;
        case 'g': case 'G': opciones.gouraud = !opciones.gouraud; break;
        case 'v': case 'V': opciones.detalleMalla = (opciones.detalleMalla + 1) % 3; break;
        case 'm': case 'M': opciones.materialBrillante = !opciones.materialBrillante; break;
        case 't': case 'T': opciones.texturas = !opciones.texturas; break;
        case 'z': case 'Z': opciones.depthBuffer = !opciones.depthBuffer; break;
        case 'b': case 'B': opciones.transparencias = !opciones.transparencias; break;
        case 'o': case 'O': opciones.orbitas = !opciones.orbitas; break;
        case 'n': case 'N':
            camara.nivelCerca = (camara.nivelCerca + 1) % 3;
            camara.applyProjection(anchoVentana, altoVentana);
            break;
        case '+': case '=': camara.zoom(0.9f); break;
        case '-': case '_': camara.zoom(1.1f); break;
        case ',': velocidad *= 0.5f; break;
        case '.': velocidad *= 2.0f; break;
        case ' ': pausado = !pausado; break;
        case 'h': case 'H': mostrarHUD = !mostrarHUD; break;
        case 'r': case 'R': resetAll(); break;
    }
}

void Application::onSpecialKey(int tecla) {
    switch (tecla) {
        case GLUT_KEY_LEFT:  camara.rotate(-4.0f, 0.0f); break;
        case GLUT_KEY_RIGHT: camara.rotate( 4.0f, 0.0f); break;
        case GLUT_KEY_UP:    camara.rotate(0.0f,  3.0f); break;
        case GLUT_KEY_DOWN:  camara.rotate(0.0f, -3.0f); break;
    }
}

void Application::onMouse(int boton, int estado, int x, int y) {
    botonMouse = (estado == GLUT_DOWN) ? boton : -1;
    mouseX = x;
    mouseY = y;
    // Rueda del mouse (freeglut la reporta como botones 3 y 4)
    if (estado == GLUT_DOWN && boton == 3) camara.zoom(0.9f);
    if (estado == GLUT_DOWN && boton == 4) camara.zoom(1.1f);
}

void Application::onMotion(int x, int y) {
    int dx = x - mouseX, dy = y - mouseY;
    mouseX = x;
    mouseY = y;
    if (botonMouse == GLUT_LEFT_BUTTON)       camara.rotate(-dx * 0.4f, dy * 0.4f);
    else if (botonMouse == GLUT_RIGHT_BUTTON) camara.zoom(1.0f + dy * 0.01f);
}

// ------------------------------------------------------------------
//  Puentes de GLUT -> instancia
// ------------------------------------------------------------------
void Application::displayCallback()                       { instancia->onDisplay(); }
void Application::reshapeCallback(int w, int h)           { instancia->onReshape(w, h); }
void Application::timerCallback(int)                      { instancia->onTimer(); }
void Application::keyboardCallback(unsigned char t, int, int) { instancia->onKey(t); }
void Application::specialCallback(int t, int, int)        { instancia->onSpecialKey(t); }
void Application::mouseCallback(int b, int e, int x, int y) { instancia->onMouse(b, e, x, y); }
void Application::motionCallback(int x, int y)            { instancia->onMotion(x, y); }
