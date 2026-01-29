#ifndef EPIGIMP_IMAGECOMMAND_HPP
#define EPIGIMP_IMAGECOMMAND_HPP

#include "Command.hpp"
#include <gdkmm/pixbuf.h>
#include <functional>

namespace EpiGimp {

/**
 * @brief Command for image modifications
 *
 * Stores the image state before and after a modification.
 * Used for operations like brush strokes, eraser, transformations, etc.
 */
class ImageCommand : public Command {
public:
    /**
     * @brief Construct an ImageCommand
     * @param description Human-readable description
     * @param beforeState The image state before the change
     * @param afterState The image state after the change
     * @param onRestore Callback to update the application's image reference
     */
    ImageCommand(const std::string& description,
                 Glib::RefPtr<Gdk::Pixbuf> beforeState,
                 Glib::RefPtr<Gdk::Pixbuf> afterState,
                 std::function<void(Glib::RefPtr<Gdk::Pixbuf>)> onRestore);

    void execute() override;
    void undo() override;
    std::string description() const override { return m_description; }

private:
    std::string m_description;
    Glib::RefPtr<Gdk::Pixbuf> m_beforeState;
    Glib::RefPtr<Gdk::Pixbuf> m_afterState;
    std::function<void(Glib::RefPtr<Gdk::Pixbuf>)> m_onRestore;
};

/**
 * @brief Command for region-based image modifications
 *
 * More memory-efficient version that only stores the affected region.
 * Useful for brush strokes and small edits.
 */
class RegionImageCommand : public Command {
public:
    /**
     * @brief Construct a RegionImageCommand
     * @param description Human-readable description
     * @param region The affected region (x, y, width, height)
     * @param beforeRegion The region data before the change
     * @param afterRegion The region data after the change
     * @param onRestore Callback to apply a region to the image
     */
    RegionImageCommand(const std::string& description,
                       int x, int y, int width, int height,
                       Glib::RefPtr<Gdk::Pixbuf> beforeRegion,
                       Glib::RefPtr<Gdk::Pixbuf> afterRegion,
                       std::function<void(int, int, Glib::RefPtr<Gdk::Pixbuf>)> onRestore);

    void execute() override;
    void undo() override;
    std::string description() const override { return m_description; }

    // For merging brush strokes
    bool canMergeWith(const Command* other) const override;
    void mergeWith(Command* other) override;

private:
    std::string m_description;
    int m_x, m_y, m_width, m_height;
    Glib::RefPtr<Gdk::Pixbuf> m_beforeRegion;
    Glib::RefPtr<Gdk::Pixbuf> m_afterRegion;
    std::function<void(int, int, Glib::RefPtr<Gdk::Pixbuf>)> m_onRestore;
};

} // namespace EpiGimp

#endif // EPIGIMP_IMAGECOMMAND_HPP
