#!/usr/bin/env python3
# =============================================================
#  Conversor GLB (glTF 2.0 binario) -> OBJ + MTL
#
#  Los modelos de NASA 3D Resources se descargan en formato .glb.
#  El programa en C++ lee OBJ (texto plano, facil de explicar), asi
#  que este script hace la conversion una sola vez:
#    - recorre la escena aplicando las transformaciones de cada nodo
#    - escribe posiciones (v), normales (vn), coordenadas UV (vt)
#    - un grupo "usemtl" por primitiva, con su material en el .mtl
#
#    - extrae las texturas embebidas (PNG/JPG) y las enlaza con map_Kd
#
#  Uso:  python3 tools/glb_a_obj.py entrada.glb salida.obj
#  Casi todos los modelos de NASA vienen comprimidos con Draco
#  (KHR_draco_mesh_compression); para esos se necesita DracoPy:
#    python3 -m venv tools/.venv
#    tools/.venv/bin/pip install DracoPy numpy
#    tools/.venv/bin/python tools/glb_a_obj.py entrada.glb salida.obj
# =============================================================

import json
import math
import os
import struct
import sys

TAMANO_COMPONENTE = {5120: 1, 5121: 1, 5122: 2, 5123: 2, 5125: 4, 5126: 4}
FORMATO_COMPONENTE = {5120: 'b', 5121: 'B', 5122: 'h', 5123: 'H', 5125: 'I', 5126: 'f'}
COMPONENTES_TIPO = {'SCALAR': 1, 'VEC2': 2, 'VEC3': 3, 'VEC4': 4, 'MAT4': 16}


def leer_glb(ruta):
    datos = open(ruta, 'rb').read()
    magia, version, _ = struct.unpack('<4sII', datos[:12])
    if magia != b'glTF' or version != 2:
        raise ValueError('no es un GLB version 2')
    pos, gltf, binario = 12, None, b''
    while pos < len(datos):
        largo, tipo = struct.unpack('<I4s', datos[pos:pos + 8])
        trozo = datos[pos + 8:pos + 8 + largo]
        if tipo == b'JSON':
            gltf = json.loads(trozo)
        elif tipo == b'BIN\x00':
            binario = trozo
        pos += 8 + largo
    return gltf, binario


def datos_vista(gltf, binario, indice):
    vista = gltf['bufferViews'][indice]
    inicio = vista.get('byteOffset', 0)
    return binario[inicio:inicio + vista['byteLength']]


def leer_primitiva(gltf, binario, prim):
    """Devuelve (posiciones, normales, uvs, indices) de una primitiva."""
    atr = prim['attributes']
    draco = prim.get('extensions', {}).get('KHR_draco_mesh_compression')
    if draco:
        import DracoPy   # solo se necesita para modelos comprimidos
        malla = DracoPy.decode(datos_vista(gltf, binario, draco['bufferView']))
        pos = [tuple(map(float, p)) for p in malla.points]
        nor = [tuple(map(float, n)) for n in malla.normals] if 'NORMAL' in atr and len(malla.normals) else None
        uv = [tuple(map(float, t)) for t in malla.tex_coord] if 'TEXCOORD_0' in atr and len(malla.tex_coord) else None
        idx = [int(i) for cara in malla.faces for i in cara]
        return pos, nor, uv, idx
    pos = leer_accesor(gltf, binario, atr['POSITION'])
    nor = leer_accesor(gltf, binario, atr['NORMAL']) if 'NORMAL' in atr else None
    uv = leer_accesor(gltf, binario, atr['TEXCOORD_0']) if 'TEXCOORD_0' in atr else None
    idx = leer_accesor(gltf, binario, prim['indices']) if 'indices' in prim else list(range(len(pos)))
    return pos, nor, uv, idx


