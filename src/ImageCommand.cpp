#include "epigimp/ImageCommand.hpp"

namespace EpiGimp {

// === ImageCommand ===

ImageCommand::ImageCommand(const std::string& description,
                           Glib::RefPtr<Gdk::Pixbuf> beforeState,
                           Glib::RefPtr<Gdk::Pixbuf> afterState,
                           std::function<void(Glib::RefPtr<Gdk::Pixbuf>)> onRestore)
    : m_description(description)
    , m_beforeState(beforeState)
    , m_afterState(afterState)
    , m_onRestore(onRestore)
{
}

void ImageCommand::execute()
{
    if (m_onRestore && m_afterState) {
        m_onRestore(m_afterState->copy());
    }
}

void ImageCommand::undo()
{
    if (m_onRestore && m_beforeState) {
        m_onRestore(m_beforeState->copy());
    }
}

// === RegionImageCommand ===

RegionImageCommand::RegionImageCommand(const std::string& description,
                                       int x, int y, int width, int height,
                                       Glib::RefPtr<Gdk::Pixbuf> beforeRegion,
                                       Glib::RefPtr<Gdk::Pixbuf> afterRegion,
                                       std::function<void(int, int, Glib::RefPtr<Gdk::Pixbuf>)> onRestore)
    : m_description(description)
    , m_x(x), m_y(y), m_width(width), m_height(height)
    , m_beforeRegion(beforeRegion)
    , m_afterRegion(afterRegion)
    , m_onRestore(onRestore)
{
}

void RegionImageCommand::execute()
{
    if (m_onRestore && m_afterRegion) {
        m_onRestore(m_x, m_y, m_afterRegion);
    }
}

void RegionImageCommand::undo()
{
    if (m_onRestore && m_beforeRegion) {
        m_onRestore(m_x, m_y, m_beforeRegion);
    }
}

bool RegionImageCommand::canMergeWith(const Command* other) const
{
    // Only merge commands with the same description (same tool operation)
    auto* otherRegion = dynamic_cast<const RegionImageCommand*>(other);
    if (!otherRegion) return false;

    return m_description == otherRegion->m_description;
}

void RegionImageCommand::mergeWith(Command* other)
{
    auto* otherRegion = dynamic_cast<RegionImageCommand*>(other);
    if (!otherRegion) return;

    // Expand our region to include the other region
    int newX = std::min(m_x, otherRegion->m_x);
    int newY = std::min(m_y, otherRegion->m_y);
    int newRight = std::max(m_x + m_width, otherRegion->m_x + otherRegion->m_width);
    int newBottom = std::max(m_y + m_height, otherRegion->m_y + otherRegion->m_height);

    m_x = newX;
    m_y = newY;
    m_width = newRight - newX;
    m_height = newBottom - newY;

    // Update the after state to the other's after state
    m_afterRegion = otherRegion->m_afterRegion;
}

} // namespace EpiGimp
