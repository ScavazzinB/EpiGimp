#include "epigimp/MainWindow.hpp"
#include <gdkmm/general.h>
#include <glibmm/miscutils.h>
#include <iostream>
#include <cmath>

namespace EpiGimp {

MainWindow::MainWindow()
    : m_main_box(Gtk::ORIENTATION_VERTICAL),
      m_content_box(Gtk::ORIENTATION_HORIZONTAL),
      m_toolbar_box(Gtk::ORIENTATION_VERTICAL, 2),
      m_color_panel(Gtk::ORIENTATION_VERTICAL, 5),
      m_statusbar(Gtk::ORIENTATION_HORIZONTAL, 10)
{
    // Initialize command stack for undo/redo
    m_commandStack = std::make_unique<CommandStack>(50);
    // Window configuration
    set_default_size(1024, 768);
    update_title();

    // Initialize colors
    m_primary_color.set_rgba(0.0, 0.0, 0.0, 1.0);      // Black
    m_secondary_color.set_rgba(1.0, 1.0, 1.0, 1.0);    // White

    // Setup undo/redo callbacks
    m_commandStack->setOnStackChanged([this]() {
        update_undo_redo_menu();
    });
    m_commandStack->setOnCleanChanged([this](bool clean) {
        update_title();
        (void)clean;
    });

    // Setup UI components
    setup_menu();
    setup_toolbar();
    setup_color_panel();
    setup_layout();
    setup_statusbar();

    // Connect canvas signals
    m_canvas.signal_draw().connect(
        sigc::mem_fun(*this, &MainWindow::on_canvas_draw));

    // Enable events for canvas
    m_canvas.add_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK |
                        Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);

    m_canvas.signal_scroll_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_canvas_scroll));
    m_canvas.signal_button_press_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_canvas_button_press));
    m_canvas.signal_button_release_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_canvas_button_release));
    m_canvas.signal_motion_notify_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_canvas_motion));

    // Keyboard events on window
    signal_key_press_event().connect(
        sigc::mem_fun(*this, &MainWindow::on_key_press));

    // Show all widgets
    show_all_children();

    // Initialize undo/redo menu state
    update_undo_redo_menu();
}

void MainWindow::setup_menu()
{
    // === File Menu ===
    m_menu_file.set_label("_Fichier");
    m_menu_file.set_use_underline(true);

    m_menu_file_new.set_label("_Nouveau");
    m_menu_file_new.set_use_underline(true);
    m_menu_file_new.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_file_new));

    m_menu_file_open.set_label("_Ouvrir...");
    m_menu_file_open.set_use_underline(true);
    m_menu_file_open.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_file_open));

    m_menu_file_save.set_label("_Enregistrer");
    m_menu_file_save.set_use_underline(true);
    m_menu_file_save.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_file_save));

    m_menu_file_save_as.set_label("Enregistrer _sous...");
    m_menu_file_save_as.set_use_underline(true);
    m_menu_file_save_as.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_file_save_as));

    m_menu_file_quit.set_label("_Quitter");
    m_menu_file_quit.set_use_underline(true);
    m_menu_file_quit.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_file_quit));

    m_submenu_file.append(m_menu_file_new);
    m_submenu_file.append(m_menu_file_open);
    m_submenu_file.append(m_menu_file_save);
    m_submenu_file.append(m_menu_file_save_as);
    m_submenu_file.append(m_menu_file_separator);
    m_submenu_file.append(m_menu_file_quit);
    m_menu_file.set_submenu(m_submenu_file);

    // === Edit Menu ===
    m_menu_edit.set_label("É_dition");
    m_menu_edit.set_use_underline(true);

    m_menu_edit_undo.set_label("_Annuler");
    m_menu_edit_undo.set_use_underline(true);
    m_menu_edit_undo.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_edit_undo));

    m_menu_edit_redo.set_label("_Rétablir");
    m_menu_edit_redo.set_use_underline(true);
    m_menu_edit_redo.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_edit_redo));

    m_submenu_edit.append(m_menu_edit_undo);
    m_submenu_edit.append(m_menu_edit_redo);
    m_submenu_edit.append(m_menu_edit_separator);

    m_menu_edit_copy.set_label("_Copier");
    m_menu_edit_copy.set_use_underline(true);
    m_menu_edit_copy.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_edit_copy));
    m_submenu_edit.append(m_menu_edit_copy);

    m_menu_edit_cut.set_label("Co_uper");
    m_menu_edit_cut.set_use_underline(true);
    m_menu_edit_cut.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_edit_cut));
    m_submenu_edit.append(m_menu_edit_cut);

    m_menu_edit_paste.set_label("C_oller");
    m_menu_edit_paste.set_use_underline(true);
    m_menu_edit_paste.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_edit_paste));
    m_submenu_edit.append(m_menu_edit_paste);

    m_menu_edit_delete.set_label("_Supprimer");
    m_menu_edit_delete.set_use_underline(true);
    m_menu_edit_delete.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_edit_delete));
    m_submenu_edit.append(m_menu_edit_delete);

    m_menu_edit.set_submenu(m_submenu_edit);

    // === Image Menu ===
    m_menu_image.set_label("_Image");
    m_menu_image.set_use_underline(true);

    m_menu_image_rotate.set_label("_Rotation...");
    m_menu_image_rotate.set_use_underline(true);
    m_menu_image_rotate.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_image_rotate));

    m_menu_image_flip_h.set_label("Retourner _horizontalement");
    m_menu_image_flip_h.set_use_underline(true);
    m_menu_image_flip_h.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_image_flip_h));

    m_menu_image_flip_v.set_label("Retourner _verticalement");
    m_menu_image_flip_v.set_use_underline(true);
    m_menu_image_flip_v.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_image_flip_v));

    m_menu_image_scale.set_label("Redimensionner...");
    m_menu_image_scale.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_image_scale));

    m_submenu_image.append(m_menu_image_rotate);
    m_submenu_image.append(m_menu_image_flip_h);
    m_submenu_image.append(m_menu_image_flip_v);
    m_submenu_image.append(m_menu_image_separator);
    m_submenu_image.append(m_menu_image_scale);

    m_menu_image_crop.set_label("_Recadrer...");
    m_menu_image_crop.set_use_underline(true);
    m_menu_image_crop.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_image_crop));
    m_submenu_image.append(m_menu_image_crop);

    m_menu_image.set_submenu(m_submenu_image);

    // === Filters Menu ===
    m_menu_filters.set_label("Filt_res");
    m_menu_filters.set_use_underline(true);

    m_menu_filter_blur.set_label("_Flou...");
    m_menu_filter_blur.set_use_underline(true);
    m_menu_filter_blur.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_filter_blur));
    m_submenu_filters.append(m_menu_filter_blur);

    m_menu_filter_sharpen.set_label("_Netteté");
    m_menu_filter_sharpen.set_use_underline(true);
    m_menu_filter_sharpen.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_filter_sharpen));
    m_submenu_filters.append(m_menu_filter_sharpen);

    m_submenu_filters.append(m_menu_filter_separator);

    m_menu_filter_grayscale.set_label("Niveaux de _gris");
    m_menu_filter_grayscale.set_use_underline(true);
    m_menu_filter_grayscale.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_filter_grayscale));
    m_submenu_filters.append(m_menu_filter_grayscale);

    m_menu_filter_invert.set_label("_Inverser les couleurs");
    m_menu_filter_invert.set_use_underline(true);
    m_menu_filter_invert.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_filter_invert));
    m_submenu_filters.append(m_menu_filter_invert);

    m_menu_filter_sepia.set_label("_Sépia");
    m_menu_filter_sepia.set_use_underline(true);
    m_menu_filter_sepia.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_filter_sepia));
    m_submenu_filters.append(m_menu_filter_sepia);

    m_menu_filters.set_submenu(m_submenu_filters);

    // === Adjustments Menu ===
    m_menu_adjustments.set_label("_Ajustements");
    m_menu_adjustments.set_use_underline(true);

    m_menu_adjust_brightness_contrast.set_label("_Luminosité / Contraste...");
    m_menu_adjust_brightness_contrast.set_use_underline(true);
    m_menu_adjust_brightness_contrast.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_image_brightness_contrast));
    m_submenu_adjustments.append(m_menu_adjust_brightness_contrast);

    m_menu_adjust_gamma.set_label("_Gamma...");
    m_menu_adjust_gamma.set_use_underline(true);
    m_menu_adjust_gamma.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_adjust_gamma));
    m_submenu_adjustments.append(m_menu_adjust_gamma);

    m_menu_adjust_levels.set_label("Ni_veaux...");
    m_menu_adjust_levels.set_use_underline(true);
    m_menu_adjust_levels.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_adjust_levels));
    m_submenu_adjustments.append(m_menu_adjust_levels);

    m_submenu_adjustments.append(m_menu_adjust_separator);

    m_menu_adjust_hue_saturation.set_label("_Teinte / Saturation...");
    m_menu_adjust_hue_saturation.set_use_underline(true);
    m_menu_adjust_hue_saturation.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_adjust_hue_saturation));
    m_submenu_adjustments.append(m_menu_adjust_hue_saturation);

    m_menu_adjust_exposure.set_label("_Exposition...");
    m_menu_adjust_exposure.set_use_underline(true);
    m_menu_adjust_exposure.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_adjust_exposure));
    m_submenu_adjustments.append(m_menu_adjust_exposure);

    m_menu_adjust_temperature.set_label("Temp_érature...");
    m_menu_adjust_temperature.set_use_underline(true);
    m_menu_adjust_temperature.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_adjust_temperature));
    m_submenu_adjustments.append(m_menu_adjust_temperature);

    m_menu_adjustments.set_submenu(m_submenu_adjustments);

    // === Help Menu ===
    m_menu_help.set_label("_Aide");
    m_menu_help.set_use_underline(true);

    m_menu_help_about.set_label("À _propos");
    m_menu_help_about.set_use_underline(true);
    m_menu_help_about.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_help_about));

    m_submenu_help.append(m_menu_help_about);
    m_menu_help.set_submenu(m_submenu_help);

    // Add menus to menu bar
    m_menu_bar.append(m_menu_file);
    m_menu_bar.append(m_menu_edit);
    m_menu_bar.append(m_menu_image);
    m_menu_bar.append(m_menu_adjustments);
    m_menu_bar.append(m_menu_filters);
    m_menu_bar.append(m_menu_help);
}

