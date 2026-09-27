// =============================================================
//  GLHeaders.hpp - Cabeceras de OpenGL/GLUT segun el sistema
//
//  Todos los modulos incluyen este archivo en vez de incluir
//  OpenGL directamente, asi las diferencias entre macOS, Linux
//  y Windows quedan en un solo lugar.
// =============================================================
#pragma once

#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
  #include <GLUT/glut.h>
#else
  #ifdef _WIN32
    #include <windows.h>
  #endif
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/freeglut.h>
#endif

#include <string>

// --- Compatibilidad entre sistemas -------------------------------
// Windows solo expone cabeceras de OpenGL 1.1; estas constantes son
// de OpenGL 1.2 y el driver si las soporta. Se definen con su valor
// oficial para que el programa compile igual en los tres sistemas.
#ifndef GL_CLAMP_TO_EDGE
  #define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_LIGHT_MODEL_COLOR_CONTROL
  #define GL_LIGHT_MODEL_COLOR_CONTROL 0x81F8
#endif
#ifndef GL_SEPARATE_SPECULAR_COLOR
  #define GL_SEPARATE_SPECULAR_COLOR 0x81FA
#endif

// --- Ruta de los recursos ------------------------------------------
// CMake define ASSETS_DIR con la ruta absoluta a assets/, asi el
// programa encuentra las texturas aunque se ejecute desde build/.
#ifndef ASSETS_DIR
  #define ASSETS_DIR "assets/"
#endif

inline std::string assetPath(const std::string& relativa) {
    return std::string(ASSETS_DIR) + relativa;
}
