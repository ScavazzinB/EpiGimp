#ifndef EPIGIMP_MAINWINDOW_HPP
#define EPIGIMP_MAINWINDOW_HPP

#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/rgba.h>
#include <string>

namespace EpiGimp {

// Available tools
enum class Tool {
    NONE,
    PIPETTE,
    // Future tools will be added here
};

/**
 * @brief Main application window for EpiGimp
 *
 * This class represents the main window of the application,
 * containing the menu bar and main content area.
 */
class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow();
    virtual ~MainWindow() = default;

protected:
    // Signal handlers
    void on_menu_file_new();
    void on_menu_file_open();
    void on_menu_file_save();
    void on_menu_file_quit();
    void on_menu_edit_undo();
    void on_menu_edit_redo();
    void on_menu_help_about();

    // Canvas drawing handler
    bool on_canvas_draw(const Cairo::RefPtr<Cairo::Context>& cr);

    // Canvas event handlers
    bool on_canvas_scroll(GdkEventScroll* event);
    bool on_canvas_button_press(GdkEventButton* event);
    bool on_canvas_button_release(GdkEventButton* event);
    bool on_canvas_motion(GdkEventMotion* event);
    bool on_key_press(GdkEventKey* event);

private:
    void setup_menu();
    void setup_layout();
    void setup_toolbar();
    void setup_color_panel();
    void setup_statusbar();
    void update_title();
    void update_zoom_label();
    void update_color_display();
    bool load_image(const std::string& filepath);
    void zoom_in();
    void zoom_out();
    void zoom_fit();
    void zoom_100();
    void clamp_pan();
    void select_tool(Tool tool);
    bool pick_color_at(double x, double y);
    void swap_colors();

    // Current file info
    std::string m_current_filepath;
    Glib::RefPtr<Gdk::Pixbuf> m_image;

    // Zoom and pan state
    double m_zoom_level = 1.0;
    double m_pan_x = 0.0;
    double m_pan_y = 0.0;
    bool m_is_panning = false;
    double m_pan_start_x = 0.0;
    double m_pan_start_y = 0.0;
    double m_pan_origin_x = 0.0;
    double m_pan_origin_y = 0.0;

    static constexpr double ZOOM_MIN = 0.1;
    static constexpr double ZOOM_MAX = 10.0;
    static constexpr double ZOOM_STEP = 1.2;

    // Current tool
    Tool m_current_tool = Tool::NONE;

    // Colors
    Gdk::RGBA m_primary_color;
    Gdk::RGBA m_secondary_color;

    // Main layout
    Gtk::Box m_main_box;
    Gtk::Box m_content_box;  // Contains toolbar + canvas

    // Menu bar
    Gtk::MenuBar m_menu_bar;

    // File menu
    Gtk::MenuItem m_menu_file;
    Gtk::Menu m_submenu_file;
    Gtk::MenuItem m_menu_file_new;
    Gtk::MenuItem m_menu_file_open;
    Gtk::MenuItem m_menu_file_save;
    Gtk::SeparatorMenuItem m_menu_file_separator;
    Gtk::MenuItem m_menu_file_quit;

    // Edit menu
    Gtk::MenuItem m_menu_edit;
    Gtk::Menu m_submenu_edit;
    Gtk::MenuItem m_menu_edit_undo;
    Gtk::MenuItem m_menu_edit_redo;

    // Help menu
    Gtk::MenuItem m_menu_help;
    Gtk::Menu m_submenu_help;
    Gtk::MenuItem m_menu_help_about;

    // Toolbar
    Gtk::Box m_toolbar_box;
    Gtk::ToggleButton m_tool_pipette;

    // Color panel
    Gtk::Box m_color_panel;
    Gtk::DrawingArea m_color_display;
    Gtk::Button m_swap_colors_btn;

    // Main content area
    Gtk::ScrolledWindow m_scrolled_window;
    Gtk::DrawingArea m_canvas;

    // Status bar
    Gtk::Box m_statusbar;
    Gtk::Label m_zoom_label;
    Gtk::Label m_status_label;
    Gtk::Label m_color_label;
};

} // namespace EpiGimp

#endif // EPIGIMP_MAINWINDOW_HPP
