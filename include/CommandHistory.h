#pragma once

#include "ICommand.h"

#include <cstddef>
#include <memory>
#include <vector>

/**
 * @file CommandHistory.h
 * @brief Undo/redo stack manager for ICommand instances.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 *
 * @par Safety Classification
 * IEC 62304 Class A. Change-controlled per project SOP.
 *
 * @par Applicable Standards
 * IEC 62304, ISO 14971, MISRA C++ 2008, ISO/IEC 14882:2017
 */

namespace eval
{

/**
 * @brief Two-stack undo/redo orchestrator (classic editor semantics).
 *
 * Owns every ICommand passed to execute() via std::unique_ptr.
 * Executing a fresh command clears the redo stack, matching user expectations.
 */
class CommandHistory
{
public:
    CommandHistory()                                     = default;
    CommandHistory(const CommandHistory&)                = delete;
    CommandHistory& operator=(const CommandHistory&)     = delete;
    CommandHistory(CommandHistory&&) noexcept            = default;
    CommandHistory& operator=(CommandHistory&&) noexcept = default;
    ~CommandHistory()                                    = default;

    /**
     * @brief Run @p cmd and push it onto the undo stack; clears redo.
     * @throws std::invalid_argument if @p cmd is null.
     */
    void execute(std::unique_ptr<ICommand> cmd);

    /**
     * @brief Pop the top of the undo stack, call undo(), push to redo.
     * @return true if a command was undone; false if the undo stack was empty.
     */
    bool undo();

    /**
     * @brief Pop the top of the redo stack, re-execute, push back to undo.
     * @return true if a command was redone; false if the redo stack was empty.
     */
    bool redo();

    [[nodiscard]] bool        canUndo()   const noexcept;
    [[nodiscard]] bool        canRedo()   const noexcept;
    [[nodiscard]] std::size_t undoDepth() const noexcept;
    [[nodiscard]] std::size_t redoDepth() const noexcept;

    /** @brief Drop both stacks. */
    void clear() noexcept;

private:
    std::vector<std::unique_ptr<ICommand>> m_undo;
    std::vector<std::unique_ptr<ICommand>> m_redo;
};

} // namespace eval
