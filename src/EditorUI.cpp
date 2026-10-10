// =============================================================
//  EditorUI.cpp - Interfaz grafica del editor (Dear ImGui)
// =============================================================
#include "EditorUI.hpp"
#include "Examples.hpp"

#include "imgui.h"

#include <cstdio>
#include <cstring>

static const float ANCHO_IZQ = 270.0f;     // panel Escena
static const float ANCHO_DER = 340.0f;     // panel Propiedades
static const float ALTO_INF  = 250.0f;     // paneles Recursos y Animacion

static const ImGuiWindowFlags PANEL =
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

// stb_image voltea las imagenes al cargarlas (fila 0 abajo), asi que
// para mostrarlas derechas en la interfaz se invierte la coordenada V.
static const ImVec2 UV0(0.0f, 1.0f), UV1(1.0f, 0.0f);

static ImTextureID textureId(GLuint id) {
    return (ImTextureID)(intptr_t)id;
}

EditorUI::EditorUI()
    : salir(false), anchoVentana(1280.0f), altoVentana(720.0f), altoMenu(20.0f),
      dialogoPendiente(DLG_NINGUNO), escalaUniforme(false), clipActual(0), objetoDeClip(0),
      eliminarId(0), duplicarId(0), moverHijo(0), moverPadre(-1) {
    nombreEscena[0] = '\0';
    rutaModelo[0] = '\0';
}

// ------------------------------------------------------------------
//  Estilo y fuente
// ------------------------------------------------------------------
void EditorUI::init() {
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = 0;            // la distribucion de paneles es fija: no se guarda

    // Se intenta una fuente del sistema (mas legible); si no hay, la de ImGui
    static const char* FUENTES[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    };
    bool cargada = false;
    for (int i = 0; i < 3 && !cargada; i++) {
        FILE* f = fopen(FUENTES[i], "rb");
        if (!f) continue;
        fclose(f);
        cargada = io.Fonts->AddFontFromFileTTF(FUENTES[i], 15.0f) != 0;
    }
    if (!cargada) io.Fonts->AddFontDefault();

    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = 0.0f;
    s.FrameRounding = 3.0f;
    s.GrabRounding = 3.0f;
    s.TabRounding = 3.0f;
    s.WindowBorderSize = 1.0f;
    s.FramePadding = ImVec2(6.0f, 3.0f);
    s.ItemSpacing = ImVec2(8.0f, 5.0f);
    s.Colors[ImGuiCol_WindowBg]      = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
    s.Colors[ImGuiCol_TitleBg]       = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);
    s.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.13f, 0.19f, 1.00f);
    s.Colors[ImGuiCol_MenuBarBg]     = ImVec4(0.08f, 0.09f, 0.11f, 1.00f);
    s.Colors[ImGuiCol_FrameBg]       = ImVec4(0.17f, 0.19f, 0.24f, 1.00f);
    s.Colors[ImGuiCol_Header]        = ImVec4(0.16f, 0.33f, 0.62f, 0.75f);
    s.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.40f, 0.72f, 0.80f);
    s.Colors[ImGuiCol_PopupBg]       = ImVec4(0.10f, 0.11f, 0.14f, 0.98f);
}

void EditorUI::viewport(int& x, int& y, int& ancho, int& alto) const {
    x = (int)ANCHO_IZQ;
    y = (int)ALTO_INF;
    ancho = (int)(anchoVentana - ANCHO_IZQ - ANCHO_DER);
    alto = (int)(altoVentana - altoMenu - ALTO_INF);
    if (ancho < 1) ancho = 1;
    if (alto < 1) alto = 1;
}

void EditorUI::requestSave(Editor& ed) {
    if (ed.archivo.empty()) requestSaveAs(ed);
    else ed.saveScene(ed.archivo);
}

void EditorUI::requestSaveAs(Editor& ed) {
    snprintf(nombreEscena, sizeof(nombreEscena), "%s", ed.archivo.empty() ? "mi_escena" : ed.archivo.c_str());
    dialogoPendiente = DLG_GUARDAR;
}

void EditorUI::requestOpen()   { dialogoPendiente = DLG_ABRIR; }
void EditorUI::requestImport() { dialogoPendiente = DLG_IMPORTAR; }

void EditorUI::draw(Editor& ed) {
    ImGuiIO& io = ImGui::GetIO();
    anchoVentana = io.DisplaySize.x;
    altoVentana = io.DisplaySize.y;

    drawMenu(ed);
    drawHierarchy(ed);
    drawProperties(ed);
    drawResources(ed);
    drawAnimation(ed);
    drawToolbar(ed);
    drawDialogs(ed);
}

// ------------------------------------------------------------------
//  Barra de menu
// ------------------------------------------------------------------
void EditorUI::drawCreateItems(Editor& ed, int padre) {
    static const int PRIMITIVAS[] = { OBJ_CUBO, OBJ_ESFERA, OBJ_CILINDRO, OBJ_CONO, OBJ_PLANO, OBJ_TORO, OBJ_DISCO };
    for (int i = 0; i < 7; i++)
        if (ImGui::MenuItem(objectTypeName(PRIMITIVAS[i]))) ed.createObject(PRIMITIVAS[i], padre);
    ImGui::Separator();
    if (ImGui::MenuItem("Luz puntual"))     { SceneObject* o = ed.createLight(LUZ_PUNTUAL);     if (padre) ed.escena.setParent(o->id, padre, false); }
    if (ImGui::MenuItem("Luz direccional")) { SceneObject* o = ed.createLight(LUZ_DIRECCIONAL); if (padre) ed.escena.setParent(o->id, padre, false); }
    if (ImGui::MenuItem("Foco"))            { SceneObject* o = ed.createLight(LUZ_FOCO);        if (padre) ed.escena.setParent(o->id, padre, false); }
    ImGui::Separator();
    if (ImGui::MenuItem("Emisor de partículas")) ed.createObject(OBJ_EMISOR, padre);
    if (ImGui::MenuItem("Vacío (grupo / articulación)")) ed.createObject(OBJ_VACIO, padre);
}

