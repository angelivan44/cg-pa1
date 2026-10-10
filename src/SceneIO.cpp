// =============================================================
//  SceneIO.cpp - Guardar y cargar escenas
// =============================================================
#include "SceneIO.hpp"

#include <cstdio>
#include <cstring>

static void writeVec(FILE* f, const char* clave, const Vec3& v) {
    fprintf(f, "  %s %g %g %g\n", clave, v.x, v.y, v.z);
}

bool SceneIO::save(const Scene& escena, const Camera& camara, const std::string& ruta) {
    FILE* f = fopen(ruta.c_str(), "w");
    if (!f) return false;

    fprintf(f, "# Escena del Editor 3D - PA4 Computacion Grafica\n");
    fprintf(f, "escena 1\n");
    fprintf(f, "ambiente %g %g %g\n", escena.ambiente[0], escena.ambiente[1], escena.ambiente[2]);
    fprintf(f, "fondo %g %g %g\n", escena.fondo[0], escena.fondo[1], escena.fondo[2]);
    fprintf(f, "cuadricula %d\n", escena.cuadricula ? 1 : 0);
    fprintf(f, "fisica %g %d %g\n", escena.gravedad, escena.suelo ? 1 : 0, escena.alturaSuelo);
    fprintf(f, "camara %d %g %g %g %g %g %g %g\n", camara.ortogonal ? 1 : 0, camara.yaw, camara.pitch,
            camara.distancia, camara.objetivo.x, camara.objetivo.y, camara.objetivo.z, camara.fov);

    for (size_t i = 0; i < escena.objetos.size(); i++) {
        const SceneObject& o = *escena.objetos[i];
        fprintf(f, "\nobjeto\n");
        fprintf(f, "  id %d\n", o.id);
        fprintf(f, "  nombre %s\n", o.nombre.c_str());
        fprintf(f, "  tipo %s\n", objectTypeKey(o.tipo));
        fprintf(f, "  padre %d\n", o.padre);
        fprintf(f, "  visible %d\n", o.visible ? 1 : 0);
        writeVec(f, "pos", o.posicion);
        writeVec(f, "rot", o.rotacion);
        writeVec(f, "esc", o.escala);

        if (objectHasGeometry(o.tipo)) {
            const Material& m = o.material;
            fprintf(f, "  color %g %g %g\n", m.color[0], m.color[1], m.color[2]);
            fprintf(f, "  material %g %g %g %g %g %g %d\n", m.ambiente, m.difuso, m.especular,
                    m.brillo, m.emision, m.opacidad, m.mezcla ? 1 : 0);
            if (!o.textura.empty()) fprintf(f, "  textura %s\n", o.textura.c_str());
            fprintf(f, "  tex_repetir %g %g\n", o.texRepetir[0], o.texRepetir[1]);
            if (o.fisica.modo != FISICA_NINGUNA)
                fprintf(f, "  cuerpo %d %g %g %g %g %g %g\n", o.fisica.modo, o.fisica.masa, o.fisica.rebote,
                        o.fisica.friccion, o.fisica.velInicial.x, o.fisica.velInicial.y, o.fisica.velInicial.z);
        }
        if (o.tipo == OBJ_MODELO) {
            fprintf(f, "  modelo %s\n", o.modelo.c_str());
            fprintf(f, "  modelo_mtl %d\n", o.usarMaterialModelo ? 1 : 0);
        }
        if (o.tipo == OBJ_LUZ) {
            const LightProps& l = o.luz;
            fprintf(f, "  luz %d %d %g %g %g %g %g %g %g %g %g %g\n", l.tipo, l.encendida ? 1 : 0,
                    l.color[0], l.color[1], l.color[2], l.intensidad, l.ambiente, l.especular,
                    l.atenLineal, l.atenCuadratica, l.anguloFoco, l.exponenteFoco);
        }
        if (o.tipo == OBJ_EMISOR) {
            const EmitterProps& e = o.emisor;
            fprintf(f, "  emisor %g %g %g %g %g %g %g %d\n", e.tasa, e.vida, e.velocidad, e.dispersion,
                    e.gravedad, e.tamInicio, e.tamFin, e.aditivo ? 1 : 0);
            fprintf(f, "  emisor_color %g %g %g %g %g %g %g %g\n",
                    e.colorInicio[0], e.colorInicio[1], e.colorInicio[2], e.colorInicio[3],
                    e.colorFin[0], e.colorFin[1], e.colorFin[2], e.colorFin[3]);
        }
        for (size_t k = 0; k < o.animaciones.size(); k++) {
            const AnimClip& c = o.animaciones[k];
            fprintf(f, "  anim %d %g %g %g %g %d %d %d\n", c.propiedad, c.inicio, c.fin, c.duracion,
                    c.retardo, c.repeticion, c.curva, c.activa ? 1 : 0);
        }
        fprintf(f, "fin\n");
    }
    fclose(f);
    return true;
}

// Texto que sigue a la clave, sin espacios al inicio ni salto de linea
static std::string restOfLine(const char* linea, const char* clave) {
    const char* p = strstr(linea, clave);
    if (!p) return "";
    p += strlen(clave);
    while (*p == ' ' || *p == '\t') p++;
    std::string s = p;
    while (!s.empty() && (s[s.size() - 1] == '\n' || s[s.size() - 1] == '\r' || s[s.size() - 1] == ' '))
        s.erase(s.size() - 1);
    return s;
}

