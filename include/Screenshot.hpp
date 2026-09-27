// =============================================================
//  Screenshot.hpp - Guarda el buffer trasero en un archivo BMP
// =============================================================
#pragma once

#include <string>

class Screenshot {
public:
    // Lee el viewport actual con glReadPixels y lo guarda como BMP de
    // 24 bits (sin librerias externas). Llamar ANTES de glutSwapBuffers.
    static bool saveBMP(const std::string& ruta);
};
