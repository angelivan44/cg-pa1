// =============================================================
//  Math.cpp - Vectores y matrices 4x4 del editor
// =============================================================
#include "Math.hpp"

Mat4::Mat4() {
    for (int i = 0; i < 16; i++) m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

Mat4 Mat4::translation(const Vec3& t) {
    Mat4 r;
    r.m[12] = t.x; r.m[13] = t.y; r.m[14] = t.z;
    return r;
}

Mat4 Mat4::scaling(const Vec3& s) {
    Mat4 r;
    r.m[0] = s.x; r.m[5] = s.y; r.m[10] = s.z;
    return r;
}

Mat4 Mat4::rotationX(float grados) {
    float c = cosf(radians(grados)), s = sinf(radians(grados));
    Mat4 r;
    r.m[5] = c;  r.m[9]  = -s;
    r.m[6] = s;  r.m[10] = c;
    return r;
}

Mat4 Mat4::rotationY(float grados) {
    float c = cosf(radians(grados)), s = sinf(radians(grados));
    Mat4 r;
    r.m[0] = c;  r.m[8]  = s;
    r.m[2] = -s; r.m[10] = c;
    return r;
}

Mat4 Mat4::rotationZ(float grados) {
    float c = cosf(radians(grados)), s = sinf(radians(grados));
    Mat4 r;
    r.m[0] = c;  r.m[4] = -s;
    r.m[1] = s;  r.m[5] = c;
    return r;
}

Mat4 Mat4::compose(const Vec3& p, const Vec3& r, const Vec3& e) {
    return translation(p) * rotationZ(r.z) * rotationY(r.y) * rotationX(r.x) * scaling(e);
}

// Con R = Rz(c) * Ry(b) * Rx(a) la primera columna es
// (cos b cos c, cos b sin c, -sin b) y la tercera fila termina en
// (sin a cos b, cos a cos b): de ahi se despejan los tres angulos.
void Mat4::decompose(Vec3& p, Vec3& r, Vec3& e) const {
    p = origin();
    Vec3 ejes[3];
    for (int k = 0; k < 3; k++) {
        ejes[k] = axis(k);
        e[k] = ejes[k].length();
        ejes[k] = ejes[k].normalized();
    }
    float senoB = -ejes[0].z;
    if (senoB > 1.0f) senoB = 1.0f;
    if (senoB < -1.0f) senoB = -1.0f;
    r.y = degrees(asinf(senoB));
    r.x = degrees(atan2f(ejes[1].z, ejes[2].z));
    r.z = degrees(atan2f(ejes[0].y, ejes[0].x));
}

Mat4 Mat4::operator*(const Mat4& b) const {
    Mat4 r;
    for (int c = 0; c < 4; c++)
        for (int f = 0; f < 4; f++) {
            float s = 0.0f;
            for (int k = 0; k < 4; k++) s += m[k * 4 + f] * b.m[c * 4 + k];
            r.m[c * 4 + f] = s;
        }
    return r;
}

Vec3 Mat4::point(const Vec3& p) const {
    return Vec3(m[0] * p.x + m[4] * p.y + m[8]  * p.z + m[12],
                m[1] * p.x + m[5] * p.y + m[9]  * p.z + m[13],
                m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]);
}

Vec3 Mat4::direction(const Vec3& d) const {
    return Vec3(m[0] * d.x + m[4] * d.y + m[8]  * d.z,
                m[1] * d.x + m[5] * d.y + m[9]  * d.z,
                m[2] * d.x + m[6] * d.y + m[10] * d.z);
}

// Inversa general por cofactores (misma cuenta que gluInvertMatrix)
bool Mat4::inverse(Mat4& salida) const {
    float inv[16];
    inv[0]  =  m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15]
             + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15]
             - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8]  =  m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15]
             + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14]
             - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15]
             - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5]  =  m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15]
             + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15]
             - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] =  m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14]
             + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2]  =  m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15]
             + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15]
             - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] =  m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15]
             + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14]
             - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11]
             - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7]  =  m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11]
             + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11]
             - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] =  m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10]
             + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (fabsf(det) < 1e-12f) return false;
    for (int i = 0; i < 16; i++) salida.m[i] = inv[i] / det;
    return true;
}
