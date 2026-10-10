// =============================================================
//  Resources.cpp - Biblioteca de recursos del proyecto
// =============================================================
#include "Resources.hpp"
#include "Texture.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <dirent.h>

static const char* PROCEDURALES[] = { "(tablero)", "(ladrillos)", "(madera)", "(metal)" };
static const int   NUM_PROCEDURALES = 4;

static std::string extensionDe(const std::string& archivo) {
    size_t punto = archivo.find_last_of('.');
    if (punto == std::string::npos) return "";
    std::string ext = archivo.substr(punto + 1);
    for (size_t i = 0; i < ext.size(); i++) ext[i] = (char)tolower((unsigned char)ext[i]);
    return ext;
}

// Archivos de una carpeta cuya extension esta en la lista "ext1 ext2 ..."
static std::vector<std::string> listar(const std::string& carpeta, const std::string& extensiones) {
    std::vector<std::string> salida;
    DIR* dir = opendir(carpeta.c_str());
    if (!dir) return salida;
    while (struct dirent* entrada = readdir(dir)) {
        std::string nombre = entrada->d_name;
        if (nombre.empty() || nombre[0] == '.') continue;
        std::string ext = extensionDe(nombre);
        if (ext.empty()) continue;
        if ((" " + extensiones + " ").find(" " + ext + " ") != std::string::npos)
            salida.push_back(nombre);
    }
    closedir(dir);
    std::sort(salida.begin(), salida.end());
    return salida;
}

// Un nombre con barras es una ruta completa; si no, esta en la carpeta del proyecto
static std::string rutaDe(const std::string& carpeta, const std::string& nombre) {
    if (nombre.find('/') != std::string::npos || nombre.find('\\') != std::string::npos)
        return nombre;
    return assetPath(carpeta + nombre);
}

Resources::Resources() : texParticula(0) {}

Resources::~Resources() {
    for (std::map<std::string, Model*>::iterator it = cacheModelos.begin(); it != cacheModelos.end(); ++it)
        delete it->second;
}

void Resources::init() {
    texParticula = Texture::createGlow(64);
    for (int i = 0; i < NUM_PROCEDURALES; i++) {
        TextureEntry e = { PROCEDURALES[i], Texture::createProcedural(i, 256), true };
        texturas.push_back(e);
    }
    scan();
}

void Resources::scan() {
    std::vector<std::string> archivos = listar(assetPath("textures"), "jpg jpeg png bmp tga");
    for (size_t i = 0; i < archivos.size(); i++) {
        bool existe = false;
        for (size_t k = 0; k < texturas.size() && !existe; k++)
            existe = texturas[k].nombre == archivos[i];
        if (!existe) {
            TextureEntry e = { archivos[i], 0, false };
            texturas.push_back(e);
        }
    }
    modelos = listar(assetPath("models"), "obj");
    escenas = listar(scenePath(""), "escena");
    for (size_t i = 0; i < escenas.size(); i++)
        escenas[i] = escenas[i].substr(0, escenas[i].find_last_of('.'));
}

GLuint Resources::texture(const std::string& nombre) {
    if (nombre.empty()) return 0;
    for (size_t i = 0; i < texturas.size(); i++) {
        if (texturas[i].nombre != nombre) continue;
        if (!texturas[i].intentada) {
            texturas[i].intentada = true;
            texturas[i].id = Texture::load(rutaDe("textures/", nombre), true);
        }
        return texturas[i].id;
    }
    // No estaba en la carpeta (ruta externa o archivo de una escena cargada)
    TextureEntry e = { nombre, Texture::load(rutaDe("textures/", nombre), true), true };
    texturas.push_back(e);
    return e.id;
}

Model* Resources::model(const std::string& nombre) {
    if (nombre.empty()) return 0;
    std::map<std::string, Model*>::iterator it = cacheModelos.find(nombre);
    if (it == cacheModelos.end()) {
        Model* m = new Model();
        m->load(rutaDe("models/", nombre));
        it = cacheModelos.insert(std::make_pair(nombre, m)).first;
    }
    return it->second->isLoaded() ? it->second : 0;
}
