// =============================================================
//  Examples.cpp - Escenas de ejemplo
// =============================================================
#include "Examples.hpp"

#include <cstdio>

// ------------------------------------------------------------------
//  Atajos para armar escenas
// ------------------------------------------------------------------
static SceneObject* add(Scene& e, int tipo, const char* nombre, SceneObject* padre,
                        const Vec3& pos, const Vec3& esc) {
    SceneObject* o = e.create(tipo);
    o->nombre = nombre;
    o->padre = padre ? padre->id : 0;
    o->posicion = pos;
    o->escala = esc;
    return o;
}

static void color(SceneObject* o, float r, float g, float b) {
    o->material.color[0] = r;
    o->material.color[1] = g;
    o->material.color[2] = b;
}

static void texture(SceneObject* o, const char* archivo, float u = 1.0f, float v = 1.0f) {
    o->textura = archivo;
    o->texRepetir[0] = u;
    o->texRepetir[1] = v;
    color(o, 1.0f, 1.0f, 1.0f);          // blanco: la textura se ve sin tenir
}

static void animate(SceneObject* o, int propiedad, float inicio, float fin, float duracion,
                    int repeticion = ANIM_REPETIR, int curva = ANIM_LINEAL) {
    AnimClip c;
    c.propiedad = propiedad;
    c.inicio = inicio;
    c.fin = fin;
    c.duracion = duracion;
    c.repeticion = repeticion;
    c.curva = curva;
    o->animaciones.push_back(c);
}

static SceneObject* light(Scene& e, const char* nombre, int tipo, const Vec3& pos,
                          float r, float g, float b, float intensidad) {
    SceneObject* o = add(e, OBJ_LUZ, nombre, 0, pos, Vec3(1, 1, 1));
    o->luz.tipo = tipo;
    o->luz.color[0] = r; o->luz.color[1] = g; o->luz.color[2] = b;
    o->luz.intensidad = intensidad;
    return o;
}

// ------------------------------------------------------------------
//  1. Escena basica
// ------------------------------------------------------------------
static void buildBasic(Scene& e, Camera& cam) {
    SceneObject* piso = add(e, OBJ_PLANO, "Piso", 0, Vec3(0, 0, 0), Vec3(16, 1, 16));
    texture(piso, "(tablero)", 4, 4);
    piso->material.especular = 0.1f;

    SceneObject* cubo = add(e, OBJ_CUBO, "Cubo_01", 0, Vec3(0, 0.75f, 0), Vec3(1.5f, 1.5f, 1.5f));
    texture(cubo, "(ladrillos)");
    cubo->rotacion.y = 30.0f;

    SceneObject* esfera = add(e, OBJ_ESFERA, "Esfera_01", 0, Vec3(-2.6f, 0.8f, 0.6f), Vec3(1.6f, 1.6f, 1.6f));
    esfera->material = Material::preset(3, esfera->material);       // metal pulido

    SceneObject* cilindro = add(e, OBJ_CILINDRO, "Cilindro_01", 0, Vec3(2.4f, 1.0f, -0.4f), Vec3(1.1f, 2.0f, 1.1f));
    texture(cilindro, "(metal)");
    cilindro->material.especular = 0.9f;
    cilindro->material.brillo = 80.0f;

    SceneObject* cono = add(e, OBJ_CONO, "Cono_01", 0, Vec3(1.2f, 0.6f, 2.6f), Vec3(1.2f, 1.2f, 1.2f));
    color(cono, 0.9f, 0.35f, 0.15f);

    SceneObject* toro = add(e, OBJ_TORO, "Toro_01", 0, Vec3(-1.4f, 0.35f, 2.8f), Vec3(1.8f, 1.8f, 1.8f));
    texture(toro, "(madera)", 4, 1);

    SceneObject* luz = light(e, "Luz_puntual", LUZ_PUNTUAL, Vec3(-3.0f, 5.0f, 4.0f), 1.0f, 0.96f, 0.88f, 1.0f);
    luz->luz.atenCuadratica = 0.004f;
    SceneObject* sol = light(e, "Luz_direccional", LUZ_DIRECCIONAL, Vec3(5.0f, 6.0f, -3.0f), 0.55f, 0.65f, 0.9f, 0.5f);
    sol->rotacion = Vec3(35.0f, 0.0f, -40.0f);

    cam.reset();
}