void EditorUI::drawMenu(Editor& ed) {
    if (!ImGui::BeginMainMenuBar()) return;
    altoMenu = ImGui::GetWindowSize().y;
    bool haySeleccion = ed.escena.selected() != 0;

    if (ImGui::BeginMenu("Archivo")) {
        if (ImGui::MenuItem("Nueva escena", "Ctrl+N")) ed.newScene();
        if (ImGui::MenuItem("Abrir...", "Ctrl+O")) requestOpen();
        if (ImGui::MenuItem("Guardar", "Ctrl+S")) requestSave(ed);
        if (ImGui::MenuItem("Guardar como...")) requestSaveAs(ed);
        ImGui::Separator();
        if (ImGui::BeginMenu("Ejemplos")) {
            for (int i = 0; i < Examples::count(); i++)
                if (ImGui::MenuItem(Examples::name(i))) ed.loadExample(i);
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Salir")) salir = true;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Crear")) {
        drawCreateItems(ed, 0);
        ImGui::Separator();
        if (ImGui::MenuItem("Importar modelo OBJ...")) requestImport();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Editar")) {
        if (ImGui::MenuItem("Duplicar", "Ctrl+D", false, haySeleccion)) ed.duplicateSelected();
        if (ImGui::MenuItem("Eliminar", "Supr / X", false, haySeleccion)) ed.deleteSelected();
        if (ImGui::MenuItem("Quitar padre", 0, false, haySeleccion && ed.escena.selected()->padre != 0))
            ed.escena.setParent(ed.escena.seleccion, 0, true);
        if (ImGui::MenuItem("Enfocar selección", "F", false, haySeleccion)) ed.focusSelected();
        if (ImGui::MenuItem("Deseleccionar", "Esc", false, haySeleccion)) ed.escena.seleccion = 0;
        ImGui::Separator();
        if (ImGui::MenuItem("Mover", "W", ed.gizmo.modo == Gizmo::MOVER)) ed.gizmo.modo = Gizmo::MOVER;
        if (ImGui::MenuItem("Rotar", "E", ed.gizmo.modo == Gizmo::ROTAR)) ed.gizmo.modo = Gizmo::ROTAR;
        if (ImGui::MenuItem("Escalar", "R", ed.gizmo.modo == Gizmo::ESCALAR)) ed.gizmo.modo = Gizmo::ESCALAR;
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Ver")) {
        if (ImGui::MenuItem("Perspectiva", "P", !ed.camara.ortogonal)) ed.camara.ortogonal = false;
        if (ImGui::MenuItem("Ortogonal", "P", ed.camara.ortogonal)) ed.camara.ortogonal = true;
        ImGui::Separator();
        if (ImGui::MenuItem("Sólido", "Z", !ed.opciones.wireframe)) ed.opciones.wireframe = false;
        if (ImGui::MenuItem("Wireframe (alambre)", "Z", ed.opciones.wireframe)) ed.opciones.wireframe = true;
        ImGui::Separator();
        ImGui::MenuItem("Sombreado suave (Gouraud)", 0, &ed.opciones.gouraud);
        ImGui::MenuItem("Texturas", "T", &ed.opciones.texturas);
        ImGui::MenuItem("Iluminación", "L", &ed.opciones.iluminacion);
        ImGui::MenuItem("Cuadrícula", "G", &ed.escena.cuadricula);
        ImGui::MenuItem("Iconos de luces y vacíos", 0, &ed.opciones.ayudas);
        ImGui::Separator();
        if (ImGui::MenuItem("Vista frontal", "1"))  ed.camara.setView(0.0f, 0.0f);
        if (ImGui::MenuItem("Vista lateral", "3"))  ed.camara.setView(90.0f, 0.0f);
        if (ImGui::MenuItem("Vista superior", "7")) ed.camara.setView(0.0f, 89.0f);
        if (ImGui::MenuItem("Restablecer cámara", "0")) ed.camara.reset();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Animación")) {
        if (ImGui::MenuItem(ed.animador.isPlaying() ? "Pausar" : "Reproducir", "Espacio")) ed.togglePlay();
        if (ImGui::MenuItem("Detener")) ed.stop();
        if (ImGui::MenuItem("Reiniciar")) ed.restart();
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Ayuda")) {
        if (ImGui::MenuItem("Controles...")) dialogoPendiente = DLG_CONTROLES;
        ImGui::EndMenu();
    }

    ImGui::Separator();
    ImGui::TextDisabled("%s%s", ed.archivo.empty() ? "(sin guardar)" : ed.archivo.c_str(), "  |");
    ImGui::TextDisabled("%s", ed.mensaje.c_str());
    ImGui::EndMainMenuBar();
}

