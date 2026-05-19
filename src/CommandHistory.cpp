#include "CommandHistory.h"

#include <stdexcept>
#include <utility>

/**
 * @file CommandHistory.cpp
 * @brief Implementation of eval::CommandHistory.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 */

namespace eval
{

void CommandHistory::execute(std::unique_ptr<ICommand> cmd)
{
    if (!cmd)
    {
        throw std::invalid_argument("CommandHistory::execute: command must not be null");
    }
    cmd->execute();
    m_undo.push_back(std::move(cmd));
    m_redo.clear();
}

bool CommandHistory::undo()
{
    if (m_undo.empty())
    {
        return false;
    }
    auto cmd = std::move(m_undo.back());
    m_undo.pop_back();
    cmd->undo();
    m_redo.push_back(std::move(cmd));
    return true;
}

bool CommandHistory::redo()
{
    if (m_redo.empty())
    {
        return false;
    }
    auto cmd = std::move(m_redo.back());
    m_redo.pop_back();
    cmd->execute();
    m_undo.push_back(std::move(cmd));
    return true;
}

bool CommandHistory::canUndo() const noexcept
{
    return !m_undo.empty();
}

bool CommandHistory::canRedo() const noexcept
{
    return !m_redo.empty();
}

std::size_t CommandHistory::undoDepth() const noexcept
{
    return m_undo.size();
}

std::size_t CommandHistory::redoDepth() const noexcept
{
    return m_redo.size();
}

void CommandHistory::clear() noexcept
{
    m_undo.clear();
    m_redo.clear();
}

} // namespace eval