bool SceneIO::load(Scene& escena, Camera& camara, const std::string& ruta) {
    FILE* f = fopen(ruta.c_str(), "r");
    if (!f) return false;

    escena.clear();
    SceneObject actual;
    bool dentro = false;      // leyendo las propiedades de un objeto
    char linea[1024], clave[64];

    while (fgets(linea, sizeof(linea), f)) {
        if (sscanf(linea, "%63s", clave) != 1 || clave[0] == '#') continue;
        const char* resto = strstr(linea, clave) + strlen(clave);
        std::string k = clave;
        int a = 0, b = 0, c = 0, d = 0;

        if (!dentro) {
            if (k == "objeto") {
                actual = SceneObject();
                dentro = true;
            } else if (k == "ambiente") {
                sscanf(resto, "%f %f %f", &escena.ambiente[0], &escena.ambiente[1], &escena.ambiente[2]);
            } else if (k == "fondo") {
                sscanf(resto, "%f %f %f", &escena.fondo[0], &escena.fondo[1], &escena.fondo[2]);
            } else if (k == "cuadricula") {
                if (sscanf(resto, "%d", &a) == 1) escena.cuadricula = a != 0;
            } else if (k == "fisica") {
                if (sscanf(resto, "%f %d %f", &escena.gravedad, &a, &escena.alturaSuelo) >= 2) escena.suelo = a != 0;
            } else if (k == "camara") {
                if (sscanf(resto, "%d %f %f %f %f %f %f %f", &a, &camara.yaw, &camara.pitch, &camara.distancia,
                           &camara.objetivo.x, &camara.objetivo.y, &camara.objetivo.z, &camara.fov) >= 1)
                    camara.ortogonal = a != 0;
            }
            continue;
        }

        if (k == "fin") {
            escena.insert(actual);
            dentro = false;
        } else if (k == "id") {
            sscanf(resto, "%d", &actual.id);
        } else if (k == "nombre") {
            actual.nombre = restOfLine(linea, "nombre");
        } else if (k == "tipo") {
            char tipo[64];
            if (sscanf(resto, "%63s", tipo) == 1) {
                int t = objectTypeFromKey(tipo);
                actual.tipo = t < 0 ? OBJ_VACIO : t;
            }
        } else if (k == "padre") {
            sscanf(resto, "%d", &actual.padre);
        } else if (k == "visible") {
            if (sscanf(resto, "%d", &a) == 1) actual.visible = a != 0;
        } else if (k == "pos") {
            sscanf(resto, "%f %f %f", &actual.posicion.x, &actual.posicion.y, &actual.posicion.z);
        } else if (k == "rot") {
            sscanf(resto, "%f %f %f", &actual.rotacion.x, &actual.rotacion.y, &actual.rotacion.z);
        } else if (k == "esc") {
            sscanf(resto, "%f %f %f", &actual.escala.x, &actual.escala.y, &actual.escala.z);
        } else if (k == "color") {
            Material& m = actual.material;
            sscanf(resto, "%f %f %f", &m.color[0], &m.color[1], &m.color[2]);
        } else if (k == "material") {
            Material& m = actual.material;
            if (sscanf(resto, "%f %f %f %f %f %f %d", &m.ambiente, &m.difuso, &m.especular,
                       &m.brillo, &m.emision, &m.opacidad, &a) == 7) m.mezcla = a != 0;
        } else if (k == "textura") {
            actual.textura = restOfLine(linea, "textura");
        } else if (k == "tex_repetir") {
            sscanf(resto, "%f %f", &actual.texRepetir[0], &actual.texRepetir[1]);
        } else if (k == "modelo") {
            actual.modelo = restOfLine(linea, "modelo");
        } else if (k == "modelo_mtl") {
            if (sscanf(resto, "%d", &a) == 1) actual.usarMaterialModelo = a != 0;
        } else if (k == "cuerpo") {
            PhysicsProps& p = actual.fisica;
            sscanf(resto, "%d %f %f %f %f %f %f", &p.modo, &p.masa, &p.rebote, &p.friccion,
                   &p.velInicial.x, &p.velInicial.y, &p.velInicial.z);
        } else if (k == "luz") {
            LightProps& l = actual.luz;
            if (sscanf(resto, "%d %d %f %f %f %f %f %f %f %f %f %f", &l.tipo, &a, &l.color[0], &l.color[1],
                       &l.color[2], &l.intensidad, &l.ambiente, &l.especular, &l.atenLineal,
                       &l.atenCuadratica, &l.anguloFoco, &l.exponenteFoco) >= 2) l.encendida = a != 0;
        } else if (k == "emisor") {
            EmitterProps& e = actual.emisor;
            if (sscanf(resto, "%f %f %f %f %f %f %f %d", &e.tasa, &e.vida, &e.velocidad, &e.dispersion,
                       &e.gravedad, &e.tamInicio, &e.tamFin, &a) == 8) e.aditivo = a != 0;
        } else if (k == "emisor_color") {
            EmitterProps& e = actual.emisor;
            sscanf(resto, "%f %f %f %f %f %f %f %f",
                   &e.colorInicio[0], &e.colorInicio[1], &e.colorInicio[2], &e.colorInicio[3],
                   &e.colorFin[0], &e.colorFin[1], &e.colorFin[2], &e.colorFin[3]);
        } else if (k == "anim") {
            AnimClip clip;
            if (sscanf(resto, "%d %f %f %f %f %d %d %d", &clip.propiedad, &clip.inicio, &clip.fin,
                       &clip.duracion, &clip.retardo, &b, &c, &d) == 8) {
                clip.repeticion = b;
                clip.curva = c;
                clip.activa = d != 0;
                actual.animaciones.push_back(clip);
            }
        }
    }
    fclose(f);

    // Un padre que no existe en el archivo dejaria al objeto huerfano
    for (size_t i = 0; i < escena.objetos.size(); i++)
        if (!escena.find(escena.objetos[i]->padre)) escena.objetos[i]->padre = 0;
    return true;
}
