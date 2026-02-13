#ifndef EPIGIMP_MAINWINDOW_HPP
#define EPIGIMP_MAINWINDOW_HPP

#include <gtkmm.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/rgba.h>
#include <string>
#include <memory>
#include "CommandStack.hpp"
#include "ImageCommand.hpp"

namespace EpiGimp {

// Available tools
enum class Tool {
    NONE,
    PIPETTE,
    BRUSH,
    ERASER,
    SELECTION,
    LINE,
    RECTANGLE,
    CIRCLE,
    CROP
};

// Shape fill mode
enum class ShapeMode {
    OUTLINE,
    FILLED
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
    void on_menu_file_save_as();
    void on_menu_file_quit();

    // Image transformation handlers
    void on_menu_image_rotate();
    void on_menu_image_flip_h();
    void on_menu_image_flip_v();
    void on_menu_image_scale();
    void on_menu_image_crop();
    void on_menu_image_brightness_contrast();
    void on_menu_edit_undo();
    void on_menu_edit_redo();
    void on_menu_edit_copy();
    void on_menu_edit_cut();
    void on_menu_edit_paste();
    void on_menu_edit_delete();
    void on_menu_help_about();

    // Filter handlers
    void on_menu_filter_blur();
    void on_menu_filter_sharpen();
    void on_menu_filter_grayscale();
    void on_menu_filter_invert();
    void on_menu_filter_sepia();

    // Adjustment handlers
    void on_menu_adjust_gamma();
    void on_menu_adjust_hue_saturation();
    void on_menu_adjust_exposure();
    void on_menu_adjust_temperature();
    void on_menu_adjust_levels();

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
    void update_undo_redo_menu();
    void restore_image(Glib::RefPtr<Gdk::Pixbuf> image);

    // Brush/Eraser drawing
    void canvas_to_image_coords(double canvas_x, double canvas_y, int& img_x, int& img_y);
    void image_to_canvas_coords(int img_x, int img_y, double& canvas_x, double& canvas_y);
    void draw_brush_stroke(int x1, int y1, int x2, int y2, bool erase = false);
    void draw_brush_point(int x, int y, bool erase = false);
    void start_drawing(double x, double y);
    void continue_drawing(double x, double y);
    void finish_drawing();

    // Shape drawing
    void start_shape(double x, double y);
    void continue_shape(double x, double y);
    void finish_shape();
    void draw_line_on_image(int x1, int y1, int x2, int y2);
    void draw_rectangle_on_image(int x1, int y1, int x2, int y2, bool filled);
    void draw_circle_on_image(int cx, int cy, int radius, bool filled);

    // Selection
    void start_selection(double x, double y);
    void continue_selection(double x, double y);
    void finish_selection();
    void clear_selection();
    bool has_selection() const;
    void draw_selection_overlay(const Cairo::RefPtr<Cairo::Context>& cr);

    // Crop
    void start_crop(double x, double y);
    void continue_crop(double x, double y);
    void finish_crop();
    void apply_crop();

    // Filters
    void apply_blur(int radius);
    void apply_sharpen();
    void apply_grayscale();
    void apply_invert();
    void apply_sepia();
    void apply_brightness_contrast(int brightness, int contrast);
    void apply_gamma(double gamma);
    void apply_hue_saturation(int hue, int saturation, int lightness);
    void apply_exposure(double exposure);
    void apply_temperature(int temperature);
    void apply_levels(int black_point, int white_point, double gamma);

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

    // Brush/Drawing state
    int m_brush_size = 10;
    bool m_is_drawing = false;
    int m_last_draw_x = 0;
    int m_last_draw_y = 0;
    Glib::RefPtr<Gdk::Pixbuf> m_image_before_stroke;  // For undo

    // Shape drawing state
    ShapeMode m_shape_mode = ShapeMode::OUTLINE;
    bool m_is_drawing_shape = false;
    int m_shape_start_x = 0;
    int m_shape_start_y = 0;
    int m_shape_end_x = 0;
    int m_shape_end_y = 0;