void MainWindow::setup_toolbar()
{
    m_toolbar_box.set_margin_start(2);
    m_toolbar_box.set_margin_end(2);
    m_toolbar_box.set_margin_top(5);

    // Brush tool button
    m_tool_brush.set_label("🖌");
    m_tool_brush.set_tooltip_text(
        "Pinceau (B)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Dessiner avec la couleur primaire.\n\n"
        "• Clic + glisser pour peindre\n"
        "• [ ] pour ajuster la taille\n"
        "• X pour échanger les couleurs");
    m_tool_brush.set_size_request(40, 40);
    m_tool_brush.signal_toggled().connect([this]() {
        if (m_tool_brush.get_active()) {
            select_tool(Tool::BRUSH);
        } else if (m_current_tool == Tool::BRUSH) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_brush, Gtk::PACK_SHRINK);

    // Eraser tool button
    m_tool_eraser.set_label("🧹");
    m_tool_eraser.set_tooltip_text(
        "Gomme (E)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Effacer avec la couleur secondaire.\n\n"
        "• Clic + glisser pour effacer\n"
        "• [ ] pour ajuster la taille\n"
        "• X pour échanger les couleurs");
    m_tool_eraser.set_size_request(40, 40);
    m_tool_eraser.signal_toggled().connect([this]() {
        if (m_tool_eraser.get_active()) {
            select_tool(Tool::ERASER);
        } else if (m_current_tool == Tool::ERASER) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_eraser, Gtk::PACK_SHRINK);

    // Pipette tool button
    m_tool_pipette.set_label("🎨");
    m_tool_pipette.set_tooltip_text(
        "Pipette (I)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Récupérer une couleur de l'image.\n\n"
        "• Clic sur l'image pour prélever\n"
        "• La couleur devient la couleur primaire\n"
        "• RGB affiché dans la barre de statut");
    m_tool_pipette.set_size_request(40, 40);
    m_tool_pipette.signal_toggled().connect([this]() {
        if (m_tool_pipette.get_active()) {
            select_tool(Tool::PIPETTE);
        } else if (m_current_tool == Tool::PIPETTE) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_pipette, Gtk::PACK_SHRINK);

    // Selection tool button
    m_tool_selection.set_label("▢");
    m_tool_selection.set_tooltip_text(
        "Sélection rectangulaire (S)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Sélectionner une zone de l'image.\n\n"
        "• Clic + glisser pour sélectionner\n"
        "• Ctrl+C copier, Ctrl+X couper\n"
        "• Ctrl+V coller, Delete supprimer\n"
        "• Escape pour désélectionner");
    m_tool_selection.set_size_request(40, 40);
    m_tool_selection.signal_toggled().connect([this]() {
        if (m_tool_selection.get_active()) {
            select_tool(Tool::SELECTION);
        } else if (m_current_tool == Tool::SELECTION) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_selection, Gtk::PACK_SHRINK);

    // Line tool button
    m_tool_line.set_label("╱");
    m_tool_line.set_tooltip_text(
        "Ligne (L)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Tracer une ligne droite.\n\n"
        "• Clic + glisser pour tracer\n"
        "• Utilise la couleur primaire\n"
        "• [ ] pour ajuster l'épaisseur");
    m_tool_line.set_size_request(40, 40);
    m_tool_line.signal_toggled().connect([this]() {
        if (m_tool_line.get_active()) {
            select_tool(Tool::LINE);
        } else if (m_current_tool == Tool::LINE) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_line, Gtk::PACK_SHRINK);

    // Rectangle tool button
    m_tool_rectangle.set_label("□");
    m_tool_rectangle.set_tooltip_text(
        "Rectangle (R)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Dessiner un rectangle.\n\n"
        "• Clic + glisser pour dessiner\n"
        "• F pour alterner contour/rempli\n"
        "• [ ] pour ajuster l'épaisseur du trait");
    m_tool_rectangle.set_size_request(40, 40);
    m_tool_rectangle.signal_toggled().connect([this]() {
        if (m_tool_rectangle.get_active()) {
            select_tool(Tool::RECTANGLE);
        } else if (m_current_tool == Tool::RECTANGLE) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_rectangle, Gtk::PACK_SHRINK);

    // Circle tool button
    m_tool_circle.set_label("○");
    m_tool_circle.set_tooltip_text(
        "Cercle (C)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Dessiner un cercle.\n\n"
        "• Clic = centre, glisser = rayon\n"
        "• F pour alterner contour/rempli\n"
        "• [ ] pour ajuster l'épaisseur du trait");
    m_tool_circle.set_size_request(40, 40);
    m_tool_circle.signal_toggled().connect([this]() {
        if (m_tool_circle.get_active()) {
            select_tool(Tool::CIRCLE);
        } else if (m_current_tool == Tool::CIRCLE) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_circle, Gtk::PACK_SHRINK);

    // Crop tool button
    m_tool_crop.set_label("⛶");
    m_tool_crop.set_tooltip_text(
        "Recadrer (K)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Recadrer l'image.\n\n"
        "• Clic + glisser pour définir la zone\n"
        "• Relâcher pour confirmer\n"
        "• Ctrl+Z pour annuler après");
    m_tool_crop.set_size_request(40, 40);
    m_tool_crop.signal_toggled().connect([this]() {
        if (m_tool_crop.get_active()) {
            select_tool(Tool::CROP);
        } else if (m_current_tool == Tool::CROP) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_crop, Gtk::PACK_SHRINK);

    // Shape fill mode toggle
    m_shape_fill_toggle.set_label("◨");
    m_shape_fill_toggle.set_tooltip_text(
        "Mode remplissage (F)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Basculer entre contour et rempli.\n\n"
        "• Désactivé = contour uniquement\n"
        "• Activé = forme remplie\n"
        "• Affecte Rectangle et Cercle");
    m_shape_fill_toggle.set_size_request(40, 40);
    m_shape_fill_toggle.signal_toggled().connect([this]() {
        if (m_shape_fill_toggle.get_active()) {
            m_shape_mode = ShapeMode::FILLED;
            m_status_label.set_text("Mode: Formes remplies");
        } else {
            m_shape_mode = ShapeMode::OUTLINE;
            m_status_label.set_text("Mode: Contour uniquement");
        }
    });
    m_toolbar_box.pack_start(m_shape_fill_toggle, Gtk::PACK_SHRINK);

    // Brush size label
    m_brush_size_label.set_text("Taille: 10");
    m_brush_size_label.set_margin_top(15);
    m_toolbar_box.pack_start(m_brush_size_label, Gtk::PACK_SHRINK);

    // Brush size slider
    m_brush_size_scale.set_range(1, 100);
    m_brush_size_scale.set_value(10);
    m_brush_size_scale.set_size_request(60, -1);
    m_brush_size_scale.set_draw_value(false);
    m_brush_size_scale.signal_value_changed().connect([this]() {
        m_brush_size = static_cast<int>(m_brush_size_scale.get_value());
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Taille: %d", m_brush_size);
        m_brush_size_label.set_text(buffer);
    });
    m_toolbar_box.pack_start(m_brush_size_scale, Gtk::PACK_SHRINK);

    // Add color panel below tools
    m_toolbar_box.pack_start(m_color_panel, Gtk::PACK_SHRINK);
}

void MainWindow::setup_color_panel()
{
    m_color_panel.set_margin_top(20);

    // Color display area (shows primary and secondary colors)
    m_color_display.set_size_request(50, 50);
    m_color_display.signal_draw().connect([this](const Cairo::RefPtr<Cairo::Context>& cr) {
        // Draw secondary color (background, offset)
        cr->set_source_rgb(m_secondary_color.get_red(),
                          m_secondary_color.get_green(),
                          m_secondary_color.get_blue());
        cr->rectangle(15, 15, 30, 30);
        cr->fill();
        cr->set_source_rgb(0.3, 0.3, 0.3);
        cr->rectangle(15, 15, 30, 30);
        cr->stroke();

        // Draw primary color (foreground, on top)
        cr->set_source_rgb(m_primary_color.get_red(),
                          m_primary_color.get_green(),
                          m_primary_color.get_blue());
        cr->rectangle(5, 5, 30, 30);
        cr->fill();
        cr->set_source_rgb(0.3, 0.3, 0.3);
        cr->rectangle(5, 5, 30, 30);
        cr->stroke();

        return true;
    });

    m_color_panel.pack_start(m_color_display, Gtk::PACK_SHRINK);

    // Swap colors button
    m_swap_colors_btn.set_label("⇄");
    m_swap_colors_btn.set_tooltip_text(
        "Échanger les couleurs (X)\n"
        "━━━━━━━━━━━━━━━━━━━━\n"
        "Inverser couleur primaire et secondaire.\n\n"
        "• Primaire = pinceau, formes\n"
        "• Secondaire = gomme, suppression");
    m_swap_colors_btn.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::swap_colors));

    m_color_panel.pack_start(m_swap_colors_btn, Gtk::PACK_SHRINK);
}

void MainWindow::setup_layout()
{
    // Add menu bar at the top
    m_main_box.pack_start(m_menu_bar, Gtk::PACK_SHRINK);

    // Add toolbar to content box
    m_content_box.pack_start(m_toolbar_box, Gtk::PACK_SHRINK);

    // Add canvas area (expands to fill remaining space)
    m_canvas.set_hexpand(true);
    m_canvas.set_vexpand(true);
    m_canvas.set_can_focus(true);
    m_content_box.pack_start(m_canvas, Gtk::PACK_EXPAND_WIDGET);

    // Add content box to main box
    m_main_box.pack_start(m_content_box, Gtk::PACK_EXPAND_WIDGET);

    // Add main box to window
    add(m_main_box);
}

void MainWindow::setup_statusbar()
{
    m_statusbar.set_margin_start(5);
    m_statusbar.set_margin_end(5);
    m_statusbar.set_margin_top(2);
    m_statusbar.set_margin_bottom(2);

    // Zoom label on the right
    update_zoom_label();
    m_statusbar.pack_end(m_zoom_label, Gtk::PACK_SHRINK);

    // Color info label
    m_color_label.set_margin_end(20);
    m_statusbar.pack_end(m_color_label, Gtk::PACK_SHRINK);

    // Status label on the left
    m_status_label.set_text("Prêt");
    m_status_label.set_halign(Gtk::ALIGN_START);
    m_statusbar.pack_start(m_status_label, Gtk::PACK_SHRINK);

    m_main_box.pack_end(m_statusbar, Gtk::PACK_SHRINK);
}

void MainWindow::update_color_display()
{
    m_color_display.queue_draw();

    // Update color label in statusbar
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "RGB(%d, %d, %d)",
             static_cast<int>(m_primary_color.get_red() * 255),
             static_cast<int>(m_primary_color.get_green() * 255),
             static_cast<int>(m_primary_color.get_blue() * 255));
    m_color_label.set_text(buffer);
}

void MainWindow::update_zoom_label()
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "Zoom: %.0f%%", m_zoom_level * 100.0);
    m_zoom_label.set_text(buffer);
}

// === Signal Handlers ===

void MainWindow::on_menu_file_new()
{
    std::cout << "Fichier > Nouveau" << std::endl;
    // TODO: Implement new file functionality
}

void MainWindow::on_menu_file_open()
{
    Gtk::FileChooserDialog dialog(*this, "Ouvrir une image",
        Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Ouvrir", Gtk::RESPONSE_OK);

    // Add image file filters
    auto filter_images = Gtk::FileFilter::create();
    filter_images->set_name("Images (PNG, JPEG)");
    filter_images->add_mime_type("image/png");
    filter_images->add_mime_type("image/jpeg");
    filter_images->add_pattern("*.png");
    filter_images->add_pattern("*.jpg");
    filter_images->add_pattern("*.jpeg");
    dialog.add_filter(filter_images);

    auto filter_all = Gtk::FileFilter::create();
    filter_all->set_name("Tous les fichiers");
    filter_all->add_pattern("*");
    dialog.add_filter(filter_all);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        std::string filepath = dialog.get_filename();
        if (load_image(filepath)) {
            m_current_filepath = filepath;
            update_title();
            m_canvas.queue_draw();
        }
    }
}

void MainWindow::on_menu_file_save()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à enregistrer");
        return;
    }

    // If no file path, use Save As dialog
    if (m_current_filepath.empty()) {
        on_menu_file_save_as();
        return;
    }

    // Determine format from extension
    std::string ext = m_current_filepath.substr(m_current_filepath.find_last_of('.') + 1);
    std::string format = "png";  // default
    if (ext == "jpg" || ext == "jpeg" || ext == "JPG" || ext == "JPEG") {
        format = "jpeg";
    } else if (ext == "bmp" || ext == "BMP") {
        format = "bmp";
    }

    try {
        m_image->save(m_current_filepath, format);
        m_commandStack->setClean();
        update_title();
        m_status_label.set_text("Image enregistrée: " + Glib::path_get_basename(m_current_filepath));
        std::cout << "Image enregistrée: " << m_current_filepath << std::endl;
    } catch (const Glib::Error& e) {
        Gtk::MessageDialog dialog(*this, "Erreur d'enregistrement",
            false, Gtk::MESSAGE_ERROR);
        dialog.set_secondary_text(e.what());
        dialog.run();
    }
}

void MainWindow::on_menu_file_save_as()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à enregistrer");
        return;
    }

    Gtk::FileChooserDialog dialog(*this, "Enregistrer l'image sous",
        Gtk::FILE_CHOOSER_ACTION_SAVE);

    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Enregistrer", Gtk::RESPONSE_OK);
    dialog.set_do_overwrite_confirmation(true);

    // Add file filters
    auto filter_png = Gtk::FileFilter::create();
    filter_png->set_name("PNG (*.png)");
    filter_png->add_mime_type("image/png");
    filter_png->add_pattern("*.png");
    dialog.add_filter(filter_png);

    auto filter_jpeg = Gtk::FileFilter::create();
    filter_jpeg->set_name("JPEG (*.jpg, *.jpeg)");
    filter_jpeg->add_mime_type("image/jpeg");
    filter_jpeg->add_pattern("*.jpg");
    filter_jpeg->add_pattern("*.jpeg");
    dialog.add_filter(filter_jpeg);

    auto filter_bmp = Gtk::FileFilter::create();
    filter_bmp->set_name("BMP (*.bmp)");
    filter_bmp->add_mime_type("image/bmp");
    filter_bmp->add_pattern("*.bmp");
    dialog.add_filter(filter_bmp);

    // Set default name if we have a current file
    if (!m_current_filepath.empty()) {
        dialog.set_current_name(Glib::path_get_basename(m_current_filepath));
    } else {
        dialog.set_current_name("image.png");
    }

    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        std::string filepath = dialog.get_filename();

        // Determine format from filter or extension
        std::string format = "png";
        auto selected_filter = dialog.get_filter();
        if (selected_filter == filter_jpeg) {
            format = "jpeg";
            // Add extension if missing
            if (filepath.find(".jpg") == std::string::npos &&
                filepath.find(".jpeg") == std::string::npos) {
                filepath += ".jpg";
            }
        } else if (selected_filter == filter_bmp) {
            format = "bmp";
            if (filepath.find(".bmp") == std::string::npos) {
                filepath += ".bmp";
            }
        } else {
            // PNG
            if (filepath.find(".png") == std::string::npos) {
                filepath += ".png";
            }
        }

        try {
            m_image->save(filepath, format);
            m_current_filepath = filepath;
            m_commandStack->setClean();
            update_title();
            m_status_label.set_text("Image enregistrée: " + Glib::path_get_basename(filepath));
            std::cout << "Image enregistrée: " << filepath << std::endl;
        } catch (const Glib::Error& e) {
            Gtk::MessageDialog error_dialog(*this, "Erreur d'enregistrement",
                false, Gtk::MESSAGE_ERROR);
            error_dialog.set_secondary_text(e.what());
            error_dialog.run();
        }
    }
}

void MainWindow::on_menu_file_quit()
{
    // Close the window, which will trigger application quit
    close();
}

void MainWindow::on_menu_edit_undo()
{
    if (m_commandStack->canUndo()) {
        std::string desc = m_commandStack->undoText();
        m_commandStack->undo();
        m_canvas.queue_draw();
        m_status_label.set_text("Annulé: " + desc);
        std::cout << "Annulé: " << desc << std::endl;
    } else {
        m_status_label.set_text("Rien à annuler");
    }
}

void MainWindow::on_menu_edit_redo()
{
    if (m_commandStack->canRedo()) {
        std::string desc = m_commandStack->redoText();
        m_commandStack->redo();
        m_canvas.queue_draw();
        m_status_label.set_text("Rétabli: " + desc);
        std::cout << "Rétabli: " << desc << std::endl;
    } else {
        m_status_label.set_text("Rien à rétablir");
    }
}

void MainWindow::update_undo_redo_menu()
{
    // Update menu item labels with action descriptions
    if (m_commandStack->canUndo()) {
        m_menu_edit_undo.set_label("_Annuler " + m_commandStack->undoText());
        m_menu_edit_undo.set_sensitive(true);
    } else {
        m_menu_edit_undo.set_label("_Annuler");
        m_menu_edit_undo.set_sensitive(false);
    }

    if (m_commandStack->canRedo()) {
        m_menu_edit_redo.set_label("_Rétablir " + m_commandStack->redoText());
        m_menu_edit_redo.set_sensitive(true);
    } else {
        m_menu_edit_redo.set_label("_Rétablir");
        m_menu_edit_redo.set_sensitive(false);
    }
}

void MainWindow::restore_image(Glib::RefPtr<Gdk::Pixbuf> image)
{
    m_image = image;
    m_canvas.queue_draw();
}

void MainWindow::on_menu_help_about()
{
    Gtk::AboutDialog dialog;
    dialog.set_transient_for(*this);
    dialog.set_program_name("EpiGimp");
    dialog.set_version("1.0.0");
    dialog.set_comments("Éditeur d'images simple inspiré de GIMP");
    dialog.set_copyright("© 2025 EpiGimp");
    dialog.set_license_type(Gtk::LICENSE_MIT_X11);

    dialog.run();
}

bool MainWindow::load_image(const std::string& filepath)
{
    try {
        m_image = Gdk::Pixbuf::create_from_file(filepath);
        std::cout << "Image chargée: " << filepath
                  << " (" << m_image->get_width() << "x"
                  << m_image->get_height() << ")" << std::endl;

        // Reset zoom and pan
        m_zoom_level = 1.0;
        m_pan_x = 0.0;
        m_pan_y = 0.0;
        update_zoom_label();

        // Clear undo/redo history for new image
        m_commandStack->clear();
        m_commandStack->setClean();

        return true;
    } catch (const Glib::FileError& e) {
        Gtk::MessageDialog dialog(*this, "Erreur de fichier",
            false, Gtk::MESSAGE_ERROR);
        dialog.set_secondary_text(e.what());
        dialog.run();
        return false;
    } catch (const Gdk::PixbufError& e) {
        Gtk::MessageDialog dialog(*this, "Erreur de chargement d'image",
            false, Gtk::MESSAGE_ERROR);
        dialog.set_secondary_text(e.what());
        dialog.run();
        return false;
    }
}

void MainWindow::update_title()
{
    std::string title = "EpiGimp";
    if (!m_current_filepath.empty()) {
        title += " - " + Glib::path_get_basename(m_current_filepath);
    }
    // Add asterisk if there are unsaved changes
    if (m_commandStack && !m_commandStack->isClean()) {
        title += " *";
    }
    set_title(title);
}