// ------------------------------------------------------------------
//  Panel Escena: arbol de objetos
// ------------------------------------------------------------------
void EditorUI::drawNode(Editor& ed, SceneObject* o) {
    std::vector<SceneObject*> hijos;
    ed.escena.children(o->id, hijos);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                               ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
    if (hijos.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (o->id == ed.escena.seleccion) flags |= ImGuiTreeNodeFlags_Selected;

    if (!o->visible) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    bool abierto = ImGui::TreeNodeEx((void*)(intptr_t)o->id, flags, "%s", o->nombre.c_str());
    if (!o->visible) ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) ed.escena.seleccion = o->id;

    // Menu contextual (clic derecho)
    if (ImGui::BeginPopupContextItem()) {
        ed.escena.seleccion = o->id;
        if (ImGui::BeginMenu("Crear hijo")) {
            drawCreateItems(ed, o->id);
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Duplicar")) duplicarId = o->id;
        if (ImGui::MenuItem("Eliminar")) eliminarId = o->id;
        if (ImGui::MenuItem("Quitar padre", 0, false, o->padre != 0)) { moverHijo = o->id; moverPadre = 0; }
        if (ImGui::MenuItem("Enfocar")) ed.focusSelected();
        ImGui::EndPopup();
    }

    // Arrastrar un objeto sobre otro lo convierte en su hijo
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("OBJETO", &o->id, sizeof(int));
        ImGui::Text("%s", o->nombre.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("OBJETO")) {
            moverHijo = *(const int*)p->Data;
            moverPadre = o->id;
        }
        ImGui::EndDragDropTarget();
    }

    // Tipo del objeto, alineado a la derecha (si el nombre deja espacio)
    const char* tipo = objectTypeName(o->tipo);
    float finNombre = ImGui::GetItemRectMin().x + ImGui::GetTreeNodeToLabelSpacing() +
                      ImGui::CalcTextSize(o->nombre.c_str()).x - ImGui::GetWindowPos().x;
    float inicioTipo = ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(tipo).x;
    if (finNombre + 8.0f < inicioTipo) {
        ImGui::SameLine(inicioTipo);
        ImGui::TextDisabled("%s", tipo);
    }

    if (abierto) {
        for (size_t i = 0; i < hijos.size(); i++) drawNode(ed, hijos[i]);
        ImGui::TreePop();
    }
}

void EditorUI::drawHierarchy(Editor& ed) {
    ImGui::SetNextWindowPos(ImVec2(0.0f, altoMenu));
    ImGui::SetNextWindowSize(ImVec2(ANCHO_IZQ, altoVentana - altoMenu - ALTO_INF));
    ImGui::Begin("Escena", 0, PANEL);

    if (ImGui::Button("+ Crear")) ImGui::OpenPopup("crear");
    if (ImGui::BeginPopup("crear")) {
        drawCreateItems(ed, 0);
        ImGui::Separator();
        if (ImGui::MenuItem("Importar modelo OBJ...")) requestImport();
        ImGui::EndPopup();
    }
    bool hay = ed.escena.selected() != 0;
    ImGui::SameLine();
    ImGui::BeginDisabled(!hay);
    if (ImGui::Button("Duplicar")) duplicarId = ed.escena.seleccion;
    ImGui::SameLine();
    if (ImGui::Button("Eliminar")) eliminarId = ed.escena.seleccion;
    ImGui::EndDisabled();
    ImGui::Separator();

    ImGui::BeginChild("arbol");
    // La lista se copia: crear un hijo desde el menu contextual la modifica
    std::vector<SceneObject*> raices;
    ed.escena.children(0, raices);
    for (size_t i = 0; i < raices.size(); i++) drawNode(ed, raices[i]);

    // El espacio vacio: clic deselecciona, soltar ahi quita el padre
    ImVec2 libre = ImGui::GetContentRegionAvail();
    if (libre.y < 40.0f) libre.y = 40.0f;
    if (libre.x < 1.0f) libre.x = 1.0f;
    if (ImGui::InvisibleButton("##vacio", libre)) ed.escena.seleccion = 0;
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("OBJETO")) {
            moverHijo = *(const int*)p->Data;
            moverPadre = 0;
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::EndChild();
    ImGui::End();

    // Las operaciones se aplican con el arbol ya recorrido
    if (moverHijo != 0 && moverPadre >= 0) {
        if (!ed.escena.setParent(moverHijo, moverPadre, true))
            ed.mensaje = "No se puede: un objeto no puede ser hijo de sí mismo ni de sus descendientes.";
    }
    if (duplicarId != 0) { ed.escena.seleccion = duplicarId; ed.duplicateSelected(); }
    if (eliminarId != 0) { ed.escena.seleccion = eliminarId; ed.deleteSelected(); }
    eliminarId = duplicarId = moverHijo = 0;
    moverPadre = -1;
}