// ------------------------------------------------------------------
//  2. Sistema solar (la escena del PA3, ahora editable)
//
//  Cada planeta cuelga de un "pivote" vacio en el centro. Girar el
//  pivote sobre Y hace que el planeta describa su orbita:
//      Orbita (gira)  ->  Sistema (a la distancia de la orbita)
//                              ->  Planeta (inclinado, gira sobre su eje)
//                              ->  Orbita de la luna  ->  Luna
// ------------------------------------------------------------------
static SceneObject* orbit(Scene& e, const char* nombre, SceneObject* padre, float fase, float periodo) {
    SceneObject* o = add(e, OBJ_VACIO, nombre, padre, Vec3(0, 0, 0), Vec3(1, 1, 1));
    float vuelta = periodo < 0.0f ? -360.0f : 360.0f;
    animate(o, ANIM_ROT_Y, fase, fase + vuelta, fabsf(periodo));
    return o;
}

static SceneObject* planet(Scene& e, const char* nombre, SceneObject* padre, const Vec3& pos, float radio,
                           const char* archivo, float inclinacion, float periodoGiro) {
    SceneObject* o = add(e, OBJ_ESFERA, nombre, padre, pos, Vec3(radio * 2, radio * 2, radio * 2));
    texture(o, archivo);
    o->rotacion.z = inclinacion;
    o->material.especular = 0.0f;
    o->material.ambiente = 1.0f;
    o->material.difuso = 1.0f;
    if (periodoGiro > 0.0f) animate(o, ANIM_ROT_Y, 0.0f, 360.0f, periodoGiro);
    return o;
}

static void buildSolarSystem(Scene& e, Camera& cam) {
    e.fondo[0] = e.fondo[1] = e.fondo[2] = 0.0f;
    e.ambiente[0] = e.ambiente[1] = e.ambiente[2] = 0.06f;
    e.cuadricula = false;

    SceneObject* sol = planet(e, "Sol", 0, Vec3(0, 0, 0), 2.2f, "2k_sun.jpg", 7.3f, 90.0f);
    sol->material.emision = 1.0f;         // emisivo: se ve sin que lo ilumine nadie
    sol->material.ambiente = 0.0f;
    sol->material.difuso = 0.0f;

    // Luz puntual en el centro del Sol: la cara de cada planeta que
    // mira al Sol queda de dia y la opuesta de noche.
    SceneObject* luzSol = light(e, "Luz_Sol", LUZ_PUNTUAL, Vec3(0, 0, 0), 1.0f, 0.95f, 0.85f, 1.0f);
    luzSol->padre = sol->id;
    luzSol->luz.atenCuadratica = 0.00035f;
    SceneObject* relleno = light(e, "Luz_relleno", LUZ_DIRECCIONAL, Vec3(-6.0f, 8.0f, 0.0f), 0.5f, 0.6f, 1.0f, 0.3f);
    relleno->rotacion = Vec3(30.0f, 0.0f, 55.0f);
    relleno->luz.especular = 0.0f;

    // Tierra con la Luna y la Estacion Espacial
    SceneObject* oTierra = orbit(e, "Orbita_Tierra", 0, 100.0f, 30.0f);
    SceneObject* sTierra = add(e, OBJ_VACIO, "Sistema_Tierra", oTierra, Vec3(6.5f, 0, 0), Vec3(1, 1, 1));
    planet(e, "Tierra", sTierra, Vec3(0, 0, 0), 1.3f, "2k_earth_daymap.jpg", 23.4f, 9.0f);
    SceneObject* oLuna = orbit(e, "Orbita_Luna", sTierra, 60.0f, 12.0f);
    planet(e, "Luna", oLuna, Vec3(2.4f, 0, 0), 0.38f, "2k_moon.jpg", 6.7f, 0.0f);
    SceneObject* oISS = orbit(e, "Orbita_ISS", sTierra, 200.0f, -6.0f);
    SceneObject* iss = add(e, OBJ_MODELO, "ISS", oISS, Vec3(1.8f, 0, 0), Vec3(0.45f, 0.45f, 0.45f));
    iss->modelo = "iss.obj";
    animate(iss, ANIM_ROT_Y, 0.0f, 360.0f, 18.0f);

    SceneObject* oMarte = orbit(e, "Orbita_Marte", 0, 170.0f, 40.0f);
    planet(e, "Marte", oMarte, Vec3(10.5f, 0, 0), 0.95f, "2k_mars.jpg", 25.2f, 10.0f);

    SceneObject* oJupiter = orbit(e, "Orbita_Jupiter", 0, 55.0f, 60.0f);
    planet(e, "Jupiter", oJupiter, Vec3(14.5f, 0, 0), 2.1f, "2k_jupiter.jpg", 3.1f, 7.0f);

    // Saturno: los anillos son un disco con textura RGBA y mezcla alfa
    SceneObject* oSaturno = orbit(e, "Orbita_Saturno", 0, 140.0f, 90.0f);
    SceneObject* sSaturno = add(e, OBJ_VACIO, "Sistema_Saturno", oSaturno, Vec3(20.0f, 0, 0), Vec3(1, 1, 1));
    sSaturno->rotacion.z = 26.7f;
    planet(e, "Saturno", sSaturno, Vec3(0, 0, 0), 1.7f, "2k_saturn.jpg", 0.0f, 8.0f);
    SceneObject* anillos = add(e, OBJ_DISCO, "Anillos", sSaturno, Vec3(0, 0, 0), Vec3(8.0f, 1.0f, 8.0f));
    texture(anillos, "2k_saturn_ring_alpha.png");
    anillos->material.mezcla = true;
    anillos->material.especular = 0.1f;

    SceneObject* oVoyager = orbit(e, "Orbita_Voyager", 0, 215.0f, 120.0f);
    SceneObject* voyager = add(e, OBJ_MODELO, "Voyager", oVoyager, Vec3(26.0f, 0, 0), Vec3(1.5f, 1.5f, 1.5f));
    voyager->modelo = "voyager.obj";
    animate(voyager, ANIM_ROT_Y, 0.0f, 360.0f, 24.0f);

    cam.reset();
    cam.yaw = 30.0f;
    cam.pitch = 32.0f;
    cam.distancia = 40.0f;
    cam.objetivo = Vec3(0, 0, 0);
}