def exportar_textura(gltf, binario, id_textura, carpeta, base):
    """Guarda la imagen PNG/JPG de una textura y devuelve su nombre."""
    tex = gltf['textures'][id_textura]
    id_imagen = tex.get('source')     # la version PNG/JPG (la WebP va en extensions)
    if id_imagen is None:
        return None
    imagen = gltf['images'][id_imagen]
    ext = {'image/png': 'png', 'image/jpeg': 'jpg'}.get(imagen.get('mimeType'))
    if ext is None or 'bufferView' not in imagen:
        return None
    nombre = '%s_tex%d.%s' % (base, id_imagen, ext)
    ruta = os.path.join(carpeta, nombre)
    if not os.path.exists(ruta):
        open(ruta, 'wb').write(datos_vista(gltf, binario, imagen['bufferView']))
    return nombre


def leer_accesor(gltf, binario, indice):
    acc = gltf['accessors'][indice]
    vista = gltf['bufferViews'][acc['bufferView']]
    n = COMPONENTES_TIPO[acc['type']]
    tam = TAMANO_COMPONENTE[acc['componentType']]
    paso = vista.get('byteStride', n * tam)
    inicio = vista.get('byteOffset', 0) + acc.get('byteOffset', 0)
    fmt = '<' + FORMATO_COMPONENTE[acc['componentType']] * n
    salida = []
    for i in range(acc['count']):
        valores = struct.unpack_from(fmt, binario, inicio + i * paso)
        salida.append(valores if n > 1 else valores[0])
    return salida


# --- Matrices 4x4 en orden de columnas (como glTF y OpenGL) ------
def identidad():
    return [1.0 if i % 5 == 0 else 0.0 for i in range(16)]


def multiplicar(a, b):
    r = [0.0] * 16
    for c in range(4):
        for f in range(4):
            r[c * 4 + f] = sum(a[k * 4 + f] * b[c * 4 + k] for k in range(4))
    return r


def matriz_nodo(nodo):
    if 'matrix' in nodo:
        return list(nodo['matrix'])
    tx, ty, tz = nodo.get('translation', [0, 0, 0])
    x, y, z, w = nodo.get('rotation', [0, 0, 0, 1])
    sx, sy, sz = nodo.get('scale', [1, 1, 1])
    # M = T * R * S  (cuaternion -> matriz de rotacion)
    r = [1 - 2 * (y * y + z * z), 2 * (x * y + z * w), 2 * (x * z - y * w), 0,
         2 * (x * y - z * w), 1 - 2 * (x * x + z * z), 2 * (y * z + x * w), 0,
         2 * (x * z + y * w), 2 * (y * z - x * w), 1 - 2 * (x * x + y * y), 0,
         0, 0, 0, 1]
    s = [sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1]
    t = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, tx, ty, tz, 1]
    return multiplicar(t, multiplicar(r, s))


def transformar_punto(m, p):
    x, y, z = p
    return (m[0] * x + m[4] * y + m[8] * z + m[12],
            m[1] * x + m[5] * y + m[9] * z + m[13],
            m[2] * x + m[6] * y + m[10] * z + m[14])


def transformar_normal(m, n):
    # Aproximacion valida para escalas uniformes: se usa la parte 3x3
    x, y, z = n
    r = (m[0] * x + m[4] * y + m[8] * z,
         m[1] * x + m[5] * y + m[9] * z,
         m[2] * x + m[6] * y + m[10] * z)
    largo = math.sqrt(r[0] ** 2 + r[1] ** 2 + r[2] ** 2) or 1.0
    return (r[0] / largo, r[1] / largo, r[2] / largo)