bool MainWindow::on_canvas_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Draw dark background
    cr->set_source_rgb(0.2, 0.2, 0.2);
    cr->paint();

    if (!m_image) {
        return true;
    }

    // Get canvas dimensions
    const int canvas_width = m_canvas.get_allocated_width();
    const int canvas_height = m_canvas.get_allocated_height();
    const int img_width = m_image->get_width();
    const int img_height = m_image->get_height();

    // Calculate scaled image dimensions
    double scaled_width = img_width * m_zoom_level;
    double scaled_height = img_height * m_zoom_level;

    // Calculate centered position with pan offset
    double offset_x = (canvas_width - scaled_width) / 2.0 + m_pan_x;
    double offset_y = (canvas_height - scaled_height) / 2.0 + m_pan_y;

    // Draw the image
    cr->save();
    cr->translate(offset_x, offset_y);
    cr->scale(m_zoom_level, m_zoom_level);
    Gdk::Cairo::set_source_pixbuf(cr, m_image, 0, 0);
    cr->paint();
    cr->restore();

    // Draw selection overlay
    if (m_has_selection || m_is_selecting) {
        draw_selection_overlay(cr);
    }

    // Draw shape preview while drawing
    if (m_is_drawing_shape) {
        cr->save();
        cr->set_source_rgba(m_primary_color.get_red(),
                           m_primary_color.get_green(),
                           m_primary_color.get_blue(), 0.7);
        cr->set_line_width(m_brush_size);

        double cx1, cy1, cx2, cy2;
        image_to_canvas_coords(m_shape_start_x, m_shape_start_y, cx1, cy1);
        image_to_canvas_coords(m_shape_end_x, m_shape_end_y, cx2, cy2);

        if (m_current_tool == Tool::LINE) {
            cr->move_to(cx1, cy1);
            cr->line_to(cx2, cy2);
            cr->stroke();
        } else if (m_current_tool == Tool::RECTANGLE) {
            double rx = std::min(cx1, cx2);
            double ry = std::min(cy1, cy2);
            double rw = std::abs(cx2 - cx1);
            double rh = std::abs(cy2 - cy1);
            cr->rectangle(rx, ry, rw, rh);
            if (m_shape_mode == ShapeMode::FILLED) {
                cr->fill();
            } else {
                cr->stroke();
            }
        } else if (m_current_tool == Tool::CIRCLE) {
            double radius = std::sqrt(std::pow(cx2 - cx1, 2) + std::pow(cy2 - cy1, 2));
            cr->arc(cx1, cy1, radius, 0, 2 * M_PI);
            if (m_shape_mode == ShapeMode::FILLED) {
                cr->fill();
            } else {
                cr->stroke();
            }
        }
        cr->restore();
    }

    // Draw crop overlay
    if ((m_current_tool == Tool::CROP && (m_crop_x1 != m_crop_x2 || m_crop_y1 != m_crop_y2)) || m_is_cropping) {
        cr->save();

        double cx1, cy1, cx2, cy2;
        image_to_canvas_coords(m_crop_x1, m_crop_y1, cx1, cy1);
        image_to_canvas_coords(m_crop_x2, m_crop_y2, cx2, cy2);

        double rx = std::min(cx1, cx2);
        double ry = std::min(cy1, cy2);
        double rw = std::abs(cx2 - cx1);
        double rh = std::abs(cy2 - cy1);

        // Darken area outside crop
        cr->set_source_rgba(0, 0, 0, 0.5);

        // Top
        cr->rectangle(offset_x, offset_y, scaled_width, ry - offset_y);
        cr->fill();
        // Bottom
        cr->rectangle(offset_x, ry + rh, scaled_width, offset_y + scaled_height - (ry + rh));
        cr->fill();
        // Left
        cr->rectangle(offset_x, ry, rx - offset_x, rh);
        cr->fill();
        // Right
        cr->rectangle(rx + rw, ry, offset_x + scaled_width - (rx + rw), rh);
        cr->fill();

        // Draw crop rectangle border
        cr->set_source_rgb(1, 1, 1);
        cr->set_line_width(2);
        cr->rectangle(rx, ry, rw, rh);
        cr->stroke();

        // Draw dashed inner line
        std::vector<double> dashes = {5.0, 5.0};
        cr->set_dash(dashes, 0);
        cr->set_source_rgb(0, 0, 0);
        cr->rectangle(rx, ry, rw, rh);
        cr->stroke();

        cr->restore();
    }

    return true;
}

// === Zoom functions ===

void MainWindow::zoom_in()
{
    double new_zoom = m_zoom_level * ZOOM_STEP;
    if (new_zoom <= ZOOM_MAX) {
        m_zoom_level = new_zoom;
        update_zoom_label();
        m_canvas.queue_draw();
    }
}

void MainWindow::zoom_out()
{
    double new_zoom = m_zoom_level / ZOOM_STEP;
    if (new_zoom >= ZOOM_MIN) {
        m_zoom_level = new_zoom;
        update_zoom_label();
        m_canvas.queue_draw();
    }
}

void MainWindow::zoom_fit()
{
    if (!m_image) return;

    const int canvas_width = m_canvas.get_allocated_width();
    const int canvas_height = m_canvas.get_allocated_height();
    const int img_width = m_image->get_width();
    const int img_height = m_image->get_height();

    double scale_x = static_cast<double>(canvas_width) / img_width;
    double scale_y = static_cast<double>(canvas_height) / img_height;
    m_zoom_level = std::min(scale_x, scale_y);
    m_pan_x = 0.0;
    m_pan_y = 0.0;

    update_zoom_label();
    m_canvas.queue_draw();
}

void MainWindow::zoom_100()
{
    m_zoom_level = 1.0;
    m_pan_x = 0.0;
    m_pan_y = 0.0;
    update_zoom_label();
    m_canvas.queue_draw();
}

void MainWindow::clamp_pan()
{
    if (!m_image) return;

    const int canvas_width = m_canvas.get_allocated_width();
    const int canvas_height = m_canvas.get_allocated_height();
    const double img_width = m_image->get_width() * m_zoom_level;
    const double img_height = m_image->get_height() * m_zoom_level;

    // Calculate max pan values to keep image visible
    // Allow panning until only 20% of image remains visible
    const double margin_x = std::min(img_width * 0.8, canvas_width * 0.4);
    const double margin_y = std::min(img_height * 0.8, canvas_height * 0.4);

    double max_pan_x, max_pan_y;

    if (img_width <= canvas_width) {
        // Image smaller than canvas - limit pan to keep centered
        max_pan_x = (canvas_width - img_width) / 2.0;
    } else {
        // Image larger than canvas - allow scrolling but keep edges visible
        max_pan_x = margin_x;
    }

    if (img_height <= canvas_height) {
        max_pan_y = (canvas_height - img_height) / 2.0;
    } else {
        max_pan_y = margin_y;
    }

    // Clamp pan values
    m_pan_x = std::max(-max_pan_x, std::min(max_pan_x, m_pan_x));
    m_pan_y = std::max(-max_pan_y, std::min(max_pan_y, m_pan_y));
}

void MainWindow::select_tool(Tool tool)
{
    m_current_tool = tool;

    // Update button states
    m_tool_pipette.set_active(tool == Tool::PIPETTE);
    m_tool_brush.set_active(tool == Tool::BRUSH);
    m_tool_eraser.set_active(tool == Tool::ERASER);
    m_tool_selection.set_active(tool == Tool::SELECTION);
    m_tool_line.set_active(tool == Tool::LINE);
    m_tool_rectangle.set_active(tool == Tool::RECTANGLE);
    m_tool_circle.set_active(tool == Tool::CIRCLE);
    m_tool_crop.set_active(tool == Tool::CROP);

    // Update cursor based on tool
    if (m_canvas.get_window()) {
        auto display = get_display();
        Glib::RefPtr<Gdk::Cursor> cursor;

        switch (tool) {
            case Tool::PIPETTE:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Pipette: Cliquez sur l'image pour récupérer une couleur");
                break;
            case Tool::BRUSH:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Pinceau: Cliquez et glissez pour dessiner");
                break;
            case Tool::ERASER:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Gomme: Cliquez et glissez pour effacer");
                break;
            case Tool::SELECTION:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Sélection: Cliquez et glissez pour sélectionner une zone");
                break;
            case Tool::LINE:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Ligne: Cliquez et glissez pour tracer une ligne");
                break;
            case Tool::RECTANGLE:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Rectangle: Cliquez et glissez pour dessiner un rectangle");
                break;
            case Tool::CIRCLE:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Cercle: Cliquez et glissez pour dessiner un cercle");
                break;
            case Tool::CROP:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Recadrer: Sélectionnez la zone à conserver, Entrée pour valider");
                break;
            default:
                cursor = Gdk::Cursor::create(display, "default");
                m_status_label.set_text("Prêt");
                break;
        }
        m_canvas.get_window()->set_cursor(cursor);
    }
}

bool MainWindow::pick_color_at(double x, double y)
{
    if (!m_image) return false;

    // Convert canvas coordinates to image coordinates
    const int canvas_width = m_canvas.get_allocated_width();
    const int canvas_height = m_canvas.get_allocated_height();
    const int img_width = m_image->get_width();
    const int img_height = m_image->get_height();

    double scaled_width = img_width * m_zoom_level;
    double scaled_height = img_height * m_zoom_level;

    double offset_x = (canvas_width - scaled_width) / 2.0 + m_pan_x;
    double offset_y = (canvas_height - scaled_height) / 2.0 + m_pan_y;

    // Calculate image pixel coordinates
    int img_x = static_cast<int>((x - offset_x) / m_zoom_level);
    int img_y = static_cast<int>((y - offset_y) / m_zoom_level);

    // Check bounds
    if (img_x < 0 || img_x >= img_width || img_y < 0 || img_y >= img_height) {
        return false;
    }

    // Get pixel data
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    guchar* pixel = pixels + img_y * rowstride + img_x * n_channels;

    double r = pixel[0] / 255.0;
    double g = pixel[1] / 255.0;
    double b = pixel[2] / 255.0;

    m_primary_color.set_rgba(r, g, b, 1.0);
    update_color_display();

    std::cout << "Couleur récupérée: RGB(" << static_cast<int>(pixel[0])
              << ", " << static_cast<int>(pixel[1])
              << ", " << static_cast<int>(pixel[2]) << ")" << std::endl;

    return true;
}

void MainWindow::swap_colors()
{
    std::swap(m_primary_color, m_secondary_color);
    update_color_display();
}

// === Drawing functions ===

void MainWindow::canvas_to_image_coords(double canvas_x, double canvas_y, int& img_x, int& img_y)
{
    if (!m_image) {
        img_x = img_y = -1;
        return;
    }

    const int canvas_width = m_canvas.get_allocated_width();
    const int canvas_height = m_canvas.get_allocated_height();
    const int img_width = m_image->get_width();
    const int img_height = m_image->get_height();

    double scaled_width = img_width * m_zoom_level;
    double scaled_height = img_height * m_zoom_level;

    double offset_x = (canvas_width - scaled_width) / 2.0 + m_pan_x;
    double offset_y = (canvas_height - scaled_height) / 2.0 + m_pan_y;

    img_x = static_cast<int>((canvas_x - offset_x) / m_zoom_level);
    img_y = static_cast<int>((canvas_y - offset_y) / m_zoom_level);
}

void MainWindow::draw_brush_point(int x, int y, bool erase)
{
    if (!m_image) return;

    const int img_width = m_image->get_width();
    const int img_height = m_image->get_height();
    const int n_channels = m_image->get_n_channels();
    const int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    // Get color to draw
    guchar r, g, b;
    if (erase) {
        r = static_cast<guchar>(m_secondary_color.get_red() * 255);
        g = static_cast<guchar>(m_secondary_color.get_green() * 255);
        b = static_cast<guchar>(m_secondary_color.get_blue() * 255);
    } else {
        r = static_cast<guchar>(m_primary_color.get_red() * 255);
        g = static_cast<guchar>(m_primary_color.get_green() * 255);
        b = static_cast<guchar>(m_primary_color.get_blue() * 255);
    }

    int radius = m_brush_size / 2;
    int radius_sq = radius * radius;

    // Draw a filled circle
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius_sq) {
                int px = x + dx;
                int py = y + dy;

                // Check bounds
                if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                    guchar* pixel = pixels + py * rowstride + px * n_channels;
                    pixel[0] = r;
                    pixel[1] = g;
                    pixel[2] = b;
                    if (n_channels == 4) {
                        pixel[3] = 255;  // Full opacity
                    }
                }
            }
        }
    }
}

