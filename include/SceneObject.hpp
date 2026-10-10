// =============================================================
//  SceneObject.hpp - Un objeto de la escena y sus propiedades
//
//  Todo lo que aparece en el panel Escena es un SceneObject: las
//  primitivas, los modelos OBJ, las luces, los emisores de particulas
//  y los objetos vacios (que solo sirven para agrupar). Cada uno
//  guarda su propia transformacion, material, textura y animaciones.
// =============================================================
#pragma once

#include "Animation.hpp"
#include "Material.hpp"
#include "Math.hpp"

#include <string>
#include <vector>

enum ObjectType {
    OBJ_VACIO = 0,    // sin geometria: articulacion o grupo de la jerarquia
    OBJ_CUBO,
    OBJ_ESFERA,
    OBJ_CILINDRO,
    OBJ_CONO,
    OBJ_PLANO,
    OBJ_TORO,
    OBJ_DISCO,        // corona circular (anillo plano)
    OBJ_MODELO,       // modelo importado de un archivo OBJ
    OBJ_LUZ,
    OBJ_EMISOR,       // emisor de particulas
    OBJ_NUM_TIPOS
};

const char* objectTypeName(int tipo);                // nombre para mostrar
const char* objectTypeKey(int tipo);                 // palabra usada en el archivo
int         objectTypeFromKey(const char* clave);    // -1 si no existe
bool        objectHasGeometry(int tipo);             // usa material y textura

// --- Fuente de luz -------------------------------------------------
enum LightType { LUZ_PUNTUAL = 0, LUZ_DIRECCIONAL, LUZ_FOCO };

struct LightProps {
    int   tipo;
    bool  encendida;
    float color[3];
    float intensidad;        // multiplica la componente difusa
    float ambiente;          // aporte ambiental de esta luz
    float especular;         // multiplica la componente especular
    float atenLineal;        // atenuacion con la distancia (puntual y foco)
    float atenCuadratica;
    float anguloFoco;        // semiangulo del cono (grados)
    float exponenteFoco;     // concentracion de la luz hacia el centro

    LightProps();
};

// --- Emisor de particulas ------------------------------------------
struct EmitterProps {
    float tasa;              // particulas por segundo
    float vida;              // segundos que dura cada particula
    float velocidad;         // rapidez inicial
    float dispersion;        // apertura del cono de salida (grados)
    float gravedad;          // aceleracion hacia abajo
    float tamInicio, tamFin;
    float colorInicio[4], colorFin[4];
    bool  aditivo;           // mezcla aditiva (fuego, chispas) o alfa (humo)

    EmitterProps();
};

// --- Fisica basica -------------------------------------------------
enum PhysicsMode { FISICA_NINGUNA = 0, FISICA_DINAMICO, FISICA_ESTATICO };

struct PhysicsProps {
    int   modo;
    float masa;
    float rebote;            // 0 = no rebota, 1 = rebota sin perder energia
    float friccion;
    Vec3  velInicial;

    // Estado de la simulacion (no se guarda)
    Vec3  velocidad;
    Vec3  posInicio;
    bool  enContacto;

    PhysicsProps();
};

struct SceneObject {
    int         id;          // identificador unico dentro de la escena
    std::string nombre;
    int         tipo;        // ObjectType
    int         padre;       // id del padre, 0 = sin padre
    bool        visible;

    // Transformacion local (respecto al padre)
    Vec3 posicion;
    Vec3 rotacion;           // grados, se aplica X, luego Y, luego Z
    Vec3 escala;

    // Apariencia
    Material    material;
    std::string textura;         // nombre en la biblioteca de recursos ("" = ninguna)
    float       texRepetir[2];   // repeticiones de la textura en U y V
    std::string modelo;          // archivo OBJ (solo OBJ_MODELO)
    bool        usarMaterialModelo;   // usar los materiales y texturas del MTL

    LightProps   luz;
    EmitterProps emisor;
    PhysicsProps fisica;
    std::vector<AnimClip> animaciones;

    // --- Calculado en cada cuadro (no se guarda) ---
    Vec3 posAnim, rotAnim, escAnim;   // transformacion con las animaciones aplicadas
    Mat4 mundo;                       // matriz local -> mundo (incluye a los padres)
    int  sello;                       // cuadro en que se calculo "mundo"
    Vec3 limMin, limMax;              // caja envolvente del modelo OBJ

    SceneObject();

    Mat4 localMatrix() const { return Mat4::compose(posAnim, rotAnim, escAnim); }
    // Caja envolvente en coordenadas locales (seleccion y colisiones)
    void localBounds(Vec3& minimo, Vec3& maximo) const;
    // Caja envolvente alineada a los ejes del mundo
    void worldBounds(Vec3& minimo, Vec3& maximo) const;
};
