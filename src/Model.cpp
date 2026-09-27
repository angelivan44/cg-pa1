// =============================================================
//  Model.cpp - Modelo 3D leido de un archivo Wavefront OBJ + MTL
// =============================================================
#include "Model.hpp"
#include "Texture.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

struct MaterialOBJ {
    float  ka[4], kd[4], ks[4];
    float  ns;
    GLuint textura;
};

struct Esquina { int p, t, n; };          // indices (base 0, -1 = no tiene)

struct Tramo {                            // triangulos seguidos con el mismo material
    int material;
    std::vector<Esquina> esquinas;        // de 3 en 3
};

// Carpeta de un archivo, con la barra final ("assets/modelos/")
static std::string carpetaDe(const std::string& ruta) {
    size_t p = ruta.find_last_of("/\\");
    return p == std::string::npos ? "" : ruta.substr(0, p + 1);
}

static MaterialOBJ materialPorDefecto() {
    MaterialOBJ m = {
        { 0.2f, 0.2f, 0.2f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f },
        { 0.2f, 0.2f, 0.2f, 1.0f }, 20.0f, 0
    };
    return m;
}

static void leerMTL(const std::string& ruta, std::vector<MaterialOBJ>& materiales,
                    std::map<std::string, int>& indices) {
    FILE* f = fopen(ruta.c_str(), "r");
    if (!f) {
        printf("  [aviso] no se encontro %s\n", ruta.c_str());
        return;
    }
    char linea[512], texto[400];
    MaterialOBJ* actual = 0;
    while (fgets(linea, sizeof(linea), f)) {
        float a, b, c;
        if (sscanf(linea, "newmtl %399s", texto) == 1) {
            indices[texto] = (int)materiales.size();
            materiales.push_back(materialPorDefecto());
            actual = &materiales.back();
        } else if (!actual) {
            continue;
        } else if (sscanf(linea, "Ka %f %f %f", &a, &b, &c) == 3) {
            actual->ka[0] = a; actual->ka[1] = b; actual->ka[2] = c;
        } else if (sscanf(linea, "Kd %f %f %f", &a, &b, &c) == 3) {
            actual->kd[0] = a; actual->kd[1] = b; actual->kd[2] = c;
        } else if (sscanf(linea, "Ks %f %f %f", &a, &b, &c) == 3) {
            actual->ks[0] = a; actual->ks[1] = b; actual->ks[2] = c;
        } else if (sscanf(linea, "Ns %f", &a) == 1) {
            actual->ns = a > 128.0f ? 128.0f : a;
        } else if (sscanf(linea, "map_Kd %399s", texto) == 1) {
            actual->textura = Texture::load(carpetaDe(ruta) + texto, true);
        }
    }
    fclose(f);
}

// Convierte un indice OBJ (base 1, o negativo = relativo al final)
static int indiceOBJ(int i, int total) {
    if (i > 0) return i - 1;
    if (i < 0) return total + i;
    return -1;
}

static void compilarLista(const std::vector<Tramo>& tramos, const std::vector<MaterialOBJ>& mats,
                          const std::vector<float>& pos, const std::vector<float>& uv,
                          const std::vector<float>& nor, bool conTextura) {
    for (size_t i = 0; i < tramos.size(); i++) {
        const MaterialOBJ& m = mats[tramos[i].material];
        float emision[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   m.ka);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   m.kd);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  m.ks);
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  emision);
        glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, m.ns);

        bool textura = conTextura && m.textura != 0;
        if (textura) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, m.textura);
        } else {
            glDisable(GL_TEXTURE_2D);
        }

        const std::vector<Esquina>& e = tramos[i].esquinas;
        glBegin(GL_TRIANGLES);
        for (size_t k = 0; k + 2 < e.size(); k += 3) {
            // Si el modelo no trae normales se usa la normal de la cara
            float nc[3] = { 0, 0, 0 };
            if (e[k].n < 0) {
                const float* a = &pos[e[k].p * 3];
                const float* b = &pos[e[k + 1].p * 3];
                const float* c = &pos[e[k + 2].p * 3];
                float u[3] = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
                float v[3] = { c[0] - a[0], c[1] - a[1], c[2] - a[2] };
                nc[0] = u[1] * v[2] - u[2] * v[1];
                nc[1] = u[2] * v[0] - u[0] * v[2];
                nc[2] = u[0] * v[1] - u[1] * v[0];
            }
            for (int j = 0; j < 3; j++) {
                const Esquina& q = e[k + j];
                if (q.n >= 0) glNormal3fv(&nor[q.n * 3]); else glNormal3fv(nc);
                if (textura && q.t >= 0) glTexCoord2fv(&uv[q.t * 2]);
                glVertex3fv(&pos[q.p * 3]);
            }
        }
        glEnd();
    }
    glDisable(GL_TEXTURE_2D);
}

Model::Model() : listaConTextura(0), listaSinTextura(0), triangulos(0), cargado(false) {}