void MainWindow::draw_brush_stroke(int x1, int y1, int x2, int y2, bool erase)
{
    // Bresenham's line algorithm with brush at each point
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    // Calculate spacing based on brush size (draw every few pixels)
    int spacing = std::max(1, m_brush_size / 4);
    int step_count = 0;

    while (true) {
        if (step_count % spacing == 0) {
            draw_brush_point(x1, y1, erase);
        }
        step_count++;

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void MainWindow::start_drawing(double canvas_x, double canvas_y)
{
    if (!m_image) return;

    // Save image state for undo
    m_image_before_stroke = m_image->copy();

    int img_x, img_y;
    canvas_to_image_coords(canvas_x, canvas_y, img_x, img_y);

    if (img_x < 0 || img_y < 0 ||
        img_x >= m_image->get_width() || img_y >= m_image->get_height()) {
        return;
    }

    m_is_drawing = true;
    m_last_draw_x = img_x;
    m_last_draw_y = img_y;

    // Draw initial point
    draw_brush_point(img_x, img_y, m_current_tool == Tool::ERASER);
    m_canvas.queue_draw();
}

void MainWindow::continue_drawing(double canvas_x, double canvas_y)
{
    if (!m_is_drawing || !m_image) return;

    int img_x, img_y;
    canvas_to_image_coords(canvas_x, canvas_y, img_x, img_y);

    // Clamp to image bounds
    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    // Draw line from last point to current point
    draw_brush_stroke(m_last_draw_x, m_last_draw_y, img_x, img_y,
                      m_current_tool == Tool::ERASER);

    m_last_draw_x = img_x;
    m_last_draw_y = img_y;

    m_canvas.queue_draw();
}

void MainWindow::finish_drawing()
{
    if (!m_is_drawing || !m_image) return;

    m_is_drawing = false;

    // Create undo command
    if (m_image_before_stroke) {
        std::string desc = (m_current_tool == Tool::ERASER) ? "Gomme" : "Pinceau";
        auto cmd = std::make_unique<ImageCommand>(
            desc,
            m_image_before_stroke,
            m_image->copy(),
            [this](Glib::RefPtr<Gdk::Pixbuf> img) {
                restore_image(img);
            }
        );

        // Push without executing (already drawn)
        m_commandStack->pushExecuted(std::move(cmd));
        m_image_before_stroke.reset();
    }
}

// === Event handlers ===

bool MainWindow::on_canvas_scroll(GdkEventScroll* event)
{
    if (!m_image) return false;

    // Get mouse position relative to canvas
    double mouse_x = event->x;
    double mouse_y = event->y;

    double old_zoom = m_zoom_level;

    if (event->direction == GDK_SCROLL_UP) {
        double new_zoom = m_zoom_level * ZOOM_STEP;
        if (new_zoom <= ZOOM_MAX) {
            m_zoom_level = new_zoom;
        }
    } else if (event->direction == GDK_SCROLL_DOWN) {
        double new_zoom = m_zoom_level / ZOOM_STEP;
        if (new_zoom >= ZOOM_MIN) {
            m_zoom_level = new_zoom;
        }
    } else if (event->direction == GDK_SCROLL_SMOOTH) {
        double delta_y = event->delta_y;
        if (delta_y < 0) {
            double new_zoom = m_zoom_level * ZOOM_STEP;
            if (new_zoom <= ZOOM_MAX) {
                m_zoom_level = new_zoom;
            }
        } else if (delta_y > 0) {
            double new_zoom = m_zoom_level / ZOOM_STEP;
            if (new_zoom >= ZOOM_MIN) {
                m_zoom_level = new_zoom;
            }
        }
    }

    if (old_zoom != m_zoom_level) {
        // Adjust pan to zoom towards mouse position
        double zoom_ratio = m_zoom_level / old_zoom;
        m_pan_x = mouse_x - (mouse_x - m_pan_x) * zoom_ratio;
        m_pan_y = mouse_y - (mouse_y - m_pan_y) * zoom_ratio;

        clamp_pan();
        update_zoom_label();
        m_canvas.queue_draw();
    }

    return true;
}

bool MainWindow::on_canvas_button_press(GdkEventButton* event)
{
    // Left click - check for tool actions first
    if (event->button == 1) {
        if (m_current_tool == Tool::PIPETTE) {
            pick_color_at(event->x, event->y);
            return true;
        }

        if (m_current_tool == Tool::BRUSH || m_current_tool == Tool::ERASER) {
            start_drawing(event->x, event->y);
            return true;
        }

        if (m_current_tool == Tool::SELECTION) {
            start_selection(event->x, event->y);
            return true;
        }

        if (m_current_tool == Tool::LINE || m_current_tool == Tool::RECTANGLE ||
            m_current_tool == Tool::CIRCLE) {
            start_shape(event->x, event->y);
            return true;
        }

        if (m_current_tool == Tool::CROP) {
            start_crop(event->x, event->y);
            return true;
        }

        // No tool active - start panning
        m_is_panning = true;
        m_pan_start_x = event->x;
        m_pan_start_y = event->y;
        m_pan_origin_x = m_pan_x;
        m_pan_origin_y = m_pan_y;

        auto display = get_display();
        auto cursor = Gdk::Cursor::create(display, "grabbing");
        m_canvas.get_window()->set_cursor(cursor);

        return true;
    }

    // Middle button - always panning
    if (event->button == 2) {
        m_is_panning = true;
        m_pan_start_x = event->x;
        m_pan_start_y = event->y;
        m_pan_origin_x = m_pan_x;
        m_pan_origin_y = m_pan_y;

        auto display = get_display();
        auto cursor = Gdk::Cursor::create(display, "grabbing");
        m_canvas.get_window()->set_cursor(cursor);

        return true;
    }

    return false;
}

bool MainWindow::on_canvas_button_release(GdkEventButton* event)
{
    if (event->button == 1) {
        // Finish drawing if we were drawing
        if (m_is_drawing) {
            finish_drawing();
            return true;
        }

        // Finish selection if we were selecting
        if (m_is_selecting) {
            finish_selection();
            return true;
        }

        // Finish shape if we were drawing a shape
        if (m_is_drawing_shape) {
            finish_shape();
            return true;
        }

        // Finish crop selection if we were cropping
        if (m_is_cropping) {
            finish_crop();
            return true;
        }

        if (m_is_panning) {
            m_is_panning = false;

            // Restore cursor based on current tool
            auto display = get_display();
            Glib::RefPtr<Gdk::Cursor> cursor;
            if (m_current_tool != Tool::NONE) {
                cursor = Gdk::Cursor::create(display, "crosshair");
            } else {
                cursor = Gdk::Cursor::create(display, "default");
            }
            m_canvas.get_window()->set_cursor(cursor);
        }
        return true;
    }

    if (event->button == 2) {
        if (m_is_panning) {
            m_is_panning = false;

            // Restore cursor based on current tool
            auto display = get_display();
            Glib::RefPtr<Gdk::Cursor> cursor;
            if (m_current_tool != Tool::NONE) {
                cursor = Gdk::Cursor::create(display, "crosshair");
            } else {
                cursor = Gdk::Cursor::create(display, "default");
            }
            m_canvas.get_window()->set_cursor(cursor);
        }
        return true;
    }
    return false;
}

bool MainWindow::on_canvas_motion(GdkEventMotion* event)
{
    // Handle drawing
    if (m_is_drawing) {
        continue_drawing(event->x, event->y);
        return true;
    }

    // Handle selection
    if (m_is_selecting) {
        continue_selection(event->x, event->y);
        return true;
    }

    // Handle shape drawing
    if (m_is_drawing_shape) {
        continue_shape(event->x, event->y);
        return true;
    }

    // Handle cropping
    if (m_is_cropping) {
        continue_crop(event->x, event->y);
        return true;
    }

    // Handle panning
    if (m_is_panning) {
        double dx = event->x - m_pan_start_x;
        double dy = event->y - m_pan_start_y;
        m_pan_x = m_pan_origin_x + dx;
        m_pan_y = m_pan_origin_y + dy;
        clamp_pan();
        m_canvas.queue_draw();
        return true;
    }
    return false;
}

bool MainWindow::on_key_press(GdkEventKey* event)
{
    // Ctrl+S for save
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        if (event->state & GDK_SHIFT_MASK) {
            on_menu_file_save_as();
        } else {
            on_menu_file_save();
        }
        return true;
    }

    // Ctrl+Z for undo
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_z || event->keyval == GDK_KEY_Z)) {
        on_menu_edit_undo();
        return true;
    }

    // Ctrl+Y or Ctrl+Shift+Z for redo
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_y || event->keyval == GDK_KEY_Y)) {
        on_menu_edit_redo();
        return true;
    }
    if ((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) ==
        (GDK_CONTROL_MASK | GDK_SHIFT_MASK) &&
        (event->keyval == GDK_KEY_z || event->keyval == GDK_KEY_Z)) {
        on_menu_edit_redo();
        return true;
    }

    // Ctrl+Plus or Ctrl+= for zoom in
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_plus || event->keyval == GDK_KEY_equal ||
         event->keyval == GDK_KEY_KP_Add)) {
        zoom_in();
        return true;
    }

    // Ctrl+Minus for zoom out
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_minus || event->keyval == GDK_KEY_KP_Subtract)) {
        zoom_out();
        return true;
    }

    // Ctrl+0 for 100% zoom
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_0 || event->keyval == GDK_KEY_KP_0)) {
        zoom_100();
        return true;
    }

    // Ctrl+9 for fit to window
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_9) {
        zoom_fit();
        return true;
    }

    // B for brush tool
    if (event->keyval == GDK_KEY_b || event->keyval == GDK_KEY_B) {
        if (m_current_tool == Tool::BRUSH) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::BRUSH);
        }
        return true;
    }

    // E for eraser tool
    if (event->keyval == GDK_KEY_e || event->keyval == GDK_KEY_E) {
        if (m_current_tool == Tool::ERASER) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::ERASER);
        }
        return true;
    }

    // I for pipette tool
    if (event->keyval == GDK_KEY_i || event->keyval == GDK_KEY_I) {
        if (m_current_tool == Tool::PIPETTE) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::PIPETTE);
        }
        return true;
    }

    // [ to decrease brush size
    if (event->keyval == GDK_KEY_bracketleft) {
        m_brush_size = std::max(1, m_brush_size - 5);
        m_brush_size_scale.set_value(m_brush_size);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Taille: %d", m_brush_size);
        m_brush_size_label.set_text(buffer);
        return true;
    }

    // ] to increase brush size
    if (event->keyval == GDK_KEY_bracketright) {
        m_brush_size = std::min(100, m_brush_size + 5);
        m_brush_size_scale.set_value(m_brush_size);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Taille: %d", m_brush_size);
        m_brush_size_label.set_text(buffer);
        return true;
    }

    // X to swap colors
    if (event->keyval == GDK_KEY_x || event->keyval == GDK_KEY_X) {
        swap_colors();
        return true;
    }

    // Escape to deselect tool or clear selection
    if (event->keyval == GDK_KEY_Escape) {
        if (m_has_selection) {
            clear_selection();
        } else if (m_is_cropping) {
            m_is_cropping = false;
            m_canvas.queue_draw();
        } else {
            select_tool(Tool::NONE);
        }
        return true;
    }

    // S for selection tool
    if (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S) {
        if (!(event->state & GDK_CONTROL_MASK)) {
            if (m_current_tool == Tool::SELECTION) {
                select_tool(Tool::NONE);
            } else {
                select_tool(Tool::SELECTION);
            }
            return true;
        }
    }

    // L for line tool
    if (event->keyval == GDK_KEY_l || event->keyval == GDK_KEY_L) {
        if (m_current_tool == Tool::LINE) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::LINE);
        }
        return true;
    }

    // R for rectangle tool
    if (event->keyval == GDK_KEY_r || event->keyval == GDK_KEY_R) {
        if (m_current_tool == Tool::RECTANGLE) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::RECTANGLE);
        }
        return true;
    }

    // C for circle tool
    if (event->keyval == GDK_KEY_c || event->keyval == GDK_KEY_C) {
        if (!(event->state & GDK_CONTROL_MASK)) {
            if (m_current_tool == Tool::CIRCLE) {
                select_tool(Tool::NONE);
            } else {
                select_tool(Tool::CIRCLE);
            }
            return true;
        }
    }

    // K for crop tool
    if (event->keyval == GDK_KEY_k || event->keyval == GDK_KEY_K) {
        if (m_current_tool == Tool::CROP) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::CROP);
        }
        return true;
    }

    // F for fill mode toggle
    if (event->keyval == GDK_KEY_f || event->keyval == GDK_KEY_F) {
        m_shape_fill_toggle.set_active(!m_shape_fill_toggle.get_active());
        return true;
    }

    // Ctrl+C for copy
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_c || event->keyval == GDK_KEY_C)) {
        on_menu_edit_copy();
        return true;
    }

    // Ctrl+X for cut
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_x || event->keyval == GDK_KEY_X)) {
        on_menu_edit_cut();
        return true;
    }

    // Ctrl+V for paste
    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_v || event->keyval == GDK_KEY_V)) {
        on_menu_edit_paste();
        return true;
    }

    // Delete key for delete selection
    if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_BackSpace) {
        on_menu_edit_delete();
        return true;
    }

    // Enter to apply crop
    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        if (m_current_tool == Tool::CROP && m_crop_x1 != m_crop_x2 && m_crop_y1 != m_crop_y2) {
            apply_crop();
            return true;
        }
    }

    return false;
}

// === Image transformation handlers ===

// Helper function to rotate a pixbuf by arbitrary angle using Cairo
static Glib::RefPtr<Gdk::Pixbuf> rotate_pixbuf(Glib::RefPtr<Gdk::Pixbuf> src, double angle_degrees)
{
    if (!src) return src;

    double angle_rad = angle_degrees * M_PI / 180.0;
    int src_width = src->get_width();
    int src_height = src->get_height();

    // Calculate new dimensions to fit rotated image
    double cos_a = std::abs(std::cos(angle_rad));
    double sin_a = std::abs(std::sin(angle_rad));
    int new_width = static_cast<int>(src_width * cos_a + src_height * sin_a + 0.5);
    int new_height = static_cast<int>(src_width * sin_a + src_height * cos_a + 0.5);

    // Create Cairo surface
    auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, new_width, new_height);
    auto cr = Cairo::Context::create(surface);

    // Fill with transparency
    cr->set_source_rgba(0, 0, 0, 0);
    cr->paint();

    // Move to center, rotate, then draw image centered
    cr->translate(new_width / 2.0, new_height / 2.0);
    cr->rotate(angle_rad);
    cr->translate(-src_width / 2.0, -src_height / 2.0);

    Gdk::Cairo::set_source_pixbuf(cr, src, 0, 0);
    cr->paint();

    // Convert back to pixbuf
    return Gdk::Pixbuf::create(surface, 0, 0, new_width, new_height);
}

void MainWindow::on_menu_image_rotate()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à transformer");
        return;
    }

    // Save original image for preview
    auto original = m_image->copy();

    // Create dialog
    Gtk::Dialog dialog("Rotation de l'image", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(450, 400);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);
    content->set_margin_bottom(10);

    // Preview area
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 250);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Angle label
    Gtk::Label angle_label("Angle: 0°");
    angle_label.set_margin_top(10);
    content->pack_start(angle_label, Gtk::PACK_SHRINK);

    // Angle slider
    Gtk::HScale angle_scale;
    angle_scale.set_range(-180, 180);
    angle_scale.set_value(0);
    angle_scale.set_increments(1, 15);
    angle_scale.set_draw_value(false);
    content->pack_start(angle_scale, Gtk::PACK_SHRINK);

    // Quick rotation buttons
    Gtk::Box buttons_box(Gtk::ORIENTATION_HORIZONTAL, 5);
    buttons_box.set_halign(Gtk::ALIGN_CENTER);
    buttons_box.set_margin_top(5);

    Gtk::Button btn_m90("-90°");
    Gtk::Button btn_m45("-45°");
    Gtk::Button btn_0("0°");
    Gtk::Button btn_p45("+45°");
    Gtk::Button btn_p90("+90°");

    btn_m90.signal_clicked().connect([&]() { angle_scale.set_value(-90); });
    btn_m45.signal_clicked().connect([&]() { angle_scale.set_value(-45); });
    btn_0.signal_clicked().connect([&]() { angle_scale.set_value(0); });
    btn_p45.signal_clicked().connect([&]() { angle_scale.set_value(45); });
    btn_p90.signal_clicked().connect([&]() { angle_scale.set_value(90); });

    buttons_box.pack_start(btn_m90, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_m45, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_0, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_p45, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_p90, Gtk::PACK_SHRINK);
    content->pack_start(buttons_box, Gtk::PACK_SHRINK);

    // Preview pixbuf (scaled for display)
    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    double current_angle = 0;

    // Function to update preview
    auto update_preview = [&]() {
        current_angle = angle_scale.get_value();

        // Update label
        char label_text[32];
        snprintf(label_text, sizeof(label_text), "Angle: %.0f°", current_angle);
        angle_label.set_text(label_text);

        // Create rotated preview (use smaller version for performance)
        int preview_max = 200;
        double scale = 1.0;
        if (original->get_width() > preview_max || original->get_height() > preview_max) {
            scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
        }

        auto small = original->scale_simple(
            static_cast<int>(original->get_width() * scale),
            static_cast<int>(original->get_height() * scale),
            Gdk::INTERP_BILINEAR);

        preview_pixbuf = rotate_pixbuf(small, current_angle);
        preview_area.queue_draw();
    };

    // Connect slider change
    angle_scale.signal_value_changed().connect(update_preview);

    // Draw preview
    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        // Dark background
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();

        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            int img_w = preview_pixbuf->get_width();
            int img_h = preview_pixbuf->get_height();

            // Center the preview
            double x = (area_w - img_w) / 2.0;
            double y = (area_h - img_h) / 2.0;

            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, x, y);
            cr->paint();
        }
        return true;
    });

    // Initial preview
    update_preview();

    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && current_angle != 0) {
        // Apply rotation to full image
        auto before = original;
        m_image = rotate_pixbuf(original, current_angle);

        // Create undo command
        char desc[64];
        snprintf(desc, sizeof(desc), "Rotation %.0f°", current_angle);
        auto cmd = std::make_unique<ImageCommand>(
            desc,
            before,
            m_image->copy(),
            [this](Glib::RefPtr<Gdk::Pixbuf> img) {
                restore_image(img);
            }
        );
        m_commandStack->pushExecuted(std::move(cmd));

        m_canvas.queue_draw();
        char status[64];
        snprintf(status, sizeof(status), "Rotation de %.0f° appliquée", current_angle);
        m_status_label.set_text(status);
    }
}