// ------------------------------------------------------------------
//  Panel Propiedades
// ------------------------------------------------------------------
void EditorUI::drawProperties(Editor& ed) {
    ImGui::SetNextWindowPos(ImVec2(anchoVentana - ANCHO_DER, altoMenu));
    ImGui::SetNextWindowSize(ImVec2(ANCHO_DER, altoVentana - altoMenu - ALTO_INF));
    ImGui::Begin("Propiedades", 0, PANEL);
    if (ImGui::BeginTabBar("pestanas")) {
        if (ImGui::BeginTabItem("Objeto"))  { drawObjectTab(ed);      ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Entorno")) { drawEnvironmentTab(ed); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Vista"))   { drawViewTab(ed);        ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorUI::drawObjectTab(Editor& ed) {
    SceneObject* o = ed.escena.selected();
    if (!o) {
        ImGui::Spacing();
        ImGui::TextWrapped("Ningún objeto seleccionado.");
        ImGui::TextWrapped("Haz clic en un objeto del Viewport o del panel Escena, "
                           "o crea uno nuevo desde el menú Crear.");
        return;
    }
    ImGui::BeginChild("objeto");
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.62f);

    char nombre[128];
    snprintf(nombre, sizeof(nombre), "%s", o->nombre.c_str());
    if (ImGui::InputText("Nombre", nombre, sizeof(nombre)) && nombre[0] != '\0') o->nombre = nombre;
    ImGui::LabelText("Tipo", "%s", objectTypeName(o->tipo));
    ImGui::Checkbox("Visible", &o->visible);

    // Padre: cualquier objeto que no sea el mismo ni uno de sus descendientes
    SceneObject* padre = ed.escena.find(o->padre);
    if (ImGui::BeginCombo("Padre", padre ? padre->nombre.c_str() : "(ninguno)")) {
        if (ImGui::Selectable("(ninguno)", padre == 0)) ed.escena.setParent(o->id, 0, true);
        for (size_t i = 0; i < ed.escena.objetos.size(); i++) {
            SceneObject* p = ed.escena.objetos[i];
            if (p == o || ed.escena.isDescendant(p->id, o->id)) continue;
            ImGui::PushID(p->id);
            if (ImGui::Selectable(p->nombre.c_str(), p == padre)) ed.escena.setParent(o->id, p->id, true);
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    // --- Transformacion -------------------------------------------
    if (ImGui::CollapsingHeader("Transformación", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::DragFloat3("Posición", &o->posicion.x, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat3("Rotación", &o->rotacion.x, 1.0f, 0.0f, 0.0f, "%.1f");
        Vec3 antes = o->escala;
        if (ImGui::DragFloat3("Escala", &o->escala.x, 0.02f, 0.0f, 0.0f, "%.2f")) {
            if (escalaUniforme) {
                // El eje que cambio arrastra a los otros dos en la misma proporcion
                for (int k = 0; k < 3; k++) {
                    if (o->escala[k] == antes[k] || fabsf(antes[k]) < 1e-6f) continue;
                    float factor = o->escala[k] / antes[k];
                    o->escala = antes * factor;
                    break;
                }
            }
            for (int k = 0; k < 3; k++)
                if (fabsf(o->escala[k]) < 0.001f) o->escala[k] = 0.001f;
        }
        ImGui::Checkbox("Escala uniforme", &escalaUniforme);
        ImGui::SameLine();
        if (ImGui::Button("Restablecer")) {
            o->posicion = Vec3();
            o->rotacion = Vec3();
            o->escala = Vec3(1.0f, 1.0f, 1.0f);
        }
        if (ed.animador.estado != Animator::DETENIDO && !o->animaciones.empty())
            ImGui::TextDisabled("Reproduciendo: aquí se edita el valor base.");
    }

    // --- Luz -------------------------------------------------------
    if (o->tipo == OBJ_LUZ && ImGui::CollapsingHeader("Luz", ImGuiTreeNodeFlags_DefaultOpen)) {
        LightProps& l = o->luz;
        ImGui::Checkbox("Encendida", &l.encendida);
        ImGui::Combo("Tipo de luz", &l.tipo, "Puntual\0Direccional\0Foco\0");
        ImGui::ColorEdit3("Color", l.color);
        ImGui::SliderFloat("Difusa", &l.intensidad, 0.0f, 3.0f, "%.2f");
        ImGui::SliderFloat("Especular", &l.especular, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Ambiental", &l.ambiente, 0.0f, 1.0f, "%.2f");
        if (l.tipo != LUZ_DIRECCIONAL) {
            ImGui::SliderFloat("Aten. lineal", &l.atenLineal, 0.0f, 0.5f, "%.3f");
            ImGui::SliderFloat("Aten. cuadrática", &l.atenCuadratica, 0.0f, 0.1f, "%.4f");
        }
        if (l.tipo == LUZ_FOCO) {
            ImGui::SliderFloat("Ángulo del cono", &l.anguloFoco, 1.0f, 90.0f, "%.0f°");
            ImGui::SliderFloat("Concentración", &l.exponenteFoco, 0.0f, 64.0f, "%.0f");
        }
        if (l.tipo != LUZ_PUNTUAL)
            ImGui::TextDisabled("Apunta hacia su eje -Y: gírala con Rotación.");
    }

    // --- Emisor de particulas ---------------------------------------
    if (o->tipo == OBJ_EMISOR && ImGui::CollapsingHeader("Partículas", ImGuiTreeNodeFlags_DefaultOpen)) {
        EmitterProps& e = o->emisor;
        ImGui::SliderFloat("Partículas/s", &e.tasa, 0.0f, 600.0f, "%.0f");
        ImGui::SliderFloat("Vida (s)", &e.vida, 0.1f, 8.0f, "%.2f");
        ImGui::SliderFloat("Velocidad", &e.velocidad, 0.0f, 20.0f, "%.2f");
        ImGui::SliderFloat("Dispersión", &e.dispersion, 0.0f, 180.0f, "%.0f°");
        ImGui::SliderFloat("Gravedad", &e.gravedad, -10.0f, 20.0f, "%.2f");
        ImGui::SliderFloat("Tamaño inicial", &e.tamInicio, 0.01f, 2.0f, "%.2f");
        ImGui::SliderFloat("Tamaño final", &e.tamFin, 0.0f, 2.0f, "%.2f");
        ImGui::ColorEdit4("Color inicial", e.colorInicio);
        ImGui::ColorEdit4("Color final", e.colorFin);
        ImGui::Checkbox("Mezcla aditiva (fuego, chispas)", &e.aditivo);
        ImGui::TextDisabled("Emite hacia su eje +Y al reproducir.");
    }

    if (objectHasGeometry(o->tipo)) {
        // --- Modelo OBJ ---------------------------------------------
        if (o->tipo == OBJ_MODELO && ImGui::CollapsingHeader("Modelo OBJ", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginCombo("Archivo", o->modelo.c_str())) {
                for (size_t i = 0; i < ed.recursos.modelos.size(); i++)
                    if (ImGui::Selectable(ed.recursos.modelos[i].c_str(), ed.recursos.modelos[i] == o->modelo))
                        o->modelo = ed.recursos.modelos[i];
                ImGui::EndCombo();
            }
            Model* m = ed.recursos.model(o->modelo);
            if (m) ImGui::TextDisabled("%d triángulos", m->triangleCount());
            else   ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No se pudo cargar el archivo");
            ImGui::Checkbox("Usar materiales del MTL", &o->usarMaterialModelo);
        }
        bool bloqueado = o->tipo == OBJ_MODELO && o->usarMaterialModelo;

        // --- Material -----------------------------------------------
        if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (bloqueado) ImGui::TextDisabled("Desmarca \"Usar materiales del MTL\" para editarlo.");
            ImGui::BeginDisabled(bloqueado);
            Material& m = o->material;
            if (ImGui::BeginCombo("Preajuste", "Elegir...")) {
                for (int i = 0; i < Material::presetCount(); i++)
                    if (ImGui::Selectable(Material::presetName(i))) m = Material::preset(i, m);
                ImGui::EndCombo();
            }
            ImGui::ColorEdit3("Color", m.color);
            ImGui::SliderFloat("Ambiental", &m.ambiente, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Difusa", &m.difuso, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Especular", &m.especular, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Brillo", &m.brillo, 1.0f, 128.0f, "%.0f");
            ImGui::SliderFloat("Emisión", &m.emision, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("Opacidad", &m.opacidad, 0.0f, 1.0f, "%.2f");
            ImGui::Checkbox("Mezcla alfa (texturas con transparencia)", &m.mezcla);
            ImGui::EndDisabled();
        }

        // --- Textura ------------------------------------------------
        if (ImGui::CollapsingHeader("Textura", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginDisabled(bloqueado);
            GLuint id = ed.recursos.texture(o->textura);
            if (id != 0) {
                ImGui::Image(textureId(id), ImVec2(56.0f, 56.0f), UV0, UV1);
                ImGui::SameLine();
            }
            ImGui::BeginGroup();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 8.0f);
            if (ImGui::BeginCombo("##textura", o->textura.empty() ? "(ninguna)" : o->textura.c_str())) {
                if (ImGui::Selectable("(ninguna)", o->textura.empty())) ed.assignTexture("");
                for (size_t i = 0; i < ed.recursos.texturas.size(); i++) {
                    const std::string& n = ed.recursos.texturas[i].nombre;
                    if (ImGui::Selectable(n.c_str(), n == o->textura)) ed.assignTexture(n);
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            if (ImGui::Button("Quitar textura")) ed.assignTexture("");
            ImGui::EndGroup();
            ImGui::DragFloat2("Repetición U, V", o->texRepetir, 0.05f, 0.05f, 100.0f, "%.2f");
            ImGui::EndDisabled();
        }

        // --- Fisica -------------------------------------------------
        if (ImGui::CollapsingHeader("Física")) {
            PhysicsProps& f = o->fisica;
            ImGui::Combo("Cuerpo", &f.modo, "Ninguno\0Dinámico (cae y rebota)\0Colisionador estático\0");
            if (f.modo == FISICA_DINAMICO) {
                ImGui::SliderFloat("Masa", &f.masa, 0.1f, 20.0f, "%.2f");
                ImGui::SliderFloat("Rebote", &f.rebote, 0.0f, 1.0f, "%.2f");
                ImGui::SliderFloat("Fricción", &f.friccion, 0.0f, 1.0f, "%.2f");
                ImGui::DragFloat3("Velocidad inicial", &f.velInicial.x, 0.1f, 0.0f, 0.0f, "%.1f");
                if (o->padre != 0)
                    ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Solo se simulan objetos sin padre.");
            }
            if (f.modo != FISICA_NINGUNA) {
                if (ed.fisica.isActive()) ImGui::Text("En contacto: %s", f.enContacto ? "sí" : "no");
                else ImGui::TextDisabled("Pulsa Reproducir para simular.");
            }
        }
    }

    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void EditorUI::drawEnvironmentTab(Editor& ed) {
    Scene& e = ed.escena;
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.58f);
    if (ImGui::CollapsingHeader("Iluminación global", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Luz ambiental", e.ambiente);
        ImGui::Text("Luces encendidas: %d de %d", ed.lucesActivas, (int)Lighting::MAX_LUCES);
        ImGui::TextDisabled("Las luces son objetos: Crear > Luz.");
    }
    if (ImGui::CollapsingHeader("Fondo", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit3("Color de fondo", e.fondo);
        ImGui::Checkbox("Cuadrícula del suelo", &e.cuadricula);
    }
    if (ImGui::CollapsingHeader("Física", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Gravedad", &e.gravedad, 0.0f, 30.0f, "%.1f");
        ImGui::Checkbox("Suelo", &e.suelo);
        ImGui::DragFloat("Altura del suelo", &e.alturaSuelo, 0.05f, 0.0f, 0.0f, "%.2f");
        if (ed.fisica.isActive()) ImGui::Text("Contactos: %d", ed.fisica.contactos);
    }
    if (ImGui::CollapsingHeader("Estadísticas", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Objetos: %d", (int)e.objetos.size());
        ImGui::Text("Partículas: %d", ed.particulas.count());
        ImGui::Text("%.0f cuadros por segundo", ImGui::GetIO().Framerate);
    }
    ImGui::PopItemWidth();
}

void EditorUI::drawViewTab(Editor& ed) {
    Camera& c = ed.camara;
    RenderOptions& r = ed.opciones;
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.58f);
    if (ImGui::CollapsingHeader("Cámara", ImGuiTreeNodeFlags_DefaultOpen)) {
        int proyeccion = c.ortogonal ? 1 : 0;
        if (ImGui::Combo("Proyección", &proyeccion, "Perspectiva\0Ortogonal\0")) c.ortogonal = proyeccion == 1;
        ImGui::SliderFloat("Campo de visión", &c.fov, 10.0f, 110.0f, "%.0f°");
        ImGui::DragFloat("Distancia", &c.distancia, 0.1f, 0.2f, 1000.0f, "%.2f");
        ImGui::SliderFloat("Giro (yaw)", &c.yaw, -180.0f, 180.0f, "%.0f°");
        ImGui::SliderFloat("Elevación (pitch)", &c.pitch, -89.0f, 89.0f, "%.0f°");
        ImGui::DragFloat3("Objetivo", &c.objetivo.x, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Plano cercano", &c.cerca, 0.01f, 0.01f, 100.0f, "%.2f");
        ImGui::DragFloat("Plano lejano", &c.lejos, 1.0f, 10.0f, 5000.0f, "%.0f");
        if (ImGui::Button("Frente"))   c.setView(0.0f, 0.0f);
        ImGui::SameLine();
        if (ImGui::Button("Lado"))     c.setView(90.0f, 0.0f);
        ImGui::SameLine();
        if (ImGui::Button("Arriba"))   c.setView(0.0f, 89.0f);
        ImGui::SameLine();
        if (ImGui::Button("Restablecer")) c.reset();
    }
    if (ImGui::CollapsingHeader("Renderizado", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Checkbox("Wireframe (alambre)", &r.wireframe);
        ImGui::Checkbox("Sombreado suave (Gouraud)", &r.gouraud);
        ImGui::Checkbox("Texturas", &r.texturas);
        ImGui::Checkbox("Iluminación", &r.iluminacion);
        ImGui::Checkbox("Depth buffer", &r.depthBuffer);
        ImGui::Checkbox("Iconos de luces y vacíos", &r.ayudas);
        ImGui::Combo("Detalle de malla", &r.detalleMalla, "Bajo\0Medio\0Alto\0");
    }
    ImGui::PopItemWidth();
}

// ------------------------------------------------------------------
//  Panel Recursos: texturas, modelos y escenas del proyecto
// ------------------------------------------------------------------
void EditorUI::drawResources(Editor& ed) {
    float ancho = anchoVentana * 0.5f;
    ImGui::SetNextWindowPos(ImVec2(0.0f, altoVentana - ALTO_INF));
    ImGui::SetNextWindowSize(ImVec2(ancho, ALTO_INF));
    ImGui::Begin("Recursos", 0, PANEL);
    SceneObject* sel = ed.escena.selected();

    if (ImGui::BeginTabBar("recursos")) {
        if (ImGui::BeginTabItem("Texturas")) {
            if (ImGui::SmallButton("Recargar carpeta")) ed.recursos.scan();
            ImGui::SameLine();
            ImGui::TextDisabled("assets/textures/  -  clic para asignar al objeto seleccionado");
            ImGui::BeginChild("texturas");
            const float lado = 64.0f;
            float disponible = ImGui::GetContentRegionAvail().x;
            float x = 0.0f;
            for (size_t i = 0; i < ed.recursos.texturas.size(); i++) {
                const std::string nombre = ed.recursos.texturas[i].nombre;
                GLuint id = ed.recursos.texture(nombre);
                bool usada = sel && sel->textura == nombre;

                if (x > 0.0f && x + lado + 16.0f > disponible) x = 0.0f;
                else if (x > 0.0f) ImGui::SameLine();
                x += lado + 20.0f;

                ImGui::PushID((int)i);
                ImGui::BeginGroup();
                if (usada) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.6f, 0.1f, 1.0f));
                bool clic = id != 0 ? ImGui::ImageButton("t", textureId(id), ImVec2(lado, lado), UV0, UV1)
                                    : ImGui::Button("?", ImVec2(lado + 12.0f, lado + 6.0f));
                if (usada) ImGui::PopStyleColor();
                std::string corto = nombre.size() > 11 ? nombre.substr(0, 10) + ".." : nombre;
                ImGui::TextUnformatted(corto.c_str());
                ImGui::EndGroup();
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", nombre.c_str());
                if (clic) ed.assignTexture(nombre);
                ImGui::PopID();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Modelos OBJ")) {
            if (ImGui::SmallButton("Recargar carpeta")) ed.recursos.scan();
            ImGui::SameLine();
            ImGui::TextDisabled("assets/models/");
            ImGui::BeginChild("modelos");
            for (size_t i = 0; i < ed.recursos.modelos.size(); i++) {
                ImGui::PushID((int)i);
                if (ImGui::SmallButton("Agregar a la escena")) ed.importModel(ed.recursos.modelos[i]);
                ImGui::SameLine();
                ImGui::TextUnformatted(ed.recursos.modelos[i].c_str());
                ImGui::PopID();
            }
            if (ed.recursos.modelos.empty()) ImGui::TextDisabled("No hay archivos .obj en la carpeta.");
            ImGui::Separator();
            if (ImGui::Button("Importar desde otra ruta...")) requestImport();
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Escenas")) {
            if (ImGui::Button("Guardar")) requestSave(ed);
            ImGui::SameLine();
            if (ImGui::Button("Guardar como...")) requestSaveAs(ed);
            ImGui::SameLine();
            if (ImGui::SmallButton("Recargar")) ed.recursos.scan();
            ImGui::SameLine();
            ImGui::TextDisabled("escenas/*.escena");
            ImGui::BeginChild("escenas");
            for (size_t i = 0; i < ed.recursos.escenas.size(); i++) {
                ImGui::PushID((int)i);
                if (ImGui::SmallButton("Abrir")) ed.loadScene(ed.recursos.escenas[i]);
                ImGui::SameLine();
                ImGui::TextUnformatted(ed.recursos.escenas[i].c_str());
                ImGui::PopID();
            }
            if (ed.recursos.escenas.empty()) ImGui::TextDisabled("Todavía no hay escenas guardadas.");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

// ------------------------------------------------------------------
//  Panel Animacion
// ------------------------------------------------------------------
static float propertyValue(const SceneObject& o, int propiedad) {
    if (propiedad <= ANIM_POS_Z) return o.posicion[propiedad - ANIM_POS_X];
    if (propiedad <= ANIM_ROT_Z) return o.rotacion[propiedad - ANIM_ROT_X];
    if (propiedad <= ANIM_ESC_Z) return o.escala[propiedad - ANIM_ESC_X];
    return o.escala.x;
}

void EditorUI::drawAnimation(Editor& ed) {
    float x = anchoVentana * 0.5f;
    ImGui::SetNextWindowPos(ImVec2(x, altoVentana - ALTO_INF));
    ImGui::SetNextWindowSize(ImVec2(anchoVentana - x, ALTO_INF));
    ImGui::Begin("Animación", 0, PANEL);

    // --- Reproduccion ---
    Animator& a = ed.animador;
    if (ImGui::Button(a.isPlaying() ? "Pausar" : "Reproducir", ImVec2(96.0f, 0.0f))) ed.togglePlay();
    ImGui::SameLine();
    if (ImGui::Button("Detener")) ed.stop();
    ImGui::SameLine();
    if (ImGui::Button("Reiniciar")) ed.restart();
    ImGui::SameLine();
    static const char* ESTADOS[] = { "Detenido", "Reproduciendo", "En pausa" };
    ImGui::Text("%s   t = %.2f s", ESTADOS[a.estado], a.tiempo);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    ImGui::SliderFloat("Velocidad", &a.velocidad, 0.1f, 4.0f, "x%.1f");
    ImGui::Separator();

    SceneObject* o = ed.escena.selected();
    if (!o) {
        ImGui::TextDisabled("Selecciona un objeto para configurar sus animaciones.");
        ImGui::End();
        return;
    }
    if (objetoDeClip != o->id) { objetoDeClip = o->id; clipActual = 0; }
    if (clipActual >= (int)o->animaciones.size()) clipActual = (int)o->animaciones.size() - 1;
    if (clipActual < 0) clipActual = 0;

    // --- Lista de clips del objeto (izquierda) ---
    float anchoLista = ImGui::GetContentRegionAvail().x * 0.42f;
    ImGui::BeginChild("clips", ImVec2(anchoLista, 0.0f));
    ImGui::Text("Objeto: %s", o->nombre.c_str());
    if (ImGui::Button("+ Añadir animación")) {
        AnimClip nuevo;
        o->animaciones.push_back(nuevo);
        clipActual = (int)o->animaciones.size() - 1;
    }
    for (size_t i = 0; i < o->animaciones.size(); i++) {
        AnimClip& c = o->animaciones[i];
        ImGui::PushID((int)i);
        ImGui::Checkbox("##activa", &c.activa);
        ImGui::SameLine();
        char etiqueta[128];
        snprintf(etiqueta, sizeof(etiqueta), "%s  %.4g -> %.4g  (%.4g s)",
                 AnimClip::propertyName(c.propiedad), c.inicio, c.fin, c.duracion);
        if (ImGui::Selectable(etiqueta, clipActual == (int)i)) clipActual = (int)i;
        ImGui::PopID();
    }
    if (o->animaciones.empty()) ImGui::TextDisabled("Este objeto no tiene animaciones.");
    ImGui::EndChild();
    ImGui::SameLine();

    // --- Editor del clip seleccionado (derecha) ---
    ImGui::BeginChild("clip");
    if (!o->animaciones.empty()) {
        AnimClip& c = o->animaciones[clipActual];
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (ImGui::BeginCombo("Tipo", AnimClip::propertyName(c.propiedad))) {
            for (int p = 0; p < ANIM_NUM_PROPIEDADES; p++)
                if (ImGui::Selectable(AnimClip::propertyName(p), p == c.propiedad)) c.propiedad = p;
            ImGui::EndCombo();
        }
        float paso = (c.propiedad >= ANIM_ROT_X && c.propiedad <= ANIM_ROT_Z) ? 1.0f : 0.05f;
        ImGui::DragFloat("Inicio", &c.inicio, paso, 0.0f, 0.0f, "%.2f");
        ImGui::SameLine();
        if (ImGui::SmallButton("= actual##i")) c.inicio = propertyValue(*o, c.propiedad);
        ImGui::DragFloat("Fin", &c.fin, paso, 0.0f, 0.0f, "%.2f");
        ImGui::SameLine();
        if (ImGui::SmallButton("= actual##f")) c.fin = propertyValue(*o, c.propiedad);
        ImGui::DragFloat("Duración (s)", &c.duracion, 0.05f, 0.05f, 600.0f, "%.2f");
        ImGui::DragFloat("Retardo (s)", &c.retardo, 0.05f, 0.0f, 600.0f, "%.2f");
        if (ImGui::BeginCombo("Repetición", AnimClip::loopName(c.repeticion))) {
            for (int r = 0; r < ANIM_NUM_REPETICIONES; r++)
                if (ImGui::Selectable(AnimClip::loopName(r), r == c.repeticion)) c.repeticion = r;
            ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("Interpolación", AnimClip::curveName(c.curva))) {
            for (int k = 0; k < ANIM_NUM_CURVAS; k++)
                if (ImGui::Selectable(AnimClip::curveName(k), k == c.curva)) c.curva = k;
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        if (ImGui::Button("Eliminar animación")) {
            o->animaciones.erase(o->animaciones.begin() + clipActual);
            if (clipActual > 0) clipActual--;
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

// ------------------------------------------------------------------
//  Barra flotante del Viewport
// ------------------------------------------------------------------
void EditorUI::drawToolbar(Editor& ed) {
    const ImGuiWindowFlags flotante =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowPos(ImVec2(ANCHO_IZQ + 8.0f, altoMenu + 8.0f));
    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin("##barra", 0, flotante);
    int modo = ed.gizmo.modo;
    ImGui::RadioButton("Mover", &modo, Gizmo::MOVER);     ImGui::SameLine();
    ImGui::RadioButton("Rotar", &modo, Gizmo::ROTAR);     ImGui::SameLine();
    ImGui::RadioButton("Escalar", &modo, Gizmo::ESCALAR); ImGui::SameLine();
    ed.gizmo.modo = (Gizmo::Mode)modo;
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    if (ImGui::Button(ed.camara.ortogonal ? "Ortogonal" : "Perspectiva")) ed.camara.ortogonal = !ed.camara.ortogonal;
    ImGui::SameLine();
    if (ImGui::Button(ed.opciones.wireframe ? "Wireframe" : "Sólido")) ed.opciones.wireframe = !ed.opciones.wireframe;
    ImGui::SameLine();
    ImGui::TextDisabled("|");
    ImGui::SameLine();
    if (ImGui::Button(ed.animador.isPlaying() ? "Pausar" : "Reproducir")) ed.togglePlay();
    ImGui::SameLine();
    if (ImGui::Button("Detener")) ed.stop();
    ImGui::End();

    // Ayuda rapida de navegacion, abajo a la izquierda del Viewport
    ImGui::SetNextWindowPos(ImVec2(ANCHO_IZQ + 8.0f, altoVentana - ALTO_INF - 8.0f), 0, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.7f);
    ImGui::Begin("##ayuda", 0, flotante | ImGuiWindowFlags_NoInputs);
    ImGui::TextDisabled("Clic: seleccionar   Arrastrar: orbitar   Clic derecho / Shift: desplazar   Rueda / + -: zoom");
    ImGui::End();
}

// ------------------------------------------------------------------
//  Dialogos
// ------------------------------------------------------------------
void EditorUI::drawDialogs(Editor& ed) {
    static const char* TITULOS[] = { "", "Guardar escena", "Abrir escena", "Importar modelo OBJ", "Controles" };
    if (dialogoPendiente != DLG_NINGUNO) {
        ed.recursos.scan();
        ImGui::OpenPopup(TITULOS[dialogoPendiente]);
        dialogoPendiente = DLG_NINGUNO;
    }
    ImVec2 centro(anchoVentana * 0.5f, altoVentana * 0.5f);

    ImGui::SetNextWindowPos(centro, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(TITULOS[DLG_GUARDAR], 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Se guarda en la carpeta escenas/ del proyecto.");
        ImGui::SetNextItemWidth(320.0f);
        bool enter = ImGui::InputText("Nombre", nombreEscena, sizeof(nombreEscena), ImGuiInputTextFlags_EnterReturnsTrue);
        if (!ed.recursos.escenas.empty()) {
            ImGui::TextDisabled("Escenas existentes (clic para sobrescribir):");
            for (size_t i = 0; i < ed.recursos.escenas.size(); i++)
                if (ImGui::Selectable(ed.recursos.escenas[i].c_str(), false, ImGuiSelectableFlags_DontClosePopups))
                    snprintf(nombreEscena, sizeof(nombreEscena), "%s", ed.recursos.escenas[i].c_str());
        }
        ImGui::Separator();
        if ((ImGui::Button("Guardar", ImVec2(120.0f, 0.0f)) || enter) && nombreEscena[0] != '\0') {
            ed.saveScene(nombreEscena);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancelar", ImVec2(120.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(centro, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(TITULOS[DLG_ABRIR], 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Escenas guardadas en escenas/:");
        ImGui::BeginChild("lista", ImVec2(340.0f, 200.0f), ImGuiChildFlags_Borders);
        for (size_t i = 0; i < ed.recursos.escenas.size(); i++) {
            if (ImGui::Selectable(ed.recursos.escenas[i].c_str())) {
                ed.loadScene(ed.recursos.escenas[i]);
                ImGui::CloseCurrentPopup();
            }
        }
        if (ed.recursos.escenas.empty()) ImGui::TextDisabled("Todavía no hay escenas guardadas.");
        ImGui::EndChild();
        if (ImGui::Button("Cancelar", ImVec2(120.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(centro, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(TITULOS[DLG_IMPORTAR], 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Modelos en assets/models/:");
        ImGui::BeginChild("lista", ImVec2(420.0f, 180.0f), ImGuiChildFlags_Borders);
        for (size_t i = 0; i < ed.recursos.modelos.size(); i++) {
            if (ImGui::Selectable(ed.recursos.modelos[i].c_str())) {
                ed.importModel(ed.recursos.modelos[i]);
                ImGui::CloseCurrentPopup();
            }
        }
        if (ed.recursos.modelos.empty()) ImGui::TextDisabled("No hay archivos .obj en la carpeta.");
        ImGui::EndChild();
        ImGui::Text("O escribe la ruta completa de un archivo .obj:");
        ImGui::SetNextItemWidth(420.0f);
        ImGui::InputText("##ruta", rutaModelo, sizeof(rutaModelo));
        if (ImGui::Button("Importar ruta", ImVec2(120.0f, 0.0f)) && rutaModelo[0] != '\0') {
            ed.importModel(rutaModelo);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancelar", ImVec2(120.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(centro, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(TITULOS[DLG_CONTROLES], 0, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Viewport");
        ImGui::BulletText("Clic: seleccionar un objeto (clic en el vacío: deseleccionar)");
        ImGui::BulletText("Arrastrar con el botón izquierdo: orbitar la cámara");
        ImGui::BulletText("Botón derecho o Shift + arrastrar: desplazar la cámara");
        ImGui::BulletText("Rueda, botón central, Ctrl + arrastrar o teclas + / -: zoom");
        ImGui::BulletText("Arrastrar un eje del gizmo: mover, rotar o escalar el objeto");
        ImGui::Separator();
        ImGui::Text("Teclado");
        ImGui::BulletText("W mover   E rotar   R escalar   F enfocar   Esc deseleccionar");
        ImGui::BulletText("Supr o X eliminar   Ctrl+D duplicar");
        ImGui::BulletText("P perspectiva / ortogonal   Z sólido / wireframe");
        ImGui::BulletText("T texturas   L iluminación   G cuadrícula");
        ImGui::BulletText("1 frente   3 lado   7 arriba   0 restablecer cámara   Flechas orbitar");
        ImGui::BulletText("Espacio reproducir / pausar la animación");
        ImGui::BulletText("Ctrl+N nueva   Ctrl+O abrir   Ctrl+S guardar");
        ImGui::Separator();
        ImGui::Text("Jerarquía");
        ImGui::BulletText("En el panel Escena arrastra un objeto sobre otro para hacerlo su hijo");
        ImGui::BulletText("Suéltalo en el espacio vacío para quitarle el padre");
        ImGui::Separator();
        if (ImGui::Button("Cerrar", ImVec2(120.0f, 0.0f))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}
