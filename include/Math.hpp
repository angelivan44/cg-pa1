// =============================================================
//  Math.hpp - Vectores y matrices 4x4 del editor
//
//  Las matrices se guardan por columnas (igual que OpenGL), asi se
//  pueden pasar directo a glMultMatrixf. El editor las necesita en
//  la CPU para la jerarquia padre-hijo, la seleccion con el mouse
//  (rayo), el gizmo y la fisica.
// =============================================================
#pragma once

#include <cmath>

const float PI = 3.14159265f;

inline float radians(float grados) { return grados * PI / 180.0f; }
inline float degrees(float rad)    { return rad * 180.0f / PI; }

struct Vec3 {
    float x, y, z;

    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float a, float b, float c) : x(a), y(b), z(c) {}

    float&       operator[](int i)       { return (&x)[i]; }
    const float& operator[](int i) const { return (&x)[i]; }

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float k) const       { return Vec3(x * k, y * k, z * k); }
    Vec3& operator+=(const Vec3& o)     { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(const Vec3& o)     { x -= o.x; y -= o.y; z -= o.z; return *this; }

    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3  cross(const Vec3& o) const {
        return Vec3(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
    }
    float length() const { return sqrtf(x * x + y * y + z * z); }
    Vec3  normalized() const {
        float l = length();
        return l > 1e-8f ? Vec3(x / l, y / l, z / l) : Vec3(0.0f, 0.0f, 0.0f);
    }
};

struct Mat4 {
    float m[16];          // m[columna * 4 + fila]

    Mat4();               // identidad

    static Mat4 translation(const Vec3& t);
    static Mat4 scaling(const Vec3& s);
    static Mat4 rotationX(float grados);
    static Mat4 rotationY(float grados);
    static Mat4 rotationZ(float grados);

    // Transformacion local de un objeto:  M = T * Rz * Ry * Rx * S
    // (primero se escala, luego se gira en X, Y, Z y al final se traslada)
    static Mat4 compose(const Vec3& posicion, const Vec3& rotacion, const Vec3& escala);
    // Operacion inversa de compose (supone que no hay deformacion por corte)
    void decompose(Vec3& posicion, Vec3& rotacion, Vec3& escala) const;

    Mat4 operator*(const Mat4& b) const;
    Vec3 point(const Vec3& p) const;       // transforma un punto (w = 1)
    Vec3 direction(const Vec3& d) const;   // transforma una direccion (w = 0)
    Vec3 axis(int k) const { return Vec3(m[k * 4], m[k * 4 + 1], m[k * 4 + 2]); }
    Vec3 origin() const    { return Vec3(m[12], m[13], m[14]); }
    bool inverse(Mat4& salida) const;      // false si la matriz no es invertible
};
