#ifndef EPIGIMP_COMMAND_HPP
#define EPIGIMP_COMMAND_HPP

#include <string>
#include <memory>

namespace EpiGimp {

/**
 * @brief Abstract base class for all undoable commands
 *
 * Implements the Command Pattern for undo/redo functionality.
 * Each command encapsulates an action and its reverse.
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * @brief Execute the command (also used for redo)
     */
    virtual void execute() = 0;

    /**
     * @brief Undo the command, reverting to the previous state
     */
    virtual void undo() = 0;

    /**
     * @brief Get a human-readable description of the command
     * @return Description string (e.g., "Brush Stroke", "Rotate 90°")
     */
    virtual std::string description() const = 0;

    /**
     * @brief Check if this command can be merged with another
     *
     * Used for command compression (e.g., merging multiple small brush strokes)
     * @param other The command to potentially merge with
     * @return true if commands can be merged
     */
    virtual bool canMergeWith(const Command* other) const {
        (void)other;
        return false;
    }

    /**
     * @brief Merge another command into this one
     *
     * Called only if canMergeWith() returned true
     * @param other The command to merge
     */
    virtual void mergeWith(Command* other) {
        (void)other;
    }
};

} // namespace EpiGimp

#endif // EPIGIMP_COMMAND_HPP