def convertir(ruta_glb, ruta_obj):
    gltf, binario = leer_glb(ruta_glb)
    base = os.path.splitext(os.path.basename(ruta_obj))[0]
    ruta_mtl = os.path.splitext(ruta_obj)[0] + '.mtl'

    # Recorre los nodos de la escena acumulando la matriz del padre
    instancias = []
    def visitar(i, padre):
        nodo = gltf['nodes'][i]
        m = multiplicar(padre, matriz_nodo(nodo))
        if 'mesh' in nodo:
            instancias.append((nodo['mesh'], m))
        for h in nodo.get('children', []):
            visitar(h, m)
    escena = gltf['scenes'][gltf.get('scene', 0)]
    for raiz in escena['nodes']:
        visitar(raiz, identidad())

    materiales = gltf.get('materials', [])
    v_total = vt_total = vn_total = 0
    caras = 0
    with open(ruta_obj, 'w') as obj:
        obj.write('# Convertido desde %s con tools/glb_a_obj.py\n' % os.path.basename(ruta_glb))
        obj.write('mtllib %s.mtl\n' % base)
        for id_malla, m in instancias:
            for prim in gltf['meshes'][id_malla]['primitives']:
                if prim.get('mode', 4) != 4:      # solo triangulos
                    continue
                pos, nor, uv, idx = leer_primitiva(gltf, binario, prim)
                pos = [transformar_punto(m, p) for p in pos]
                nor = [transformar_normal(m, n) for n in nor] if nor else None

                for p in pos:
                    obj.write('v %.6f %.6f %.6f\n' % p)
                if uv:
                    for u, v in uv:
                        obj.write('vt %.6f %.6f\n' % (u, 1.0 - v))   # glTF tiene V invertida
                if nor:
                    for n in nor:
                        obj.write('vn %.6f %.6f %.6f\n' % n)

                mat = prim.get('material')
                obj.write('usemtl %s\n' % (('mat%d' % mat) if mat is not None else 'defecto'))
                for k in range(0, len(idx) - 2, 3):
                    esquinas = []
                    for j in idx[k:k + 3]:
                        v = str(v_total + j + 1)
                        t = str(vt_total + j + 1) if uv else ''
                        n = str(vn_total + j + 1) if nor else ''
                        esquinas.append(v + ('/' + t + ('/' + n if n else '') if (t or n) else ''))
                    obj.write('f %s\n' % ' '.join(esquinas))
                    caras += 1
                v_total += len(pos)
                vt_total += len(uv) if uv else 0
                vn_total += len(nor) if nor else 0

    # Material PBR (baseColor, metallic, roughness) -> Phong (Kd, Ks, Ns)
    carpeta = os.path.dirname(os.path.abspath(ruta_obj))
    with open(ruta_mtl, 'w') as mtl:
        mtl.write('# Materiales convertidos desde PBR (glTF) a Phong\n')
        mtl.write('newmtl defecto\nKa 0.2 0.2 0.2\nKd 0.8 0.8 0.8\nKs 0.2 0.2 0.2\nNs 20\n\n')
        for i, mat in enumerate(materiales):
            pbr = mat.get('pbrMetallicRoughness', {})
            r, g, b, a = pbr.get('baseColorFactor', [1, 1, 1, 1])
            metal = pbr.get('metallicFactor', 1.0)
            rugo = pbr.get('roughnessFactor', 1.0)
            esp = 0.04 + 0.5 * metal * (1.0 - rugo) + 0.25 * (1.0 - rugo)
            brillo = max(2.0, 128.0 * (1.0 - rugo) ** 2)
            mtl.write('# %s\n' % mat.get('name', 'sin nombre'))
            mtl.write('newmtl mat%d\n' % i)
            mtl.write('Ka %.4f %.4f %.4f\n' % (r * 0.3, g * 0.3, b * 0.3))
            mtl.write('Kd %.4f %.4f %.4f\n' % (r, g, b))
            mtl.write('Ks %.4f %.4f %.4f\n' % (esp, esp, esp))
            mtl.write('Ns %.1f\n' % brillo)
            mtl.write('d %.4f\n' % a)
            if 'baseColorTexture' in pbr:
                nombre = exportar_textura(gltf, binario, pbr['baseColorTexture']['index'], carpeta, base)
                if nombre:
                    mtl.write('map_Kd %s\n' % nombre)
            mtl.write('\n')

    print('%s -> %s  (%d vertices, %d triangulos, %d materiales)'
          % (os.path.basename(ruta_glb), os.path.basename(ruta_obj), v_total, caras, len(materiales)))


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('uso: python3 glb_a_obj.py entrada.glb salida.obj')
        sys.exit(1)
    convertir(sys.argv[1], sys.argv[2])