// ------------------------------------------------------------------
//  3. Brazo robotico (cinematica directa)
//
//  Las articulaciones son objetos vacios; la geometria de cada pieza
//  cuelga de su articulacion. Girar una articulacion mueve todo lo que
//  esta mas abajo en la cadena.
// ------------------------------------------------------------------
static void buildRobotArm(Scene& e, Camera& cam) {
    SceneObject* piso = add(e, OBJ_PLANO, "Piso", 0, Vec3(0, 0, 0), Vec3(14, 1, 14));
    texture(piso, "(tablero)", 4, 4);
    piso->material.especular = 0.1f;

    SceneObject* base = add(e, OBJ_CILINDRO, "Base", 0, Vec3(0, 0.2f, 0), Vec3(2.0f, 0.4f, 2.0f));
    texture(base, "(metal)");

    // Hombro: gira todo el brazo sobre el eje vertical
    SceneObject* hombro = add(e, OBJ_VACIO, "Hombro", 0, Vec3(0, 0.4f, 0), Vec3(1, 1, 1));
    animate(hombro, ANIM_ROT_Y, -70.0f, 70.0f, 4.0f, ANIM_PING_PONG, ANIM_SUAVE);
    SceneObject* torre = add(e, OBJ_CILINDRO, "Torre", hombro, Vec3(0, 0.5f, 0), Vec3(0.7f, 1.0f, 0.7f));
    color(torre, 0.95f, 0.65f, 0.1f);

    // Brazo: se inclina desde la punta de la torre
    SceneObject* codo1 = add(e, OBJ_VACIO, "Articulacion_brazo", hombro, Vec3(0, 1.0f, 0), Vec3(1, 1, 1));
    animate(codo1, ANIM_ROT_Z, -15.0f, -55.0f, 3.0f, ANIM_PING_PONG, ANIM_SUAVE);
    SceneObject* brazo = add(e, OBJ_CUBO, "Brazo", codo1, Vec3(0, 1.0f, 0), Vec3(0.4f, 2.0f, 0.4f));
    color(brazo, 0.95f, 0.65f, 0.1f);
    SceneObject* rotula1 = add(e, OBJ_ESFERA, "Rotula_brazo", codo1, Vec3(0, 0, 0), Vec3(0.6f, 0.6f, 0.6f));
    rotula1->material = Material::preset(3, rotula1->material);

    // Antebrazo
    SceneObject* codo2 = add(e, OBJ_VACIO, "Articulacion_antebrazo", codo1, Vec3(0, 2.0f, 0), Vec3(1, 1, 1));
    animate(codo2, ANIM_ROT_Z, -20.0f, -100.0f, 2.0f, ANIM_PING_PONG, ANIM_SUAVE);
    SceneObject* antebrazo = add(e, OBJ_CUBO, "Antebrazo", codo2, Vec3(0, 0.75f, 0), Vec3(0.3f, 1.5f, 0.3f));
    color(antebrazo, 0.95f, 0.65f, 0.1f);
    SceneObject* rotula2 = add(e, OBJ_ESFERA, "Rotula_antebrazo", codo2, Vec3(0, 0, 0), Vec3(0.5f, 0.5f, 0.5f));
    rotula2->material = Material::preset(3, rotula2->material);

    // Pinza: dos dedos que se abren y cierran
    SceneObject* muneca = add(e, OBJ_VACIO, "Muneca", codo2, Vec3(0, 1.5f, 0), Vec3(1, 1, 1));
    animate(muneca, ANIM_ROT_Y, 0.0f, 360.0f, 5.0f);
    SceneObject* dedo1 = add(e, OBJ_CUBO, "Dedo_01", muneca, Vec3(0.18f, 0.25f, 0), Vec3(0.08f, 0.5f, 0.2f));
    SceneObject* dedo2 = add(e, OBJ_CUBO, "Dedo_02", muneca, Vec3(-0.18f, 0.25f, 0), Vec3(0.08f, 0.5f, 0.2f));
    color(dedo1, 0.2f, 0.2f, 0.22f);
    color(dedo2, 0.2f, 0.2f, 0.22f);
    animate(dedo1, ANIM_POS_X, 0.08f, 0.24f, 1.0f, ANIM_PING_PONG, ANIM_SUAVE);
    animate(dedo2, ANIM_POS_X, -0.08f, -0.24f, 1.0f, ANIM_PING_PONG, ANIM_SUAVE);

    SceneObject* luz = light(e, "Luz_puntual", LUZ_PUNTUAL, Vec3(3.0f, 6.0f, 4.0f), 1.0f, 0.97f, 0.9f, 1.0f);
    luz->luz.atenCuadratica = 0.003f;
    SceneObject* foco = light(e, "Foco", LUZ_FOCO, Vec3(-3.0f, 6.0f, 2.0f), 0.6f, 0.75f, 1.0f, 1.0f);
    foco->rotacion = Vec3(15.0f, 0.0f, -25.0f);
    foco->luz.anguloFoco = 35.0f;

    cam.reset();
    cam.objetivo = Vec3(0, 2.0f, 0);
    cam.distancia = 11.0f;
}