void MainWindow::on_menu_image_flip_h()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à transformer");
        return;
    }

    auto original = m_image->copy();

    // Create dialog
    Gtk::Dialog dialog("Retournement horizontal", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 350);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);
    content->set_margin_bottom(10);

    // Preview area
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 220);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Flip slider (0 = normal, 100 = flipped, with smooth transition)
    Gtk::Label flip_label("Normal");
    flip_label.set_margin_top(10);
    content->pack_start(flip_label, Gtk::PACK_SHRINK);

    Gtk::HScale flip_scale;
    flip_scale.set_range(0, 100);
    flip_scale.set_value(0);
    flip_scale.set_increments(1, 10);
    flip_scale.set_draw_value(false);
    content->pack_start(flip_scale, Gtk::PACK_SHRINK);

    // Quick buttons
    Gtk::Box buttons_box(Gtk::ORIENTATION_HORIZONTAL, 10);
    buttons_box.set_halign(Gtk::ALIGN_CENTER);
    Gtk::Button btn_normal("Normal");
    Gtk::Button btn_flip("Retourné");
    btn_normal.signal_clicked().connect([&]() { flip_scale.set_value(0); });
    btn_flip.signal_clicked().connect([&]() { flip_scale.set_value(100); });
    buttons_box.pack_start(btn_normal, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_flip, Gtk::PACK_SHRINK);
    content->pack_start(buttons_box, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    bool is_flipped = false;

    // Create scaled preview
    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_normal = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);
    auto small_flipped = small_normal->flip(true);

    auto update_preview = [&]() {
        double val = flip_scale.get_value();
        is_flipped = val >= 50;

        if (val < 50) {
            flip_label.set_text("Normal");
            preview_pixbuf = small_normal;
        } else {
            flip_label.set_text("Retourné horizontalement");
            preview_pixbuf = small_flipped;
        }
        preview_area.queue_draw();
    };

    flip_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();

        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double x = (area_w - preview_pixbuf->get_width()) / 2.0;
            double y = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, x, y);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && is_flipped) {
        auto before = original;
        m_image = original->flip(true);

        auto cmd = std::make_unique<ImageCommand>(
            "Retournement horizontal",
            before,
            m_image->copy(),
            [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
        );
        m_commandStack->pushExecuted(std::move(cmd));
        m_canvas.queue_draw();
        m_status_label.set_text("Image retournée horizontalement");
    }
}

void MainWindow::on_menu_image_flip_v()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à transformer");
        return;
    }

    auto original = m_image->copy();

    // Create dialog
    Gtk::Dialog dialog("Retournement vertical", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 350);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);
    content->set_margin_bottom(10);

    // Preview area
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 220);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Flip slider
    Gtk::Label flip_label("Normal");
    flip_label.set_margin_top(10);
    content->pack_start(flip_label, Gtk::PACK_SHRINK);

    Gtk::HScale flip_scale;
    flip_scale.set_range(0, 100);
    flip_scale.set_value(0);
    flip_scale.set_increments(1, 10);
    flip_scale.set_draw_value(false);
    content->pack_start(flip_scale, Gtk::PACK_SHRINK);

    // Quick buttons
    Gtk::Box buttons_box(Gtk::ORIENTATION_HORIZONTAL, 10);
    buttons_box.set_halign(Gtk::ALIGN_CENTER);
    Gtk::Button btn_normal("Normal");
    Gtk::Button btn_flip("Retourné");
    btn_normal.signal_clicked().connect([&]() { flip_scale.set_value(0); });
    btn_flip.signal_clicked().connect([&]() { flip_scale.set_value(100); });
    buttons_box.pack_start(btn_normal, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_flip, Gtk::PACK_SHRINK);
    content->pack_start(buttons_box, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    bool is_flipped = false;

    // Create scaled preview
    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_normal = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);
    auto small_flipped = small_normal->flip(false);

    auto update_preview = [&]() {
        double val = flip_scale.get_value();
        is_flipped = val >= 50;

        if (val < 50) {
            flip_label.set_text("Normal");
            preview_pixbuf = small_normal;
        } else {
            flip_label.set_text("Retourné verticalement");
            preview_pixbuf = small_flipped;
        }
        preview_area.queue_draw();
    };

    flip_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();

        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double x = (area_w - preview_pixbuf->get_width()) / 2.0;
            double y = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, x, y);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && is_flipped) {
        auto before = original;
        m_image = original->flip(false);

        auto cmd = std::make_unique<ImageCommand>(
            "Retournement vertical",
            before,
            m_image->copy(),
            [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
        );
        m_commandStack->pushExecuted(std::move(cmd));
        m_canvas.queue_draw();
        m_status_label.set_text("Image retournée verticalement");
    }
}

void MainWindow::on_menu_image_scale()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à redimensionner");
        return;
    }

    auto original = m_image->copy();
    int original_width = original->get_width();
    int original_height = original->get_height();

    // Create dialog
    Gtk::Dialog dialog("Redimensionner l'image", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(450, 450);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);
    content->set_margin_bottom(10);

    // Preview area
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 200);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Info label
    Gtk::Label info_label;
    char info_text[128];
    snprintf(info_text, sizeof(info_text), "Original: %d x %d pixels", original_width, original_height);
    info_label.set_text(info_text);
    content->pack_start(info_label, Gtk::PACK_SHRINK);

    // Scale percentage label and slider
    Gtk::Label scale_label("Échelle: 100%");
    scale_label.set_margin_top(5);
    content->pack_start(scale_label, Gtk::PACK_SHRINK);

    Gtk::HScale scale_slider;
    scale_slider.set_range(10, 400);  // 10% to 400%
    scale_slider.set_value(100);
    scale_slider.set_increments(1, 10);
    scale_slider.set_draw_value(false);
    content->pack_start(scale_slider, Gtk::PACK_SHRINK);

    // Quick scale buttons
    Gtk::Box buttons_box(Gtk::ORIENTATION_HORIZONTAL, 5);
    buttons_box.set_halign(Gtk::ALIGN_CENTER);
    Gtk::Button btn_25("25%");
    Gtk::Button btn_50("50%");
    Gtk::Button btn_100("100%");
    Gtk::Button btn_150("150%");
    Gtk::Button btn_200("200%");

    btn_25.signal_clicked().connect([&]() { scale_slider.set_value(25); });
    btn_50.signal_clicked().connect([&]() { scale_slider.set_value(50); });
    btn_100.signal_clicked().connect([&]() { scale_slider.set_value(100); });
    btn_150.signal_clicked().connect([&]() { scale_slider.set_value(150); });
    btn_200.signal_clicked().connect([&]() { scale_slider.set_value(200); });

    buttons_box.pack_start(btn_25, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_50, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_100, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_150, Gtk::PACK_SHRINK);
    buttons_box.pack_start(btn_200, Gtk::PACK_SHRINK);
    content->pack_start(buttons_box, Gtk::PACK_SHRINK);

    // Result dimensions label
    Gtk::Label result_label;
    result_label.set_margin_top(10);
    content->pack_start(result_label, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    double current_scale = 100;

    // Create small preview of original
    int preview_max = 150;
    double preview_ratio = 1.0;
    if (original_width > preview_max || original_height > preview_max) {
        preview_ratio = static_cast<double>(preview_max) / std::max(original_width, original_height);
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original_width * preview_ratio),
        static_cast<int>(original_height * preview_ratio),
        Gdk::INTERP_BILINEAR);

    auto update_preview = [&]() {
        current_scale = scale_slider.get_value();

        // Update labels
        char label_text[64];
        snprintf(label_text, sizeof(label_text), "Échelle: %.0f%%", current_scale);
        scale_label.set_text(label_text);

        int new_width = static_cast<int>(original_width * current_scale / 100.0);
        int new_height = static_cast<int>(original_height * current_scale / 100.0);
        snprintf(label_text, sizeof(label_text), "Nouvelle taille: %d x %d pixels", new_width, new_height);
        result_label.set_text(label_text);

        // Update preview (scale the small preview proportionally)
        double preview_scale = current_scale / 100.0;
        // Limit preview size
        int pw = static_cast<int>(small_original->get_width() * preview_scale);
        int ph = static_cast<int>(small_original->get_height() * preview_scale);

        // Clamp preview size
        if (pw > 280) {
            double factor = 280.0 / pw;
            pw = 280;
            ph = static_cast<int>(ph * factor);
        }
        if (ph > 180) {
            double factor = 180.0 / ph;
            ph = 180;
            pw = static_cast<int>(pw * factor);
        }
        pw = std::max(10, pw);
        ph = std::max(10, ph);

        preview_pixbuf = original->scale_simple(pw, ph, Gdk::INTERP_BILINEAR);
        preview_area.queue_draw();
    };

    scale_slider.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();

        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double x = (area_w - preview_pixbuf->get_width()) / 2.0;
            double y = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, x, y);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && current_scale != 100) {
        int new_width = static_cast<int>(original_width * current_scale / 100.0);
        int new_height = static_cast<int>(original_height * current_scale / 100.0);

        auto before = original;
        m_image = original->scale_simple(new_width, new_height, Gdk::INTERP_BILINEAR);

        char desc[64];
        snprintf(desc, sizeof(desc), "Redimensionner à %d%%", static_cast<int>(current_scale));
        auto cmd = std::make_unique<ImageCommand>(
            desc,
            before,
            m_image->copy(),
            [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
        );
        m_commandStack->pushExecuted(std::move(cmd));

        m_canvas.queue_draw();
        char status[64];
        snprintf(status, sizeof(status), "Image redimensionnée à %d x %d", new_width, new_height);
        m_status_label.set_text(status);
    }
}

// === Helper function: image to canvas coordinates ===

void MainWindow::image_to_canvas_coords(int img_x, int img_y, double& canvas_x, double& canvas_y)
{
    if (!m_image) {
        canvas_x = canvas_y = 0;
        return;
    }

    const int canvas_width = m_canvas.get_allocated_width();
    const int canvas_height = m_canvas.get_allocated_height();
    const int img_width = m_image->get_width();
    const int img_height = m_image->get_height();

    double scaled_width = img_width * m_zoom_level;
    double scaled_height = img_height * m_zoom_level;

    double offset_x = (canvas_width - scaled_width) / 2.0 + m_pan_x;
    double offset_y = (canvas_height - scaled_height) / 2.0 + m_pan_y;

    canvas_x = offset_x + img_x * m_zoom_level;
    canvas_y = offset_y + img_y * m_zoom_level;
}

// === Selection functions ===

void MainWindow::start_selection(double x, double y)
{
    if (!m_image) return;

    int img_x, img_y;
    canvas_to_image_coords(x, y, img_x, img_y);

    // Clamp to image bounds
    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    m_is_selecting = true;
    m_selection_x1 = img_x;
    m_selection_y1 = img_y;
    m_selection_x2 = img_x;
    m_selection_y2 = img_y;
    m_canvas.queue_draw();
}

void MainWindow::continue_selection(double x, double y)
{
    if (!m_is_selecting || !m_image) return;

    int img_x, img_y;
    canvas_to_image_coords(x, y, img_x, img_y);

    // Clamp to image bounds
    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    m_selection_x2 = img_x;
    m_selection_y2 = img_y;

    // Update status with selection dimensions
    int w = std::abs(m_selection_x2 - m_selection_x1);
    int h = std::abs(m_selection_y2 - m_selection_y1);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Sélection: %d x %d pixels", w, h);
    m_status_label.set_text(buffer);

    m_canvas.queue_draw();
}

void MainWindow::finish_selection()
{
    m_is_selecting = false;

    // Only keep selection if it has some size
    if (std::abs(m_selection_x2 - m_selection_x1) > 1 &&
        std::abs(m_selection_y2 - m_selection_y1) > 1) {
        m_has_selection = true;

        // Normalize selection coordinates
        if (m_selection_x1 > m_selection_x2) std::swap(m_selection_x1, m_selection_x2);
        if (m_selection_y1 > m_selection_y2) std::swap(m_selection_y1, m_selection_y2);

        int w = m_selection_x2 - m_selection_x1;
        int h = m_selection_y2 - m_selection_y1;
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Sélection: %d x %d pixels (Ctrl+C copier, Delete supprimer)", w, h);
        m_status_label.set_text(buffer);
    } else {
        m_has_selection = false;
        m_status_label.set_text("Sélection trop petite, annulée");
    }
    m_canvas.queue_draw();
}

void MainWindow::clear_selection()
{
    m_has_selection = false;
    m_is_selecting = false;
    m_selection_x1 = m_selection_y1 = m_selection_x2 = m_selection_y2 = 0;
    m_status_label.set_text("Sélection effacée");
    m_canvas.queue_draw();
}

bool MainWindow::has_selection() const
{
    return m_has_selection;
}

void MainWindow::draw_selection_overlay(const Cairo::RefPtr<Cairo::Context>& cr)
{
    int x1 = m_selection_x1, y1 = m_selection_y1;
    int x2 = m_selection_x2, y2 = m_selection_y2;

    // Normalize
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    double cx1, cy1, cx2, cy2;
    image_to_canvas_coords(x1, y1, cx1, cy1);
    image_to_canvas_coords(x2, y2, cx2, cy2);

    cr->save();

    // Draw marching ants (dashed rectangle)
    cr->set_line_width(1);

    // White background line
    cr->set_source_rgb(1, 1, 1);
    cr->rectangle(cx1, cy1, cx2 - cx1, cy2 - cy1);
    cr->stroke();

    // Black dashed line on top
    std::vector<double> dashes = {4.0, 4.0};
    cr->set_dash(dashes, 0);
    cr->set_source_rgb(0, 0, 0);
    cr->rectangle(cx1, cy1, cx2 - cx1, cy2 - cy1);
    cr->stroke();

    cr->restore();
}

void MainWindow::on_menu_edit_copy()
{
    if (!m_image || !m_has_selection) {
        m_status_label.set_text("Aucune sélection à copier");
        return;
    }

    int x1 = std::min(m_selection_x1, m_selection_x2);
    int y1 = std::min(m_selection_y1, m_selection_y2);
    int w = std::abs(m_selection_x2 - m_selection_x1);
    int h = std::abs(m_selection_y2 - m_selection_y1);

    // Copy selection to clipboard
    m_clipboard = Gdk::Pixbuf::create_subpixbuf(m_image, x1, y1, w, h)->copy();

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Copié: %d x %d pixels", w, h);
    m_status_label.set_text(buffer);
}

