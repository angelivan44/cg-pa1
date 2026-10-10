// =============================================================
//  Scene.cpp - Lista de objetos de la escena y su jerarquia
// =============================================================
#include "Scene.hpp"

#include <cstdio>

Scene::Scene() : seleccion(0), siguienteId(1), cuadro(0) {
    clear();
}

Scene::~Scene() {
    for (size_t i = 0; i < objetos.size(); i++) delete objetos[i];
}

void Scene::clear() {
    for (size_t i = 0; i < objetos.size(); i++) delete objetos[i];
    objetos.clear();
    seleccion = 0;
    siguienteId = 1;
    ambiente[0] = ambiente[1] = ambiente[2] = 0.25f;
    fondo[0] = 0.13f; fondo[1] = 0.14f; fondo[2] = 0.17f;
    cuadricula = true;
    gravedad = 9.8f;
    suelo = true;
    alturaSuelo = 0.0f;
}

// ------------------------------------------------------------------
//  Altas, bajas y copias
// ------------------------------------------------------------------
SceneObject* Scene::create(int tipo) {
    SceneObject* o = new SceneObject();
    o->id = siguienteId++;
    o->tipo = tipo;
    o->nombre = uniqueName(objectTypeName(tipo));
    objetos.push_back(o);
    return o;
}

SceneObject* Scene::insert(const SceneObject& datos) {
    SceneObject* o = new SceneObject(datos);
    if (o->id <= 0 || find(o->id)) o->id = siguienteId;
    if (o->id >= siguienteId) siguienteId = o->id + 1;
    objetos.push_back(o);
    return o;
}

SceneObject* Scene::find(int id) const {
    if (id <= 0) return 0;
    for (size_t i = 0; i < objetos.size(); i++)
        if (objetos[i]->id == id) return objetos[i];
    return 0;
}

void Scene::remove(int id) {
    if (!find(id)) return;
    // Se borran primero los descendientes, de ultimo a primero
    for (size_t i = objetos.size(); i-- > 0;) {
        SceneObject* o = objetos[i];
        if (o->id == id || isDescendant(o->id, id)) {
            if (seleccion == o->id) seleccion = 0;
            delete o;
            objetos.erase(objetos.begin() + i);
        }
    }
}

SceneObject* Scene::duplicateTree(const SceneObject* original, int nuevoPadre) {
    // Los hijos se buscan antes de agregar la copia a la lista
    std::vector<SceneObject*> hijos;
    children(original->id, hijos);

    SceneObject* copia = new SceneObject(*original);
    copia->id = siguienteId++;
    copia->padre = nuevoPadre;
    // "Cubo_02" -> base "Cubo": la copia recibe el siguiente numero libre
    std::string base = original->nombre;
    size_t guion = base.find_last_of('_');
    if (guion != std::string::npos && guion + 1 < base.size() &&
        base.find_first_not_of("0123456789", guion + 1) == std::string::npos)
        base = base.substr(0, guion);
    copia->nombre = uniqueName(base);
    objetos.push_back(copia);

    for (size_t i = 0; i < hijos.size(); i++) duplicateTree(hijos[i], copia->id);
    return copia;
}

SceneObject* Scene::duplicate(int id) {
    SceneObject* original = find(id);
    if (!original) return 0;
    SceneObject* copia = duplicateTree(original, original->padre);
    copia->posicion.x += 1.0f;      // desplazada para que no quede encima
    return copia;
}

std::string Scene::uniqueName(const std::string& base) const {
    for (int n = 1; n < 10000; n++) {
        char nombre[160];
        snprintf(nombre, sizeof(nombre), "%s_%02d", base.c_str(), n);
        bool usado = false;
        for (size_t i = 0; i < objetos.size() && !usado; i++)
            usado = objetos[i]->nombre == nombre;
        if (!usado) return nombre;
    }
    return base;
}

// ------------------------------------------------------------------
//  Jerarquia padre-hijo
// ------------------------------------------------------------------
bool Scene::isDescendant(int id, int ancestro) const {
    SceneObject* o = find(id);
    int pasos = 0;
    while (o && o->padre != 0 && pasos++ < 1000) {
        if (o->padre == ancestro) return true;
        o = find(o->padre);
    }
    return false;
}

void Scene::children(int id, std::vector<SceneObject*>& salida) const {
    for (size_t i = 0; i < objetos.size(); i++)
        if (objetos[i]->padre == id) salida.push_back(objetos[i]);
}