    // Selection state
    bool m_has_selection = false;
    bool m_is_selecting = false;
    int m_selection_x1 = 0;
    int m_selection_y1 = 0;
    int m_selection_x2 = 0;
    int m_selection_y2 = 0;
    Glib::RefPtr<Gdk::Pixbuf> m_clipboard;  // For copy/paste

    // Crop state
    bool m_is_cropping = false;
    int m_crop_x1 = 0;
    int m_crop_y1 = 0;
    int m_crop_x2 = 0;
    int m_crop_y2 = 0;

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
    Gtk::MenuItem m_menu_file_save_as;
    Gtk::SeparatorMenuItem m_menu_file_separator;
    Gtk::MenuItem m_menu_file_quit;

    // Edit menu
    Gtk::MenuItem m_menu_edit;
    Gtk::Menu m_submenu_edit;
    Gtk::MenuItem m_menu_edit_undo;
    Gtk::MenuItem m_menu_edit_redo;
    Gtk::SeparatorMenuItem m_menu_edit_separator;
    Gtk::MenuItem m_menu_edit_copy;
    Gtk::MenuItem m_menu_edit_cut;
    Gtk::MenuItem m_menu_edit_paste;
    Gtk::MenuItem m_menu_edit_delete;

    // Image menu (transformations)
    Gtk::MenuItem m_menu_image;
    Gtk::Menu m_submenu_image;
    Gtk::MenuItem m_menu_image_rotate;
    Gtk::MenuItem m_menu_image_flip_h;
    Gtk::MenuItem m_menu_image_flip_v;
    Gtk::SeparatorMenuItem m_menu_image_separator;
    Gtk::MenuItem m_menu_image_scale;
    Gtk::MenuItem m_menu_image_crop;
    Gtk::SeparatorMenuItem m_menu_image_separator2;
    Gtk::MenuItem m_menu_image_brightness_contrast;

    // Filters menu
    Gtk::MenuItem m_menu_filters;
    Gtk::Menu m_submenu_filters;
    Gtk::MenuItem m_menu_filter_blur;
    Gtk::MenuItem m_menu_filter_sharpen;
    Gtk::SeparatorMenuItem m_menu_filter_separator;
    Gtk::MenuItem m_menu_filter_grayscale;
    Gtk::MenuItem m_menu_filter_invert;
    Gtk::MenuItem m_menu_filter_sepia;

    // Adjustments menu
    Gtk::MenuItem m_menu_adjustments;
    Gtk::Menu m_submenu_adjustments;
    Gtk::MenuItem m_menu_adjust_brightness_contrast;
    Gtk::MenuItem m_menu_adjust_gamma;
    Gtk::MenuItem m_menu_adjust_levels;
    Gtk::SeparatorMenuItem m_menu_adjust_separator;
    Gtk::MenuItem m_menu_adjust_hue_saturation;
    Gtk::MenuItem m_menu_adjust_exposure;
    Gtk::MenuItem m_menu_adjust_temperature;

    // Help menu
    Gtk::MenuItem m_menu_help;
    Gtk::Menu m_submenu_help;
    Gtk::MenuItem m_menu_help_about;

    // Toolbar
    Gtk::Box m_toolbar_box;
    Gtk::ToggleButton m_tool_pipette;
    Gtk::ToggleButton m_tool_brush;
    Gtk::ToggleButton m_tool_eraser;
    Gtk::ToggleButton m_tool_selection;
    Gtk::ToggleButton m_tool_line;
    Gtk::ToggleButton m_tool_rectangle;
    Gtk::ToggleButton m_tool_circle;
    Gtk::ToggleButton m_tool_crop;
    Gtk::ToggleButton m_shape_fill_toggle;
    Gtk::Label m_brush_size_label;
    Gtk::HScale m_brush_size_scale;

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

    // Undo/Redo system
    std::unique_ptr<CommandStack> m_commandStack;
};

} // namespace EpiGimp

#endif // EPIGIMP_MAINWINDOW_HPP