void MainWindow::on_menu_edit_cut()
{
    if (!m_image || !m_has_selection) {
        m_status_label.set_text("Aucune sélection à couper");
        return;
    }

    // First copy
    on_menu_edit_copy();

    // Save for undo
    auto before = m_image->copy();

    int x1 = std::min(m_selection_x1, m_selection_x2);
    int y1 = std::min(m_selection_y1, m_selection_y2);
    int w = std::abs(m_selection_x2 - m_selection_x1);
    int h = std::abs(m_selection_y2 - m_selection_y1);

    // Fill selection with secondary color
    guchar r = static_cast<guchar>(m_secondary_color.get_red() * 255);
    guchar g = static_cast<guchar>(m_secondary_color.get_green() * 255);
    guchar b = static_cast<guchar>(m_secondary_color.get_blue() * 255);

    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    for (int py = y1; py < y1 + h; py++) {
        for (int px = x1; px < x1 + w; px++) {
            guchar* pixel = pixels + py * rowstride + px * n_channels;
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            if (n_channels == 4) pixel[3] = 255;
        }
    }

    // Create undo command
    auto cmd = std::make_unique<ImageCommand>(
        "Couper",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    clear_selection();
    m_canvas.queue_draw();
    m_status_label.set_text("Sélection coupée");
}

void MainWindow::on_menu_edit_paste()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image ouverte");
        return;
    }

    if (!m_clipboard) {
        m_status_label.set_text("Presse-papiers vide");
        return;
    }

    // Save for undo
    auto before = m_image->copy();

    // Paste at top-left corner (or center if no selection)
    int paste_x = 0, paste_y = 0;
    if (m_has_selection) {
        paste_x = std::min(m_selection_x1, m_selection_x2);
        paste_y = std::min(m_selection_y1, m_selection_y2);
    }

    // Copy clipboard onto image
    int cw = m_clipboard->get_width();
    int ch = m_clipboard->get_height();
    int iw = m_image->get_width();
    int ih = m_image->get_height();

    // Clamp paste area
    int copy_w = std::min(cw, iw - paste_x);
    int copy_h = std::min(ch, ih - paste_y);

    if (copy_w > 0 && copy_h > 0) {
        m_clipboard->copy_area(0, 0, copy_w, copy_h, m_image, paste_x, paste_y);
    }

    // Create undo command
    auto cmd = std::make_unique<ImageCommand>(
        "Coller",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Collé depuis le presse-papiers");
}

void MainWindow::on_menu_edit_delete()
{
    if (!m_image || !m_has_selection) {
        m_status_label.set_text("Aucune sélection à supprimer");
        return;
    }

    // Save for undo
    auto before = m_image->copy();

    int x1 = std::min(m_selection_x1, m_selection_x2);
    int y1 = std::min(m_selection_y1, m_selection_y2);
    int w = std::abs(m_selection_x2 - m_selection_x1);
    int h = std::abs(m_selection_y2 - m_selection_y1);

    // Fill selection with secondary color
    guchar r = static_cast<guchar>(m_secondary_color.get_red() * 255);
    guchar g = static_cast<guchar>(m_secondary_color.get_green() * 255);
    guchar b = static_cast<guchar>(m_secondary_color.get_blue() * 255);

    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    for (int py = y1; py < y1 + h; py++) {
        for (int px = x1; px < x1 + w; px++) {
            guchar* pixel = pixels + py * rowstride + px * n_channels;
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            if (n_channels == 4) pixel[3] = 255;
        }
    }

    // Create undo command
    auto cmd = std::make_unique<ImageCommand>(
        "Supprimer sélection",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    clear_selection();
    m_canvas.queue_draw();
    m_status_label.set_text("Sélection supprimée");
}

// === Shape drawing functions ===

void MainWindow::start_shape(double x, double y)
{
    if (!m_image) return;

    // Save image for undo
    m_image_before_stroke = m_image->copy();

    int img_x, img_y;
    canvas_to_image_coords(x, y, img_x, img_y);

    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    m_is_drawing_shape = true;
    m_shape_start_x = img_x;
    m_shape_start_y = img_y;
    m_shape_end_x = img_x;
    m_shape_end_y = img_y;

    m_canvas.queue_draw();
}

void MainWindow::continue_shape(double x, double y)
{
    if (!m_is_drawing_shape || !m_image) return;

    int img_x, img_y;
    canvas_to_image_coords(x, y, img_x, img_y);

    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    m_shape_end_x = img_x;
    m_shape_end_y = img_y;

    m_canvas.queue_draw();
}

void MainWindow::finish_shape()
{
    if (!m_is_drawing_shape || !m_image) return;

    m_is_drawing_shape = false;

    // Draw the shape on the image
    if (m_current_tool == Tool::LINE) {
        draw_line_on_image(m_shape_start_x, m_shape_start_y, m_shape_end_x, m_shape_end_y);
    } else if (m_current_tool == Tool::RECTANGLE) {
        draw_rectangle_on_image(m_shape_start_x, m_shape_start_y, m_shape_end_x, m_shape_end_y,
                                m_shape_mode == ShapeMode::FILLED);
    } else if (m_current_tool == Tool::CIRCLE) {
        int radius = static_cast<int>(std::sqrt(
            std::pow(m_shape_end_x - m_shape_start_x, 2) +
            std::pow(m_shape_end_y - m_shape_start_y, 2)));
        draw_circle_on_image(m_shape_start_x, m_shape_start_y, radius,
                            m_shape_mode == ShapeMode::FILLED);
    }

    // Create undo command
    std::string desc;
    switch (m_current_tool) {
        case Tool::LINE: desc = "Ligne"; break;
        case Tool::RECTANGLE: desc = "Rectangle"; break;
        case Tool::CIRCLE: desc = "Cercle"; break;
        default: desc = "Forme"; break;
    }

    auto cmd = std::make_unique<ImageCommand>(
        desc,
        m_image_before_stroke,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));
    m_image_before_stroke.reset();

    m_canvas.queue_draw();
    m_status_label.set_text(desc + " dessiné");
}

void MainWindow::draw_line_on_image(int x1, int y1, int x2, int y2)
{
    if (!m_image) return;

    guchar r = static_cast<guchar>(m_primary_color.get_red() * 255);
    guchar g = static_cast<guchar>(m_primary_color.get_green() * 255);
    guchar b = static_cast<guchar>(m_primary_color.get_blue() * 255);

    // Draw line using brush strokes for thickness
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int img_width = m_image->get_width();
    int img_height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    int radius = m_brush_size / 2;
    int radius_sq = radius * radius;

    while (true) {
        // Draw a circle at each point
        for (int py = -radius; py <= radius; py++) {
            for (int px = -radius; px <= radius; px++) {
                if (px*px + py*py <= radius_sq) {
                    int ix = x1 + px;
                    int iy = y1 + py;
                    if (ix >= 0 && ix < img_width && iy >= 0 && iy < img_height) {
                        guchar* pixel = pixels + iy * rowstride + ix * n_channels;
                        pixel[0] = r;
                        pixel[1] = g;
                        pixel[2] = b;
                        if (n_channels == 4) pixel[3] = 255;
                    }
                }
            }
        }

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void MainWindow::draw_rectangle_on_image(int x1, int y1, int x2, int y2, bool filled)
{
    if (!m_image) return;

    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    guchar r = static_cast<guchar>(m_primary_color.get_red() * 255);
    guchar g = static_cast<guchar>(m_primary_color.get_green() * 255);
    guchar b = static_cast<guchar>(m_primary_color.get_blue() * 255);

    int img_width = m_image->get_width();
    int img_height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    int thickness = m_brush_size;

    if (filled) {
        // Fill the entire rectangle
        for (int py = y1; py <= y2; py++) {
            for (int px = x1; px <= x2; px++) {
                if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                    guchar* pixel = pixels + py * rowstride + px * n_channels;
                    pixel[0] = r;
                    pixel[1] = g;
                    pixel[2] = b;
                    if (n_channels == 4) pixel[3] = 255;
                }
            }
        }
    } else {
        // Draw outline only
        // Top and bottom lines
        for (int px = x1; px <= x2; px++) {
            for (int t = 0; t < thickness; t++) {
                // Top
                int py = y1 + t;
                if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                    guchar* pixel = pixels + py * rowstride + px * n_channels;
                    pixel[0] = r; pixel[1] = g; pixel[2] = b;
                    if (n_channels == 4) pixel[3] = 255;
                }
                // Bottom
                py = y2 - t;
                if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                    guchar* pixel = pixels + py * rowstride + px * n_channels;
                    pixel[0] = r; pixel[1] = g; pixel[2] = b;
                    if (n_channels == 4) pixel[3] = 255;
                }
            }
        }
        // Left and right lines
        for (int py = y1; py <= y2; py++) {
            for (int t = 0; t < thickness; t++) {
                // Left
                int px = x1 + t;
                if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                    guchar* pixel = pixels + py * rowstride + px * n_channels;
                    pixel[0] = r; pixel[1] = g; pixel[2] = b;
                    if (n_channels == 4) pixel[3] = 255;
                }
                // Right
                px = x2 - t;
                if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                    guchar* pixel = pixels + py * rowstride + px * n_channels;
                    pixel[0] = r; pixel[1] = g; pixel[2] = b;
                    if (n_channels == 4) pixel[3] = 255;
                }
            }
        }
    }
}

void MainWindow::draw_circle_on_image(int cx, int cy, int radius, bool filled)
{
    if (!m_image || radius <= 0) return;

    guchar r = static_cast<guchar>(m_primary_color.get_red() * 255);
    guchar g = static_cast<guchar>(m_primary_color.get_green() * 255);
    guchar b = static_cast<guchar>(m_primary_color.get_blue() * 255);

    int img_width = m_image->get_width();
    int img_height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    int thickness = m_brush_size;

    if (filled) {
        // Fill circle
        int radius_sq = radius * radius;
        for (int py = cy - radius; py <= cy + radius; py++) {
            for (int px = cx - radius; px <= cx + radius; px++) {
                int dx = px - cx;
                int dy = py - cy;
                if (dx*dx + dy*dy <= radius_sq) {
                    if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                        guchar* pixel = pixels + py * rowstride + px * n_channels;
                        pixel[0] = r;
                        pixel[1] = g;
                        pixel[2] = b;
                        if (n_channels == 4) pixel[3] = 255;
                    }
                }
            }
        }
    } else {
        // Draw circle outline using midpoint algorithm
        int inner_radius = radius - thickness / 2;
        int outer_radius = radius + thickness / 2;
        int inner_sq = inner_radius * inner_radius;
        int outer_sq = outer_radius * outer_radius;

        for (int py = cy - outer_radius; py <= cy + outer_radius; py++) {
            for (int px = cx - outer_radius; px <= cx + outer_radius; px++) {
                int dx = px - cx;
                int dy = py - cy;
                int dist_sq = dx*dx + dy*dy;
                if (dist_sq >= inner_sq && dist_sq <= outer_sq) {
                    if (px >= 0 && px < img_width && py >= 0 && py < img_height) {
                        guchar* pixel = pixels + py * rowstride + px * n_channels;
                        pixel[0] = r;
                        pixel[1] = g;
                        pixel[2] = b;
                        if (n_channels == 4) pixel[3] = 255;
                    }
                }
            }
        }
    }
}

// === Crop functions ===

void MainWindow::start_crop(double x, double y)
{
    if (!m_image) return;

    int img_x, img_y;
    canvas_to_image_coords(x, y, img_x, img_y);

    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    m_is_cropping = true;
    m_crop_x1 = img_x;
    m_crop_y1 = img_y;
    m_crop_x2 = img_x;
    m_crop_y2 = img_y;

    m_canvas.queue_draw();
}

void MainWindow::continue_crop(double x, double y)
{
    if (!m_is_cropping || !m_image) return;

    int img_x, img_y;
    canvas_to_image_coords(x, y, img_x, img_y);

    img_x = std::max(0, std::min(img_x, m_image->get_width() - 1));
    img_y = std::max(0, std::min(img_y, m_image->get_height() - 1));

    m_crop_x2 = img_x;
    m_crop_y2 = img_y;

    // Update status
    int w = std::abs(m_crop_x2 - m_crop_x1);
    int h = std::abs(m_crop_y2 - m_crop_y1);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Recadrer: %d x %d pixels", w, h);
    m_status_label.set_text(buffer);

    m_canvas.queue_draw();
}

void MainWindow::finish_crop()
{
    m_is_cropping = false;

    if (std::abs(m_crop_x2 - m_crop_x1) > 1 && std::abs(m_crop_y2 - m_crop_y1) > 1) {
        // Normalize coordinates
        if (m_crop_x1 > m_crop_x2) std::swap(m_crop_x1, m_crop_x2);
        if (m_crop_y1 > m_crop_y2) std::swap(m_crop_y1, m_crop_y2);

        int w = m_crop_x2 - m_crop_x1;
        int h = m_crop_y2 - m_crop_y1;

        m_canvas.queue_draw();

        // Show confirmation dialog
        char msg[128];
        snprintf(msg, sizeof(msg), "Recadrer l'image à %d x %d pixels ?", w, h);

        Gtk::MessageDialog dialog(*this, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);
        dialog.set_secondary_text("Cette action peut être annulée avec Ctrl+Z");
        dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
        dialog.add_button("_Recadrer", Gtk::RESPONSE_OK);

        int result = dialog.run();

        if (result == Gtk::RESPONSE_OK) {
            apply_crop();
        } else {
            // Cancel - reset crop area
            m_crop_x1 = m_crop_y1 = m_crop_x2 = m_crop_y2 = 0;
            m_canvas.queue_draw();
            m_status_label.set_text("Recadrage annulé");
        }
    } else {
        m_crop_x1 = m_crop_y1 = m_crop_x2 = m_crop_y2 = 0;
        m_status_label.set_text("Zone de recadrage trop petite");
    }

    m_canvas.queue_draw();
}

void MainWindow::apply_crop()
{
    if (!m_image) return;

    // Normalize coordinates
    int x1 = std::min(m_crop_x1, m_crop_x2);
    int y1 = std::min(m_crop_y1, m_crop_y2);
    int w = std::abs(m_crop_x2 - m_crop_x1);
    int h = std::abs(m_crop_y2 - m_crop_y1);

    if (w <= 0 || h <= 0) return;

    auto before = m_image->copy();

    // Create cropped image
    m_image = Gdk::Pixbuf::create_subpixbuf(before, x1, y1, w, h)->copy();

    // Create undo command
    auto cmd = std::make_unique<ImageCommand>(
        "Recadrer",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    // Reset crop area
    m_crop_x1 = m_crop_y1 = m_crop_x2 = m_crop_y2 = 0;

    m_canvas.queue_draw();
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Image recadrée à %d x %d", w, h);
    m_status_label.set_text(buffer);
}

void MainWindow::on_menu_image_crop()
{
    select_tool(Tool::CROP);
}

// === Filter functions ===

void MainWindow::on_menu_filter_blur()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à traiter");
        return;
    }

    // Create dialog for blur radius
    Gtk::Dialog dialog("Flou", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(300, 150);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    Gtk::Label label("Rayon du flou:");
    content->pack_start(label, Gtk::PACK_SHRINK);

    Gtk::HScale scale;
    scale.set_range(1, 20);
    scale.set_value(3);
    scale.set_increments(1, 5);
    content->pack_start(scale, Gtk::PACK_SHRINK);

    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK) {
        int radius = static_cast<int>(scale.get_value());
        apply_blur(radius);
    }
}

