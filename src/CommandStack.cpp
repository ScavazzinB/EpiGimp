#include "epigimp/CommandStack.hpp"
#include <iostream>

namespace EpiGimp {

CommandStack::CommandStack(size_t maxSize)
    : m_maxSize(maxSize)
{
}

void CommandStack::push(std::unique_ptr<Command> cmd)
{
    if (!cmd) return;

    // Clear redo stack when new command is pushed
    if (!m_redoStack.empty()) {
        m_redoStack.clear();
        // Invalidate clean state if it was in redo stack
        if (m_cleanIndex > static_cast<int>(m_undoStack.size())) {
            m_cleanIndex = -1;
            notifyCleanChanged();
        }
    }

    // Try to merge with the last command
    if (!m_undoStack.empty() && m_undoStack.back()->canMergeWith(cmd.get())) {
        m_undoStack.back()->mergeWith(cmd.get());
        // Don't execute - the merge already incorporated the changes
        notifyStackChanged();
        return;
    }

    // Execute the command
    cmd->execute();

    // Add to undo stack
    m_undoStack.push_back(std::move(cmd));

    // Trim stack if it exceeds max size
    if (m_maxSize > 0 && m_undoStack.size() > m_maxSize) {
        m_undoStack.erase(m_undoStack.begin());
        // Adjust clean index
        if (m_cleanIndex > 0) {
            m_cleanIndex--;
        } else if (m_cleanIndex == 0) {
            m_cleanIndex = -1;  // Clean state was trimmed
            notifyCleanChanged();
        }
    }

    notifyStackChanged();
}

void CommandStack::pushExecuted(std::unique_ptr<Command> cmd)
{
    if (!cmd) return;

    // Clear redo stack when new command is pushed
    if (!m_redoStack.empty()) {
        m_redoStack.clear();
        // Invalidate clean state if it was in redo stack
        if (m_cleanIndex > static_cast<int>(m_undoStack.size())) {
            m_cleanIndex = -1;
            notifyCleanChanged();
        }
    }

    // Add to undo stack WITHOUT executing (action already performed)
    m_undoStack.push_back(std::move(cmd));

    // Trim stack if it exceeds max size
    if (m_maxSize > 0 && m_undoStack.size() > m_maxSize) {
        m_undoStack.erase(m_undoStack.begin());
        // Adjust clean index
        if (m_cleanIndex > 0) {
            m_cleanIndex--;
        } else if (m_cleanIndex == 0) {
            m_cleanIndex = -1;  // Clean state was trimmed
            notifyCleanChanged();
        }
    }

    notifyStackChanged();
}

bool CommandStack::undo()
{
    if (m_undoStack.empty()) {
        return false;
    }

    bool wasClean = isClean();

    // Get the last command
    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    // Undo it
    cmd->undo();

    // Move to redo stack
    m_redoStack.push_back(std::move(cmd));

    notifyStackChanged();

    if (wasClean != isClean()) {
        notifyCleanChanged();
    }

    return true;
}

bool CommandStack::redo()
{
    if (m_redoStack.empty()) {
        return false;
    }

    bool wasClean = isClean();

    // Get the last undone command
    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    // Re-execute it
    cmd->execute();

    // Move back to undo stack
    m_undoStack.push_back(std::move(cmd));

    notifyStackChanged();

    if (wasClean != isClean()) {
        notifyCleanChanged();
    }

    return true;
}

std::string CommandStack::undoText() const
{
    if (m_undoStack.empty()) {
        return "";
    }
    return m_undoStack.back()->description();
}

std::string CommandStack::redoText() const
{
    if (m_redoStack.empty()) {
        return "";
    }
    return m_redoStack.back()->description();
}

void CommandStack::clear()
{
    bool wasClean = isClean();

    m_undoStack.clear();
    m_redoStack.clear();
    m_cleanIndex = 0;

    notifyStackChanged();

    if (!wasClean) {
        notifyCleanChanged();
    }
}

void CommandStack::setClean()
{
    bool wasClean = isClean();
    m_cleanIndex = static_cast<int>(m_undoStack.size());

    if (!wasClean) {
        notifyCleanChanged();
    }
}

bool CommandStack::isClean() const
{
    return m_cleanIndex == static_cast<int>(m_undoStack.size());
}

void CommandStack::notifyStackChanged()
{
    if (m_onStackChanged) {
        m_onStackChanged();
    }
}

void CommandStack::notifyCleanChanged()
{
    if (m_onCleanChanged) {
        m_onCleanChanged(isClean());
    }
}

} // namespace EpiGimp
