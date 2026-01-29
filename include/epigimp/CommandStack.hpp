#ifndef EPIGIMP_COMMANDSTACK_HPP
#define EPIGIMP_COMMANDSTACK_HPP

#include "Command.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace EpiGimp {

/**
 * @brief Manages the undo/redo history stack
 *
 * Provides a stack-based system for tracking and executing undo/redo operations.
 * Supports command compression, undo limits, and clean state tracking.
 */
class CommandStack {
public:
    /**
     * @brief Construct a new CommandStack
     * @param maxSize Maximum number of commands to store (0 = unlimited)
     */
    explicit CommandStack(size_t maxSize = 50);

    /**
     * @brief Push a new command onto the stack and execute it
     *
     * This will:
     * 1. Clear any redo history
     * 2. Try to merge with the previous command if possible
     * 3. Execute the command
     * 4. Add to undo stack (trimming if necessary)
     *
     * @param cmd The command to execute and store
     */
    void push(std::unique_ptr<Command> cmd);

    /**
     * @brief Push a command that has already been executed
     *
     * Use this when the action has already been performed
     * and you just need to record it for undo/redo.
     *
     * @param cmd The command to store (will NOT be executed)
     */
    void pushExecuted(std::unique_ptr<Command> cmd);

    /**
     * @brief Undo the last command
     * @return true if undo was performed, false if nothing to undo
     */
    bool undo();

    /**
     * @brief Redo the last undone command
     * @return true if redo was performed, false if nothing to redo
     */
    bool redo();

    /**
     * @brief Check if undo is available
     */
    bool canUndo() const { return !m_undoStack.empty(); }

    /**
     * @brief Check if redo is available
     */
    bool canRedo() const { return !m_redoStack.empty(); }

    /**
     * @brief Get description of next undo action
     * @return Description string, or empty if no undo available
     */
    std::string undoText() const;

    /**
     * @brief Get description of next redo action
     * @return Description string, or empty if no redo available
     */
    std::string redoText() const;

    /**
     * @brief Clear all undo/redo history
     */
    void clear();

    /**
     * @brief Mark the current state as "clean" (saved)
     */
    void setClean();

    /**
     * @brief Check if the stack is in clean state
     * @return true if no unsaved changes
     */
    bool isClean() const;

    /**
     * @brief Get the number of commands in the undo stack
     */
    size_t undoCount() const { return m_undoStack.size(); }

    /**
     * @brief Get the number of commands in the redo stack
     */
    size_t redoCount() const { return m_redoStack.size(); }

    /**
     * @brief Set callback for when undo/redo availability changes
     */
    void setOnStackChanged(std::function<void()> callback) {
        m_onStackChanged = callback;
    }

    /**
     * @brief Set callback for when clean state changes
     */
    void setOnCleanChanged(std::function<void(bool)> callback) {
        m_onCleanChanged = callback;
    }

private:
    void notifyStackChanged();
    void notifyCleanChanged();

    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
    size_t m_maxSize;

    // Clean state tracking
    int m_cleanIndex = 0;  // Index in undo stack when last saved (-1 if never saved or invalidated)

    // Callbacks
    std::function<void()> m_onStackChanged;
    std::function<void(bool)> m_onCleanChanged;
};

} // namespace EpiGimp

#endif // EPIGIMP_COMMANDSTACK_HPP