void MainWindow::apply_blur(int radius)
{
    if (!m_image || radius <= 0) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    // Create a copy for reading
    auto temp = before->copy();
    guchar* src_pixels = temp->get_pixels();

    // Box blur
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r_sum = 0, g_sum = 0, b_sum = 0, count = 0;

            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int nx = x + kx;
                    int ny = y + ky;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        guchar* src = src_pixels + ny * rowstride + nx * n_channels;
                        r_sum += src[0];
                        g_sum += src[1];
                        b_sum += src[2];
                        count++;
                    }
                }
            }

            guchar* dst = pixels + y * rowstride + x * n_channels;
            dst[0] = static_cast<guchar>(r_sum / count);
            dst[1] = static_cast<guchar>(g_sum / count);
            dst[2] = static_cast<guchar>(b_sum / count);
        }
    }

    // Create undo command
    auto cmd = std::make_unique<ImageCommand>(
        "Flou",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Filtre flou appliqué");
}

void MainWindow::on_menu_filter_sharpen()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à traiter");
        return;
    }
    apply_sharpen();
}

void MainWindow::apply_sharpen()
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    auto temp = before->copy();
    guchar* src_pixels = temp->get_pixels();

    // Sharpen kernel: center = 5, edges = -1
    // 0 -1  0
    // -1 5 -1
    // 0 -1  0

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < 3; c++) {
                int sum = 5 * src_pixels[y * rowstride + x * n_channels + c]
                        - src_pixels[(y-1) * rowstride + x * n_channels + c]
                        - src_pixels[(y+1) * rowstride + x * n_channels + c]
                        - src_pixels[y * rowstride + (x-1) * n_channels + c]
                        - src_pixels[y * rowstride + (x+1) * n_channels + c];
                pixels[y * rowstride + x * n_channels + c] = static_cast<guchar>(std::max(0, std::min(255, sum)));
            }
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Netteté",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Filtre netteté appliqué");
}

void MainWindow::on_menu_filter_grayscale()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à traiter");
        return;
    }
    apply_grayscale();
}

void MainWindow::apply_grayscale()
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            // Luminance formula
            guchar gray = static_cast<guchar>(0.299 * pixel[0] + 0.587 * pixel[1] + 0.114 * pixel[2]);
            pixel[0] = gray;
            pixel[1] = gray;
            pixel[2] = gray;
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Niveaux de gris",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Filtre niveaux de gris appliqué");
}

void MainWindow::on_menu_filter_invert()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à traiter");
        return;
    }
    apply_invert();
}

void MainWindow::apply_invert()
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            pixel[0] = 255 - pixel[0];
            pixel[1] = 255 - pixel[1];
            pixel[2] = 255 - pixel[2];
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Inverser couleurs",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Filtre inversion appliqué");
}

void MainWindow::on_menu_filter_sepia()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à traiter");
        return;
    }
    apply_sepia();
}

void MainWindow::apply_sepia()
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            int r = pixel[0];
            int g = pixel[1];
            int b = pixel[2];

            int tr = static_cast<int>(0.393 * r + 0.769 * g + 0.189 * b);
            int tg = static_cast<int>(0.349 * r + 0.686 * g + 0.168 * b);
            int tb = static_cast<int>(0.272 * r + 0.534 * g + 0.131 * b);

            pixel[0] = static_cast<guchar>(std::min(255, tr));
            pixel[1] = static_cast<guchar>(std::min(255, tg));
            pixel[2] = static_cast<guchar>(std::min(255, tb));
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Sépia",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Filtre sépia appliqué");
}

// === Brightness/Contrast adjustment ===

void MainWindow::on_menu_image_brightness_contrast()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à ajuster");
        return;
    }

    auto original = m_image->copy();

    Gtk::Dialog dialog("Luminosité / Contraste", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 350);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    // Preview area
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 200);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Brightness
    Gtk::Label brightness_label("Luminosité: 0");
    content->pack_start(brightness_label, Gtk::PACK_SHRINK);

    Gtk::HScale brightness_scale;
    brightness_scale.set_range(-100, 100);
    brightness_scale.set_value(0);
    brightness_scale.set_increments(1, 10);
    content->pack_start(brightness_scale, Gtk::PACK_SHRINK);

    // Contrast
    Gtk::Label contrast_label("Contraste: 0");
    content->pack_start(contrast_label, Gtk::PACK_SHRINK);

    Gtk::HScale contrast_scale;
    contrast_scale.set_range(-100, 100);
    contrast_scale.set_value(0);
    contrast_scale.set_increments(1, 10);
    content->pack_start(contrast_scale, Gtk::PACK_SHRINK);

    // Reset button
    Gtk::Button reset_btn("Réinitialiser");
    reset_btn.signal_clicked().connect([&]() {
        brightness_scale.set_value(0);
        contrast_scale.set_value(0);
    });
    content->pack_start(reset_btn, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    int current_brightness = 0;
    int current_contrast = 0;

    // Create small preview
    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);

    auto update_preview = [&]() {
        current_brightness = static_cast<int>(brightness_scale.get_value());
        current_contrast = static_cast<int>(contrast_scale.get_value());

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Luminosité: %d", current_brightness);
        brightness_label.set_text(buffer);
        snprintf(buffer, sizeof(buffer), "Contraste: %d", current_contrast);
        contrast_label.set_text(buffer);

        // Apply to preview
        preview_pixbuf = small_original->copy();
        int width = preview_pixbuf->get_width();
        int height = preview_pixbuf->get_height();
        int n_channels = preview_pixbuf->get_n_channels();
        int rowstride = preview_pixbuf->get_rowstride();
        guchar* pixels = preview_pixbuf->get_pixels();

        double contrast_factor = (259.0 * (current_contrast + 255)) / (255.0 * (259 - current_contrast));

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                guchar* pixel = pixels + y * rowstride + x * n_channels;
                for (int c = 0; c < 3; c++) {
                    double val = pixel[c] + current_brightness;
                    val = contrast_factor * (val - 128) + 128;
                    pixel[c] = static_cast<guchar>(std::max(0.0, std::min(255.0, val)));
                }
            }
        }

        preview_area.queue_draw();
    };

    brightness_scale.signal_value_changed().connect(update_preview);
    contrast_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();

        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double x = (area_w - preview_pixbuf->get_width()) / 2.0;
            double y = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, x, y);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && (current_brightness != 0 || current_contrast != 0)) {
        apply_brightness_contrast(current_brightness, current_contrast);
    }
}

void MainWindow::apply_brightness_contrast(int brightness, int contrast)
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    double contrast_factor = (259.0 * (contrast + 255)) / (255.0 * (259 - contrast));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            for (int c = 0; c < 3; c++) {
                double val = pixel[c] + brightness;
                val = contrast_factor * (val - 128) + 128;
                pixel[c] = static_cast<guchar>(std::max(0.0, std::min(255.0, val)));
            }
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Luminosité/Contraste",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Ajustement luminosité/contraste appliqué");
}

// === Gamma adjustment ===

void MainWindow::on_menu_adjust_gamma()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à ajuster");
        return;
    }

    auto original = m_image->copy();

    Gtk::Dialog dialog("Correction Gamma", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 300);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    // Preview
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 150);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Gamma slider
    Gtk::Label gamma_label("Gamma: 1.00");
    content->pack_start(gamma_label, Gtk::PACK_SHRINK);

    Gtk::HScale gamma_scale;
    gamma_scale.set_range(0.1, 3.0);
    gamma_scale.set_value(1.0);
    gamma_scale.set_increments(0.01, 0.1);
    content->pack_start(gamma_scale, Gtk::PACK_SHRINK);

    // Presets
    Gtk::Box presets_box(Gtk::ORIENTATION_HORIZONTAL, 5);
    presets_box.set_halign(Gtk::ALIGN_CENTER);
    Gtk::Button btn_dark("Sombre (0.5)");
    Gtk::Button btn_normal("Normal (1.0)");
    Gtk::Button btn_bright("Clair (2.0)");
    btn_dark.signal_clicked().connect([&]() { gamma_scale.set_value(0.5); });
    btn_normal.signal_clicked().connect([&]() { gamma_scale.set_value(1.0); });
    btn_bright.signal_clicked().connect([&]() { gamma_scale.set_value(2.0); });
    presets_box.pack_start(btn_dark, Gtk::PACK_SHRINK);
    presets_box.pack_start(btn_normal, Gtk::PACK_SHRINK);
    presets_box.pack_start(btn_bright, Gtk::PACK_SHRINK);
    content->pack_start(presets_box, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    double current_gamma = 1.0;

    // Small preview
    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);

    auto update_preview = [&]() {
        current_gamma = gamma_scale.get_value();
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Gamma: %.2f", current_gamma);
        gamma_label.set_text(buffer);

        preview_pixbuf = small_original->copy();
        int width = preview_pixbuf->get_width();
        int height = preview_pixbuf->get_height();
        int n_channels = preview_pixbuf->get_n_channels();
        int rowstride = preview_pixbuf->get_rowstride();
        guchar* pixels = preview_pixbuf->get_pixels();

        double inv_gamma = 1.0 / current_gamma;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                guchar* pixel = pixels + y * rowstride + x * n_channels;
                for (int c = 0; c < 3; c++) {
                    double normalized = pixel[c] / 255.0;
                    double corrected = std::pow(normalized, inv_gamma);
                    pixel[c] = static_cast<guchar>(std::min(255.0, corrected * 255.0));
                }
            }
        }
        preview_area.queue_draw();
    };

    gamma_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();
        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double x = (area_w - preview_pixbuf->get_width()) / 2.0;
            double y = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, x, y);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && current_gamma != 1.0) {
        apply_gamma(current_gamma);
    }
}

void MainWindow::apply_gamma(double gamma)
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    double inv_gamma = 1.0 / gamma;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            for (int c = 0; c < 3; c++) {
                double normalized = pixel[c] / 255.0;
                double corrected = std::pow(normalized, inv_gamma);
                pixel[c] = static_cast<guchar>(std::min(255.0, corrected * 255.0));
            }
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Gamma",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Correction gamma appliquée");
}

// === Hue/Saturation adjustment ===

void MainWindow::on_menu_adjust_hue_saturation()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à ajuster");
        return;
    }

    auto original = m_image->copy();

    Gtk::Dialog dialog("Teinte / Saturation", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 400);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    // Preview
    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 150);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Hue
    Gtk::Label hue_label("Teinte: 0");
    content->pack_start(hue_label, Gtk::PACK_SHRINK);
    Gtk::HScale hue_scale;
    hue_scale.set_range(-180, 180);
    hue_scale.set_value(0);
    hue_scale.set_increments(1, 10);
    content->pack_start(hue_scale, Gtk::PACK_SHRINK);

    // Saturation
    Gtk::Label sat_label("Saturation: 0");
    content->pack_start(sat_label, Gtk::PACK_SHRINK);
    Gtk::HScale sat_scale;
    sat_scale.set_range(-100, 100);
    sat_scale.set_value(0);
    sat_scale.set_increments(1, 10);
    content->pack_start(sat_scale, Gtk::PACK_SHRINK);

    // Lightness
    Gtk::Label light_label("Luminosité: 0");
    content->pack_start(light_label, Gtk::PACK_SHRINK);
    Gtk::HScale light_scale;
    light_scale.set_range(-100, 100);
    light_scale.set_value(0);
    light_scale.set_increments(1, 10);
    content->pack_start(light_scale, Gtk::PACK_SHRINK);

    // Reset
    Gtk::Button reset_btn("Réinitialiser");
    reset_btn.signal_clicked().connect([&]() {
        hue_scale.set_value(0);
        sat_scale.set_value(0);
        light_scale.set_value(0);
    });
    content->pack_start(reset_btn, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    int current_hue = 0, current_sat = 0, current_light = 0;

    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);

    // RGB to HSL conversion helper
    auto rgb_to_hsl = [](double r, double g, double b, double& h, double& s, double& l) {
        double max_c = std::max({r, g, b});
        double min_c = std::min({r, g, b});
        l = (max_c + min_c) / 2.0;

        if (max_c == min_c) {
            h = s = 0.0;
        } else {
            double d = max_c - min_c;
            s = l > 0.5 ? d / (2.0 - max_c - min_c) : d / (max_c + min_c);
            if (max_c == r) h = (g - b) / d + (g < b ? 6.0 : 0.0);
            else if (max_c == g) h = (b - r) / d + 2.0;
            else h = (r - g) / d + 4.0;
            h /= 6.0;
        }
    };

    // HSL to RGB conversion helper
    auto hsl_to_rgb = [](double h, double s, double l, double& r, double& g, double& b) {
        if (s == 0.0) {
            r = g = b = l;
        } else {
            auto hue2rgb = [](double p, double q, double t) {
                if (t < 0.0) t += 1.0;
                if (t > 1.0) t -= 1.0;
                if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
                if (t < 1.0/2.0) return q;
                if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
                return p;
            };
            double q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
            double p = 2.0 * l - q;
            r = hue2rgb(p, q, h + 1.0/3.0);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1.0/3.0);
        }
    };

    auto update_preview = [&]() {
        current_hue = static_cast<int>(hue_scale.get_value());
        current_sat = static_cast<int>(sat_scale.get_value());
        current_light = static_cast<int>(light_scale.get_value());

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Teinte: %d", current_hue);
        hue_label.set_text(buffer);
        snprintf(buffer, sizeof(buffer), "Saturation: %d", current_sat);
        sat_label.set_text(buffer);
        snprintf(buffer, sizeof(buffer), "Luminosité: %d", current_light);
        light_label.set_text(buffer);

        preview_pixbuf = small_original->copy();
        int width = preview_pixbuf->get_width();
        int height = preview_pixbuf->get_height();
        int n_channels = preview_pixbuf->get_n_channels();
        int rowstride = preview_pixbuf->get_rowstride();
        guchar* pixels = preview_pixbuf->get_pixels();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                guchar* pixel = pixels + y * rowstride + x * n_channels;
                double r = pixel[0] / 255.0;
                double g = pixel[1] / 255.0;
                double b = pixel[2] / 255.0;

                double h, s, l;
                rgb_to_hsl(r, g, b, h, s, l);

                h += current_hue / 360.0;
                if (h < 0) h += 1.0;
                if (h > 1) h -= 1.0;

                s = std::max(0.0, std::min(1.0, s + current_sat / 100.0));
                l = std::max(0.0, std::min(1.0, l + current_light / 100.0));

                hsl_to_rgb(h, s, l, r, g, b);

                pixel[0] = static_cast<guchar>(std::max(0.0, std::min(255.0, r * 255.0)));
                pixel[1] = static_cast<guchar>(std::max(0.0, std::min(255.0, g * 255.0)));
                pixel[2] = static_cast<guchar>(std::max(0.0, std::min(255.0, b * 255.0)));
            }
        }
        preview_area.queue_draw();
    };

    hue_scale.signal_value_changed().connect(update_preview);
    sat_scale.signal_value_changed().connect(update_preview);
    light_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();
        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double px = (area_w - preview_pixbuf->get_width()) / 2.0;
            double py = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, px, py);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && (current_hue != 0 || current_sat != 0 || current_light != 0)) {
        apply_hue_saturation(current_hue, current_sat, current_light);
    }
}

