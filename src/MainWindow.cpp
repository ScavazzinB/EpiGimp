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
    m_menu_image.set_submenu(m_submenu_image);

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
    m_menu_bar.append(m_menu_help);
}

void MainWindow::setup_toolbar()
{
    m_toolbar_box.set_margin_start(2);
    m_toolbar_box.set_margin_end(2);
    m_toolbar_box.set_margin_top(5);

    // Brush tool button
    m_tool_brush.set_label("🖌");
    m_tool_brush.set_tooltip_text("Pinceau (B) - Dessiner");
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
    m_tool_eraser.set_tooltip_text("Gomme (E) - Effacer");
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
    m_tool_pipette.set_tooltip_text("Pipette (I) - Récupérer une couleur");
    m_tool_pipette.set_size_request(40, 40);
    m_tool_pipette.signal_toggled().connect([this]() {
        if (m_tool_pipette.get_active()) {
            select_tool(Tool::PIPETTE);
        } else if (m_current_tool == Tool::PIPETTE) {
            select_tool(Tool::NONE);
        }
    });
    m_toolbar_box.pack_start(m_tool_pipette, Gtk::PACK_SHRINK);

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
    m_swap_colors_btn.set_tooltip_text("Échanger les couleurs (X)");
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

        if (m_is_panning) {
            m_is_panning = false;

            // Restore cursor based on current tool
            auto display = get_display();
            Glib::RefPtr<Gdk::Cursor> cursor;
            if (m_current_tool == Tool::PIPETTE ||
                m_current_tool == Tool::BRUSH ||
                m_current_tool == Tool::ERASER) {
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
            if (m_current_tool == Tool::PIPETTE ||
                m_current_tool == Tool::BRUSH ||
                m_current_tool == Tool::ERASER) {
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

    // Escape to deselect tool
    if (event->keyval == GDK_KEY_Escape) {
        select_tool(Tool::NONE);
        return true;
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

} // namespace EpiGimp