bool Model::load(const std::string& ruta) {
    cargado = false;
    triangulos = 0;
    FILE* f = fopen(ruta.c_str(), "r");
    if (!f) {
        printf("  [aviso] no se pudo abrir el modelo %s\n", ruta.c_str());
        return false;
    }

    std::vector<float> pos, uv, nor;
    std::vector<MaterialOBJ> materiales;
    std::map<std::string, int> indiceMaterial;
    materiales.push_back(materialPorDefecto());     // material 0 = por defecto
    std::vector<Tramo> tramos;
    int materialActual = 0;

    char linea[1024], texto[512];
    while (fgets(linea, sizeof(linea), f)) {
        float x, y, z;
        if (linea[0] == 'v' && linea[1] == ' ' && sscanf(linea + 2, "%f %f %f", &x, &y, &z) == 3) {
            pos.push_back(x); pos.push_back(y); pos.push_back(z);
        } else if (linea[0] == 'v' && linea[1] == 't' && sscanf(linea + 3, "%f %f", &x, &y) == 2) {
            uv.push_back(x); uv.push_back(y);
        } else if (linea[0] == 'v' && linea[1] == 'n' && sscanf(linea + 3, "%f %f %f", &x, &y, &z) == 3) {
            nor.push_back(x); nor.push_back(y); nor.push_back(z);
        } else if (sscanf(linea, "mtllib %511s", texto) == 1) {
            leerMTL(carpetaDe(ruta) + texto, materiales, indiceMaterial);
        } else if (sscanf(linea, "usemtl %511s", texto) == 1) {
            std::map<std::string, int>::iterator it = indiceMaterial.find(texto);
            materialActual = it == indiceMaterial.end() ? 0 : it->second;
        } else if (linea[0] == 'f' && linea[1] == ' ') {
            // Lee todas las esquinas de la cara ("v", "v/t", "v//n" o "v/t/n")
            std::vector<Esquina> cara;
            char* p = linea + 2;
            while (*p) {
                while (*p == ' ' || *p == '\t') p++;
                if (*p == '\0' || *p == '\n' || *p == '\r') break;
                Esquina q = { -1, -1, -1 };
                q.p = indiceOBJ((int)strtol(p, &p, 10), (int)pos.size() / 3);
                if (*p == '/') {
                    p++;
                    if (*p != '/') q.t = indiceOBJ((int)strtol(p, &p, 10), (int)uv.size() / 2);
                    if (*p == '/') {
                        p++;
                        q.n = indiceOBJ((int)strtol(p, &p, 10), (int)nor.size() / 3);
                    }
                }
                cara.push_back(q);
                while (*p && *p != ' ' && *p != '\t') p++;
            }
            if (cara.size() < 3) continue;
            if (tramos.empty() || tramos.back().material != materialActual) {
                Tramo t;
                t.material = materialActual;
                tramos.push_back(t);
            }
            // Poligonos de mas de 3 lados se dividen en abanico
            for (size_t k = 1; k + 1 < cara.size(); k++) {
                tramos.back().esquinas.push_back(cara[0]);
                tramos.back().esquinas.push_back(cara[k]);
                tramos.back().esquinas.push_back(cara[k + 1]);
                triangulos++;
            }
        }
    }
    fclose(f);
    if (pos.empty()) return false;

    // Normalizacion: centro de la caja envolvente al origen y radio 1
    float minimo[3] = { 1e30f, 1e30f, 1e30f }, maximo[3] = { -1e30f, -1e30f, -1e30f };
    for (size_t i = 0; i < pos.size(); i += 3) {
        for (int k = 0; k < 3; k++) {
            if (pos[i + k] < minimo[k]) minimo[k] = pos[i + k];
            if (pos[i + k] > maximo[k]) maximo[k] = pos[i + k];
        }
    }
    float centro[3], radio = 0.0f;
    for (int k = 0; k < 3; k++) centro[k] = (minimo[k] + maximo[k]) * 0.5f;
    for (size_t i = 0; i < pos.size(); i += 3) {
        float dx = pos[i] - centro[0], dy = pos[i + 1] - centro[1], dz = pos[i + 2] - centro[2];
        float d = sqrtf(dx * dx + dy * dy + dz * dz);
        if (d > radio) radio = d;
    }
    if (radio <= 0.0f) radio = 1.0f;
    for (size_t i = 0; i < pos.size(); i += 3)
        for (int k = 0; k < 3; k++) pos[i + k] = (pos[i + k] - centro[k]) / radio;

    // Se compila dos veces: con texturas y sin ellas (para la prueba T)
    listaConTextura = glGenLists(2);
    listaSinTextura = listaConTextura + 1;
    glNewList(listaConTextura, GL_COMPILE);
    compilarLista(tramos, materiales, pos, uv, nor, true);
    glEndList();
    glNewList(listaSinTextura, GL_COMPILE);
    compilarLista(tramos, materiales, pos, uv, nor, false);
    glEndList();

    cargado = true;
    size_t barra = ruta.find_last_of("/\\");
    printf("  modelo  %-32s %6d triangulos, %2d materiales\n",
           ruta.substr(barra == std::string::npos ? 0 : barra + 1).c_str(),
           triangulos, (int)materiales.size() - 1);
    return true;
}

void Model::draw(bool conTexturas) const {
    if (!cargado) return;
    // Los modelos tienen piezas delgadas (paneles, antenas) que se ven
    // por ambos lados: se ilumina tambien la cara trasera.
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glCallList(conTexturas ? listaConTextura : listaSinTextura);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}
