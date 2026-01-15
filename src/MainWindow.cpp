#include "epigimp/MainWindow.hpp"
#include <gdkmm/general.h>
#include <glibmm/miscutils.h>
#include <iostream>

namespace EpiGimp {

MainWindow::MainWindow()
    : m_main_box(Gtk::ORIENTATION_VERTICAL),
      m_content_box(Gtk::ORIENTATION_HORIZONTAL),
      m_toolbar_box(Gtk::ORIENTATION_VERTICAL, 2),
      m_color_panel(Gtk::ORIENTATION_VERTICAL, 5),
      m_statusbar(Gtk::ORIENTATION_HORIZONTAL, 10)
{
    // Window configuration
    set_default_size(1024, 768);
    update_title();

    // Initialize colors
    m_primary_color.set_rgba(0.0, 0.0, 0.0, 1.0);      // Black
    m_secondary_color.set_rgba(1.0, 1.0, 1.0, 1.0);    // White

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

    m_menu_file_quit.set_label("_Quitter");
    m_menu_file_quit.set_use_underline(true);
    m_menu_file_quit.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_menu_file_quit));

    m_submenu_file.append(m_menu_file_new);
    m_submenu_file.append(m_menu_file_open);
    m_submenu_file.append(m_menu_file_save);
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
    m_menu_bar.append(m_menu_help);
}

void MainWindow::setup_toolbar()
{
    m_toolbar_box.set_margin_start(2);
    m_toolbar_box.set_margin_end(2);
    m_toolbar_box.set_margin_top(5);

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
    std::cout << "Fichier > Enregistrer" << std::endl;
    // TODO: Implement file save functionality
}

void MainWindow::on_menu_file_quit()
{
    // Close the window, which will trigger application quit
    close();
}

void MainWindow::on_menu_edit_undo()
{
    std::cout << "Édition > Annuler" << std::endl;
    // TODO: Implement undo functionality
}

void MainWindow::on_menu_edit_redo()
{
    std::cout << "Édition > Rétablir" << std::endl;
    // TODO: Implement redo functionality
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

    // Update cursor based on tool
    if (m_canvas.get_window()) {
        auto display = get_display();
        Glib::RefPtr<Gdk::Cursor> cursor;

        switch (tool) {
            case Tool::PIPETTE:
                cursor = Gdk::Cursor::create(display, "crosshair");
                m_status_label.set_text("Pipette: Cliquez sur l'image pour récupérer une couleur");
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
    if (event->button == 2 || event->button == 1) {
        if (m_is_panning) {
            m_is_panning = false;

            // Restore cursor based on current tool
            auto display = get_display();
            Glib::RefPtr<Gdk::Cursor> cursor;
            if (m_current_tool == Tool::PIPETTE) {
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

    // I for pipette tool
    if (event->keyval == GDK_KEY_i || event->keyval == GDK_KEY_I) {
        if (m_current_tool == Tool::PIPETTE) {
            select_tool(Tool::NONE);
        } else {
            select_tool(Tool::PIPETTE);
        }
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

} // namespace EpiGimp
