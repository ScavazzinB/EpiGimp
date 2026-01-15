#include "epigimp/MainWindow.hpp"
#include <gtkmm/application.h>
#include <iostream>

int main(int argc, char* argv[])
{
    // Create GTK application
    auto app = Gtk::Application::create(argc, argv, "org.epigimp.app");

    // Create main window
    EpiGimp::MainWindow window;

    // Run the application
    // The window will be shown and the main loop will run
    // Returns when the window is closed
    return app->run(window);
}
