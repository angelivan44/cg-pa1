// =============================================================
//  Scene.hpp - Lista de objetos de la escena y su jerarquia
//
//  La escena ya no esta escrita en el codigo: es una lista dinamica
//  de SceneObject que el usuario crea, borra, duplica y relaciona
//  (padre-hijo) mientras el programa corre.
//
//  Modelado jerarquico: la matriz de mundo de un objeto es
//      M_mundo = M_mundo(padre) * M_local
//  por eso mover, girar o escalar al padre arrastra a sus hijos.
// =============================================================
#pragma once

#include "Animation.hpp"
#include "SceneObject.hpp"

#include <string>
#include <vector>

class Scene {
public:
    std::vector<SceneObject*> objetos;
    int   seleccion;            // id del objeto seleccionado, 0 = ninguno

    // Entorno (se guarda con la escena)
    float ambiente[3];          // luz ambiental global
    float fondo[3];             // color de fondo del Viewport
    bool  cuadricula;           // mostrar la cuadricula del suelo
    float gravedad;             // aceleracion hacia -Y
    bool  suelo;                // la fisica choca con un suelo horizontal
    float alturaSuelo;

    Scene();
    ~Scene();

    void         clear();
    SceneObject* create(int tipo);                    // objeto nuevo con nombre unico
    SceneObject* insert(const SceneObject& datos);    // copia tal cual (al cargar un archivo)
    SceneObject* find(int id) const;
    SceneObject* selected() const { return find(seleccion); }
    void         remove(int id);                      // borra el objeto y sus descendientes
    SceneObject* duplicate(int id);                   // copia el objeto y sus descendientes

    // Jerarquia
    bool setParent(int hijo, int padre, bool mantenerMundo);
    bool isDescendant(int id, int ancestro) const;
    void children(int id, std::vector<SceneObject*>& salida) const;

    std::string uniqueName(const std::string& base) const;

    // Aplica las animaciones y recalcula las matrices de mundo
    void updateTransforms(const Animator& animador);

    // Seleccion con el mouse: id del objeto mas cercano que toca el rayo
    int pick(const Vec3& origen, const Vec3& direccion) const;

private:
    int siguienteId;
    int cuadro;

    void computeWorld(SceneObject* o);
    SceneObject* duplicateTree(const SceneObject* original, int nuevoPadre);

    Scene(const Scene&);                 // no se copia
    Scene& operator=(const Scene&);
};
