// =============================================================
//  Editor.cpp - Estado y operaciones del Editor de Escenas 3D
// =============================================================
#include "Editor.hpp"
#include "Examples.hpp"
#include "SceneIO.hpp"

#include <sys/stat.h>
#ifdef _WIN32
  #include <direct.h>
#endif

Editor::Editor() : lucesActivas(0) {}

void Editor::init() {
    renderer.init();
    luces.init();
    recursos.init();
    loadExample(0);
    mensaje = "Listo. Crea objetos desde el menú Crear o abre un ejemplo en Archivo.";
}

// ------------------------------------------------------------------
//  Un cuadro
// ------------------------------------------------------------------
void Editor::update(float dt) {
    // La caja envolvente de un modelo OBJ depende del archivo cargado
    for (size_t i = 0; i < escena.objetos.size(); i++) {
        SceneObject* o = escena.objetos[i];
        if (o->tipo != OBJ_MODELO) continue;
        Model* m = recursos.model(o->modelo);
        if (m) { o->limMin = m->minimo; o->limMax = m->maximo; }
    }

    float paso = animador.update(dt);          // segundos de animacion transcurridos
    escena.updateTransforms(animador);
    if (animador.isPlaying()) {
        fisica.step(escena, paso);
        escena.updateTransforms(animador);     // la fisica movio objetos
        particulas.update(escena, paso);
    }
}