void MainWindow::apply_hue_saturation(int hue, int saturation, int lightness)
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    auto rgb_to_hsl = [](double r, double g, double b, double& h, double& s, double& l) {
        double max_c = std::max({r, g, b});
        double min_c = std::min({r, g, b});
        l = (max_c + min_c) / 2.0;
        if (max_c == min_c) { h = s = 0.0; }
        else {
            double d = max_c - min_c;
            s = l > 0.5 ? d / (2.0 - max_c - min_c) : d / (max_c + min_c);
            if (max_c == r) h = (g - b) / d + (g < b ? 6.0 : 0.0);
            else if (max_c == g) h = (b - r) / d + 2.0;
            else h = (r - g) / d + 4.0;
            h /= 6.0;
        }
    };

    auto hsl_to_rgb = [](double h, double s, double l, double& r, double& g, double& b) {
        if (s == 0.0) { r = g = b = l; }
        else {
            auto hue2rgb = [](double p, double q, double t) {
                if (t < 0.0) t += 1.0; if (t > 1.0) t -= 1.0;
                if (t < 1.0/6.0) return p + (q - p) * 6.0 * t;
                if (t < 1.0/2.0) return q;
                if (t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;
                return p;
            };
            double q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
            double p = 2.0 * l - q;
            r = hue2rgb(p, q, h + 1.0/3.0);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1.0/3.0);
        }
    };

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            double r = pixel[0] / 255.0, g = pixel[1] / 255.0, b = pixel[2] / 255.0;
            double h, s, l;
            rgb_to_hsl(r, g, b, h, s, l);

            h += hue / 360.0;
            if (h < 0) h += 1.0; if (h > 1) h -= 1.0;
            s = std::max(0.0, std::min(1.0, s + saturation / 100.0));
            l = std::max(0.0, std::min(1.0, l + lightness / 100.0));

            hsl_to_rgb(h, s, l, r, g, b);
            pixel[0] = static_cast<guchar>(std::max(0.0, std::min(255.0, r * 255.0)));
            pixel[1] = static_cast<guchar>(std::max(0.0, std::min(255.0, g * 255.0)));
            pixel[2] = static_cast<guchar>(std::max(0.0, std::min(255.0, b * 255.0)));
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Teinte/Saturation",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Ajustement teinte/saturation appliqué");
}

// === Exposure adjustment ===

void MainWindow::on_menu_adjust_exposure()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à ajuster");
        return;
    }

    auto original = m_image->copy();

    Gtk::Dialog dialog("Exposition", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 300);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 150);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    Gtk::Label exposure_label("Exposition: 0.00");
    content->pack_start(exposure_label, Gtk::PACK_SHRINK);

    Gtk::HScale exposure_scale;
    exposure_scale.set_range(-3.0, 3.0);
    exposure_scale.set_value(0.0);
    exposure_scale.set_increments(0.1, 0.5);
    content->pack_start(exposure_scale, Gtk::PACK_SHRINK);

    Gtk::Button reset_btn("Réinitialiser");
    reset_btn.signal_clicked().connect([&]() { exposure_scale.set_value(0.0); });
    content->pack_start(reset_btn, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    double current_exposure = 0.0;

    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);

    auto update_preview = [&]() {
        current_exposure = exposure_scale.get_value();
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Exposition: %+.2f EV", current_exposure);
        exposure_label.set_text(buffer);

        preview_pixbuf = small_original->copy();
        int width = preview_pixbuf->get_width();
        int height = preview_pixbuf->get_height();
        int n_channels = preview_pixbuf->get_n_channels();
        int rowstride = preview_pixbuf->get_rowstride();
        guchar* pixels = preview_pixbuf->get_pixels();

        double multiplier = std::pow(2.0, current_exposure);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                guchar* pixel = pixels + y * rowstride + x * n_channels;
                for (int c = 0; c < 3; c++) {
                    double val = pixel[c] * multiplier;
                    pixel[c] = static_cast<guchar>(std::max(0.0, std::min(255.0, val)));
                }
            }
        }
        preview_area.queue_draw();
    };

    exposure_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();
        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double px = (area_w - preview_pixbuf->get_width()) / 2.0;
            double py = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, px, py);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && current_exposure != 0.0) {
        apply_exposure(current_exposure);
    }
}

void MainWindow::apply_exposure(double exposure)
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    double multiplier = std::pow(2.0, exposure);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            for (int c = 0; c < 3; c++) {
                double val = pixel[c] * multiplier;
                pixel[c] = static_cast<guchar>(std::max(0.0, std::min(255.0, val)));
            }
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Exposition",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Ajustement d'exposition appliqué");
}

// === Temperature adjustment ===

void MainWindow::on_menu_adjust_temperature()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à ajuster");
        return;
    }

    auto original = m_image->copy();

    Gtk::Dialog dialog("Température de couleur", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(400, 300);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 150);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    Gtk::Label temp_label("Température: 0 (neutre)");
    content->pack_start(temp_label, Gtk::PACK_SHRINK);

    Gtk::HScale temp_scale;
    temp_scale.set_range(-100, 100);
    temp_scale.set_value(0);
    temp_scale.set_increments(1, 10);
    content->pack_start(temp_scale, Gtk::PACK_SHRINK);

    Gtk::Box presets_box(Gtk::ORIENTATION_HORIZONTAL, 5);
    presets_box.set_halign(Gtk::ALIGN_CENTER);
    Gtk::Button btn_cold("Froid (-50)");
    Gtk::Button btn_neutral("Neutre (0)");
    Gtk::Button btn_warm("Chaud (+50)");
    btn_cold.signal_clicked().connect([&]() { temp_scale.set_value(-50); });
    btn_neutral.signal_clicked().connect([&]() { temp_scale.set_value(0); });
    btn_warm.signal_clicked().connect([&]() { temp_scale.set_value(50); });
    presets_box.pack_start(btn_cold, Gtk::PACK_SHRINK);
    presets_box.pack_start(btn_neutral, Gtk::PACK_SHRINK);
    presets_box.pack_start(btn_warm, Gtk::PACK_SHRINK);
    content->pack_start(presets_box, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    int current_temp = 0;

    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);

    auto update_preview = [&]() {
        current_temp = static_cast<int>(temp_scale.get_value());
        char buffer[64];
        if (current_temp < 0)
            snprintf(buffer, sizeof(buffer), "Température: %d (froid)", current_temp);
        else if (current_temp > 0)
            snprintf(buffer, sizeof(buffer), "Température: +%d (chaud)", current_temp);
        else
            snprintf(buffer, sizeof(buffer), "Température: 0 (neutre)");
        temp_label.set_text(buffer);

        preview_pixbuf = small_original->copy();
        int width = preview_pixbuf->get_width();
        int height = preview_pixbuf->get_height();
        int n_channels = preview_pixbuf->get_n_channels();
        int rowstride = preview_pixbuf->get_rowstride();
        guchar* pixels = preview_pixbuf->get_pixels();

        // Warm = more red/yellow, less blue
        // Cold = more blue, less red
        double r_adj = current_temp * 0.5;
        double b_adj = -current_temp * 0.5;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                guchar* pixel = pixels + y * rowstride + x * n_channels;
                pixel[0] = static_cast<guchar>(std::max(0.0, std::min(255.0, pixel[0] + r_adj)));
                pixel[2] = static_cast<guchar>(std::max(0.0, std::min(255.0, pixel[2] + b_adj)));
            }
        }
        preview_area.queue_draw();
    };

    temp_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();
        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double px = (area_w - preview_pixbuf->get_width()) / 2.0;
            double py = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, px, py);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && current_temp != 0) {
        apply_temperature(current_temp);
    }
}

void MainWindow::apply_temperature(int temperature)
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    double r_adj = temperature * 0.5;
    double b_adj = -temperature * 0.5;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            pixel[0] = static_cast<guchar>(std::max(0.0, std::min(255.0, pixel[0] + r_adj)));
            pixel[2] = static_cast<guchar>(std::max(0.0, std::min(255.0, pixel[2] + b_adj)));
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Température",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Ajustement de température appliqué");
}

// === Levels adjustment ===

void MainWindow::on_menu_adjust_levels()
{
    if (!m_image) {
        m_status_label.set_text("Aucune image à ajuster");
        return;
    }

    auto original = m_image->copy();

    Gtk::Dialog dialog("Niveaux", *this, true);
    dialog.add_button("_Annuler", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Appliquer", Gtk::RESPONSE_OK);
    dialog.set_default_size(450, 400);

    auto content = dialog.get_content_area();
    content->set_spacing(10);
    content->set_margin_start(15);
    content->set_margin_end(15);
    content->set_margin_top(15);

    Gtk::Frame preview_frame("Aperçu");
    Gtk::DrawingArea preview_area;
    preview_area.set_size_request(300, 150);
    preview_frame.add(preview_area);
    content->pack_start(preview_frame, Gtk::PACK_EXPAND_WIDGET);

    // Black point
    Gtk::Label black_label("Point noir: 0");
    content->pack_start(black_label, Gtk::PACK_SHRINK);
    Gtk::HScale black_scale;
    black_scale.set_range(0, 127);
    black_scale.set_value(0);
    black_scale.set_increments(1, 10);
    content->pack_start(black_scale, Gtk::PACK_SHRINK);

    // White point
    Gtk::Label white_label("Point blanc: 255");
    content->pack_start(white_label, Gtk::PACK_SHRINK);
    Gtk::HScale white_scale;
    white_scale.set_range(128, 255);
    white_scale.set_value(255);
    white_scale.set_increments(1, 10);
    content->pack_start(white_scale, Gtk::PACK_SHRINK);

    // Gamma
    Gtk::Label gamma_label("Gamma: 1.00");
    content->pack_start(gamma_label, Gtk::PACK_SHRINK);
    Gtk::HScale gamma_scale;
    gamma_scale.set_range(0.1, 3.0);
    gamma_scale.set_value(1.0);
    gamma_scale.set_increments(0.01, 0.1);
    content->pack_start(gamma_scale, Gtk::PACK_SHRINK);

    Gtk::Button reset_btn("Réinitialiser");
    reset_btn.signal_clicked().connect([&]() {
        black_scale.set_value(0);
        white_scale.set_value(255);
        gamma_scale.set_value(1.0);
    });
    content->pack_start(reset_btn, Gtk::PACK_SHRINK);

    Glib::RefPtr<Gdk::Pixbuf> preview_pixbuf;
    int current_black = 0, current_white = 255;
    double current_gamma = 1.0;

    int preview_max = 200;
    double scale = 1.0;
    if (original->get_width() > preview_max || original->get_height() > preview_max) {
        scale = static_cast<double>(preview_max) / std::max(original->get_width(), original->get_height());
    }
    auto small_original = original->scale_simple(
        static_cast<int>(original->get_width() * scale),
        static_cast<int>(original->get_height() * scale),
        Gdk::INTERP_BILINEAR);

    auto update_preview = [&]() {
        current_black = static_cast<int>(black_scale.get_value());
        current_white = static_cast<int>(white_scale.get_value());
        current_gamma = gamma_scale.get_value();

        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Point noir: %d", current_black);
        black_label.set_text(buffer);
        snprintf(buffer, sizeof(buffer), "Point blanc: %d", current_white);
        white_label.set_text(buffer);
        snprintf(buffer, sizeof(buffer), "Gamma: %.2f", current_gamma);
        gamma_label.set_text(buffer);

        preview_pixbuf = small_original->copy();
        int width = preview_pixbuf->get_width();
        int height = preview_pixbuf->get_height();
        int n_channels = preview_pixbuf->get_n_channels();
        int rowstride = preview_pixbuf->get_rowstride();
        guchar* pixels = preview_pixbuf->get_pixels();

        double range = current_white - current_black;
        if (range <= 0) range = 1;
        double inv_gamma = 1.0 / current_gamma;

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                guchar* pixel = pixels + y * rowstride + x * n_channels;
                for (int c = 0; c < 3; c++) {
                    double val = (pixel[c] - current_black) / range;
                    val = std::max(0.0, std::min(1.0, val));
                    val = std::pow(val, inv_gamma);
                    pixel[c] = static_cast<guchar>(val * 255.0);
                }
            }
        }
        preview_area.queue_draw();
    };

    black_scale.signal_value_changed().connect(update_preview);
    white_scale.signal_value_changed().connect(update_preview);
    gamma_scale.signal_value_changed().connect(update_preview);

    preview_area.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& cr) {
        cr->set_source_rgb(0.2, 0.2, 0.2);
        cr->paint();
        if (preview_pixbuf) {
            int area_w = preview_area.get_allocated_width();
            int area_h = preview_area.get_allocated_height();
            double px = (area_w - preview_pixbuf->get_width()) / 2.0;
            double py = (area_h - preview_pixbuf->get_height()) / 2.0;
            Gdk::Cairo::set_source_pixbuf(cr, preview_pixbuf, px, py);
            cr->paint();
        }
        return true;
    });

    update_preview();
    dialog.show_all_children();
    int result = dialog.run();

    if (result == Gtk::RESPONSE_OK && (current_black != 0 || current_white != 255 || current_gamma != 1.0)) {
        apply_levels(current_black, current_white, current_gamma);
    }
}

void MainWindow::apply_levels(int black_point, int white_point, double gamma)
{
    if (!m_image) return;

    auto before = m_image->copy();

    int width = m_image->get_width();
    int height = m_image->get_height();
    int n_channels = m_image->get_n_channels();
    int rowstride = m_image->get_rowstride();
    guchar* pixels = m_image->get_pixels();

    double range = white_point - black_point;
    if (range <= 0) range = 1;
    double inv_gamma = 1.0 / gamma;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            guchar* pixel = pixels + y * rowstride + x * n_channels;
            for (int c = 0; c < 3; c++) {
                double val = (pixel[c] - black_point) / range;
                val = std::max(0.0, std::min(1.0, val));
                val = std::pow(val, inv_gamma);
                pixel[c] = static_cast<guchar>(val * 255.0);
            }
        }
    }

    auto cmd = std::make_unique<ImageCommand>(
        "Niveaux",
        before,
        m_image->copy(),
        [this](Glib::RefPtr<Gdk::Pixbuf> img) { restore_image(img); }
    );
    m_commandStack->pushExecuted(std::move(cmd));

    m_canvas.queue_draw();
    m_status_label.set_text("Ajustement des niveaux appliqué");
}

} // namespace EpiGimp