// ------------------------------------------------------------------
//  4. Fisica y particulas
// ------------------------------------------------------------------
static void buildPhysics(Scene& e, Camera& cam) {
    SceneObject* piso = add(e, OBJ_PLANO, "Piso", 0, Vec3(0, 0, 0), Vec3(18, 1, 18));
    texture(piso, "(tablero)", 6, 6);
    piso->material.especular = 0.1f;

    static const float COLORES[5][3] = {
        { 0.9f, 0.25f, 0.2f }, { 0.95f, 0.75f, 0.15f }, { 0.25f, 0.7f, 0.35f },
        { 0.2f, 0.5f, 0.95f }, { 0.75f, 0.35f, 0.9f }
    };
    for (int i = 0; i < 5; i++) {
        char nombre[32];
        snprintf(nombre, sizeof(nombre), "Pelota_%02d", i + 1);
        float d = 0.7f + 0.15f * i;
        SceneObject* p = add(e, OBJ_ESFERA, nombre, 0, Vec3(-3.0f + 1.5f * i, 3.0f + 1.2f * i, 0.2f * i), Vec3(d, d, d));
        color(p, COLORES[i][0], COLORES[i][1], COLORES[i][2]);
        p->material.especular = 0.7f;
        p->material.brillo = 60.0f;
        p->fisica.modo = FISICA_DINAMICO;
        p->fisica.rebote = 0.55f + 0.08f * i;
        p->fisica.masa = d * d * d;
        p->fisica.velInicial = Vec3(0.6f - 0.3f * i, 0.0f, 0.0f);
    }

    // Una pelota grande que cruza y choca con las demas
    SceneObject* bola = add(e, OBJ_ESFERA, "Bola_grande", 0, Vec3(-7.0f, 1.0f, 0.3f), Vec3(1.6f, 1.6f, 1.6f));
    texture(bola, "(madera)");
    bola->fisica.modo = FISICA_DINAMICO;
    bola->fisica.masa = 6.0f;
    bola->fisica.rebote = 0.4f;
    bola->fisica.velInicial = Vec3(5.0f, 3.0f, 0.0f);

    // Obstaculos estaticos
    SceneObject* caja = add(e, OBJ_CUBO, "Caja_estatica", 0, Vec3(2.5f, 0.6f, 0.4f), Vec3(1.6f, 1.2f, 1.6f));
    texture(caja, "(ladrillos)");
    caja->fisica.modo = FISICA_ESTATICO;
    SceneObject* muro = add(e, OBJ_CUBO, "Muro", 0, Vec3(6.5f, 1.0f, 0), Vec3(0.4f, 2.0f, 6.0f));
    texture(muro, "(ladrillos)", 3, 1);
    muro->fisica.modo = FISICA_ESTATICO;

    // Fuente de particulas
    SceneObject* fuego = add(e, OBJ_EMISOR, "Fuego", 0, Vec3(-4.5f, 0.1f, -3.0f), Vec3(1, 1, 1));
    fuego->emisor.gravedad = -1.0f;      // el fuego sube
    fuego->emisor.velocidad = 1.6f;
    fuego->emisor.tasa = 120.0f;
    SceneObject* fuente = add(e, OBJ_EMISOR, "Fuente", 0, Vec3(3.5f, 0.1f, -3.5f), Vec3(1, 1, 1));
    EmitterProps& f = fuente->emisor;
    f.tasa = 200.0f; f.vida = 2.0f; f.velocidad = 7.0f; f.dispersion = 12.0f; f.gravedad = 9.0f;
    f.tamInicio = 0.12f; f.tamFin = 0.2f;
    const float azul[4] = { 0.5f, 0.8f, 1.0f, 0.9f }, blanco[4] = { 0.9f, 0.95f, 1.0f, 0.0f };
    for (int k = 0; k < 4; k++) { f.colorInicio[k] = azul[k]; f.colorFin[k] = blanco[k]; }

    SceneObject* luz = light(e, "Luz_puntual", LUZ_PUNTUAL, Vec3(-2.0f, 7.0f, 5.0f), 1.0f, 0.97f, 0.9f, 1.0f);
    luz->luz.atenCuadratica = 0.002f;
    SceneObject* sol = light(e, "Luz_direccional", LUZ_DIRECCIONAL, Vec3(6.0f, 7.0f, -4.0f), 0.6f, 0.7f, 0.9f, 0.45f);
    sol->rotacion = Vec3(30.0f, 0.0f, -35.0f);

    cam.reset();
    cam.objetivo = Vec3(0, 1.5f, 0);
    cam.distancia = 17.0f;
    cam.pitch = 18.0f;
    cam.yaw = 10.0f;
}

// ------------------------------------------------------------------
static const char* NOMBRES[] = {
    "Escena básica", "Sistema solar (PA3)", "Brazo robótico (jerarquía)", "Física y partículas"
};

int Examples::count() { return 4; }

const char* Examples::name(int i) { return NOMBRES[i]; }

void Examples::build(int i, Scene& escena, Camera& camara) {
    escena.clear();
    switch (i) {
        case 1:  buildSolarSystem(escena, camara); break;
        case 2:  buildRobotArm(escena, camara);    break;
        case 3:  buildPhysics(escena, camara);     break;
        default: buildBasic(escena, camara);       break;
    }
}