void Editor::render(int x, int y, int ancho, int alto) {
    escena.updateTransforms(animador);         // por si la interfaz cambio algo en este cuadro

    glClearColor(escena.fondo[0], escena.fondo[1], escena.fondo[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camara.apply(x, y, ancho, alto);                              // 1. proyeccion y vista
    lucesActivas = luces.apply(escena);                           // 2. luces, con la vista cargada
    if (escena.cuadricula) renderer.drawGrid();
    renderer.drawObjects(escena, recursos, opciones, camara.ojo); // 3. objetos
    particulas.draw(escena, camara, recursos.texParticula);       // 4. particulas
    renderer.drawHelpers(escena, opciones);                       // 5. ayudas del editor
    SceneObject* s = escena.selected();
    if (s) gizmo.draw(*s, camara);
}

// ------------------------------------------------------------------
//  Objetos
// ------------------------------------------------------------------
SceneObject* Editor::createObject(int tipo, int padre) {
    SceneObject* o = escena.create(tipo);
    o->padre = escena.find(padre) ? padre : 0;
    if (o->padre == 0) {
        // Aparece donde mira la camara, apoyado sobre el suelo
        o->posicion = Vec3(camara.objetivo.x, 0.5f, camara.objetivo.z);
        if (tipo == OBJ_PLANO)  { o->posicion.y = 0.0f; o->escala = Vec3(10.0f, 1.0f, 10.0f); }
        if (tipo == OBJ_DISCO)  { o->posicion.y = 0.02f; o->escala = Vec3(3.0f, 1.0f, 3.0f); }
        if (tipo == OBJ_TORO)   o->posicion.y = 0.15f;
        if (tipo == OBJ_MODELO) o->posicion.y = 1.0f;
        if (tipo == OBJ_LUZ)    o->posicion.y = 4.0f;
        if (tipo == OBJ_EMISOR) o->posicion.y = 0.1f;
        if (tipo == OBJ_VACIO)  o->posicion.y = 0.0f;
    }
    escena.seleccion = o->id;
    mensaje = "Creado: " + o->nombre;
    return o;
}

SceneObject* Editor::createLight(int tipoLuz) {
    SceneObject* o = createObject(OBJ_LUZ);
    o->luz.tipo = tipoLuz;
    static const char* BASES[] = { "Luz_puntual", "Luz_direccional", "Foco" };
    o->nombre = escena.uniqueName(BASES[tipoLuz]);
    if (tipoLuz == LUZ_DIRECCIONAL) o->rotacion = Vec3(35.0f, 0.0f, -35.0f);
    mensaje = "Creado: " + o->nombre;
    return o;
}

SceneObject* Editor::importModel(const std::string& archivoObj) {
    if (!recursos.model(archivoObj)) {
        mensaje = "No se pudo cargar el modelo " + archivoObj;
        return 0;
    }
    SceneObject* o = createObject(OBJ_MODELO);
    o->modelo = archivoObj;
    // El nombre del objeto sale del archivo: "naves/iss.obj" -> "iss"
    std::string base = archivoObj;
    size_t barra = base.find_last_of("/\\");
    if (barra != std::string::npos) base = base.substr(barra + 1);
    size_t punto = base.find_last_of('.');
    if (punto != std::string::npos) base = base.substr(0, punto);
    o->nombre = escena.uniqueName(base);
    mensaje = "Modelo importado: " + archivoObj;
    return o;
}

void Editor::deleteSelected() {
    SceneObject* o = escena.selected();
    if (!o) return;
    mensaje = "Eliminado: " + o->nombre;
    escena.remove(o->id);
}

void Editor::duplicateSelected() {
    SceneObject* copia = escena.duplicate(escena.seleccion);
    if (!copia) return;
    escena.seleccion = copia->id;
    mensaje = "Duplicado: " + copia->nombre;
}

void Editor::focusSelected() {
    SceneObject* o = escena.selected();
    if (!o) return;
    Vec3 minimo, maximo;
    o->worldBounds(minimo, maximo);
    camara.focus((minimo + maximo) * 0.5f, (maximo - minimo).length() * 0.5f);
}

void Editor::assignTexture(const std::string& nombre) {
    SceneObject* o = escena.selected();
    if (!o || !objectHasGeometry(o->tipo)) {
        mensaje = "Selecciona un objeto con geometría para asignarle la textura.";
        return;
    }
    o->textura = nombre;
    // Un modelo deja de usar su MTL para que se vea la textura elegida
    if (o->tipo == OBJ_MODELO && !nombre.empty()) o->usarMaterialModelo = false;
    mensaje = nombre.empty() ? "Textura quitada de " + o->nombre
                             : "Textura " + nombre + " asignada a " + o->nombre;
}

void Editor::pick(int mx, int my) {
    Vec3 origen, direccion;
    camara.screenRay(mx, my, origen, direccion);
    escena.seleccion = escena.pick(origen, direccion);
}

// ------------------------------------------------------------------
//  Archivo
// ------------------------------------------------------------------
void Editor::newScene() {
    stop();
    escena.clear();
    camara.reset();
    archivo = "";
    mensaje = "Escena nueva (vacía).";
}

// Un nombre con barras es una ruta completa; si no, va a la carpeta escenas/
static std::string scenePathOf(const std::string& nombre) {
    if (nombre.find('/') != std::string::npos || nombre.find('\\') != std::string::npos) return nombre;
    return scenePath(nombre + ".escena");
}

bool Editor::saveScene(const std::string& nombre) {
    if (nombre.empty()) return false;
    // Se guarda la escena en reposo, no a mitad de una simulacion
    bool reproducia = animador.estado != Animator::DETENIDO;
    if (reproducia) stop();

#ifdef _WIN32
    _mkdir(scenePath("").c_str());
#else
    mkdir(scenePath("").c_str(), 0755);
#endif
    bool ok = SceneIO::save(escena, camara, scenePathOf(nombre));
    if (ok) {
        archivo = nombre;
        recursos.scan();
        mensaje = "Escena guardada: " + scenePathOf(nombre);
    } else {
        mensaje = "No se pudo guardar " + scenePathOf(nombre);
    }
    return ok;
}

bool Editor::loadScene(const std::string& nombre) {
    stop();
    bool ok = SceneIO::load(escena, camara, scenePathOf(nombre));
    if (ok) {
        archivo = nombre;
        mensaje = "Escena cargada: " + nombre;
    } else {
        mensaje = "No se pudo abrir " + scenePathOf(nombre);
    }
    return ok;
}

void Editor::loadExample(int i) {
    stop();
    Examples::build(i, escena, camara);
    archivo = "";
    mensaje = std::string("Ejemplo cargado: ") + Examples::name(i);
}

// ------------------------------------------------------------------
//  Reproduccion
// ------------------------------------------------------------------
void Editor::play() {
    if (animador.estado == Animator::DETENIDO) {
        animador.tiempo = 0.0f;
        fisica.begin(escena);
    }
    animador.estado = Animator::REPRODUCIENDO;
}

void Editor::pause() {
    if (animador.estado == Animator::REPRODUCIENDO) animador.estado = Animator::PAUSADO;
}

// Detener vuelve todo al estado editado: tiempo 0, objetos de la
// fisica en su lugar y sin particulas.
void Editor::stop() {
    fisica.end(escena);
    particulas.clear();
    animador.estado = Animator::DETENIDO;
    animador.tiempo = 0.0f;
}

void Editor::restart() {
    stop();
    play();
}

void Editor::togglePlay() {
    if (animador.isPlaying()) pause(); else play();
}