bool Scene::setParent(int hijo, int padre, bool mantenerMundo) {
    SceneObject* o = find(hijo);
    SceneObject* p = find(padre);
    if (!o || hijo == padre) return false;
    if (padre != 0 && !p) return false;
    // Un objeto no puede colgar de uno de sus propios descendientes
    if (p && isDescendant(padre, hijo)) return false;

    if (mantenerMundo) {
        // Para que el objeto no salte de lugar se busca la matriz local
        // que, con el nuevo padre, da la misma matriz de mundo:
        //     M_local = inversa(M_mundo(padre nuevo)) * M_mundo(objeto)
        Mat4 local = o->mundo;
        Mat4 inversaPadre;
        if (p && p->mundo.inverse(inversaPadre)) local = inversaPadre * o->mundo;
        local.decompose(o->posicion, o->rotacion, o->escala);
    }
    o->padre = padre;
    return true;
}

// ------------------------------------------------------------------
//  Transformaciones
// ------------------------------------------------------------------
void Scene::computeWorld(SceneObject* o) {
    if (o->sello == cuadro) return;     // ya calculada en este cuadro
    o->sello = cuadro;
    SceneObject* p = find(o->padre);
    if (p) {
        computeWorld(p);
        o->mundo = p->mundo * o->localMatrix();
    } else {
        o->mundo = o->localMatrix();
    }
}

void Scene::updateTransforms(const Animator& animador) {
    bool animar = animador.estado != Animator::DETENIDO;
    for (size_t i = 0; i < objetos.size(); i++) {
        SceneObject* o = objetos[i];
        // Se parte de la transformacion editada y cada clip activo
        // reemplaza el valor de la propiedad que anima.
        o->posAnim = o->posicion;
        o->rotAnim = o->rotacion;
        o->escAnim = o->escala;
        if (!animar) continue;
        for (size_t k = 0; k < o->animaciones.size(); k++) {
            const AnimClip& c = o->animaciones[k];
            if (!c.activa) continue;
            float v = c.evaluate(animador.tiempo);
            int p = c.propiedad;
            if (p <= ANIM_POS_Z)      o->posAnim[p - ANIM_POS_X] = v;
            else if (p <= ANIM_ROT_Z) o->rotAnim[p - ANIM_ROT_X] = v;
            else if (p <= ANIM_ESC_Z) o->escAnim[p - ANIM_ESC_X] = v;
            else                      o->escAnim = Vec3(v, v, v);
        }
    }
    cuadro++;
    for (size_t i = 0; i < objetos.size(); i++) computeWorld(objetos[i]);
}

// ------------------------------------------------------------------
//  Seleccion: interseccion rayo - caja
//  El rayo se pasa al espacio local del objeto con la inversa de su
//  matriz de mundo; ahi la caja esta alineada a los ejes y se usa el
//  metodo de las "losas" (slabs): se recorta el rayo contra cada par
//  de planos paralelos.
// ------------------------------------------------------------------
int Scene::pick(const Vec3& origen, const Vec3& direccion) const {
    int   mejor = 0;
    float mejorDistancia = 1e30f;
    for (size_t i = 0; i < objetos.size(); i++) {
        const SceneObject* o = objetos[i];
        if (!o->visible) continue;
        Mat4 inversa;
        if (!o->mundo.inverse(inversa)) continue;
        Vec3 ro = inversa.point(origen);
        Vec3 rd = inversa.direction(direccion);
        Vec3 minimo, maximo;
        o->localBounds(minimo, maximo);

        float t0 = 0.0f, t1 = 1e30f;
        bool toca = true;
        for (int k = 0; k < 3 && toca; k++) {
            if (fabsf(rd[k]) < 1e-9f) {
                toca = ro[k] >= minimo[k] && ro[k] <= maximo[k];
            } else {
                float a = (minimo[k] - ro[k]) / rd[k];
                float b = (maximo[k] - ro[k]) / rd[k];
                if (a > b) { float tmp = a; a = b; b = tmp; }
                if (a > t0) t0 = a;
                if (b < t1) t1 = b;
                toca = t0 <= t1;
            }
        }
        if (!toca) continue;
        // La distancia se compara en el mundo, no en el espacio local
        Vec3 punto = o->mundo.point(ro + rd * t0);
        float d = (punto - origen).length();
        if (d < mejorDistancia) {
            mejorDistancia = d;
            mejor = o->id;
        }
    }
    return mejor;
}
