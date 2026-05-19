#pragma once

#include <string>

/**
 * @file ICommand.h
 * @brief Abstract command interface for the GoF Command pattern.
 * @details Implementations encapsulate a single reversible mutation of
 *          Inventory state. Owned via std::unique_ptr<ICommand> by
 *          CommandHistory.
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
 * @brief Polymorphic command interface (GoF Command).
 *
 * @invariant Implementations must satisfy: undo() restores the state that
 *            existed immediately before the matching execute() call.
 */
class ICommand
{
public:
    ICommand()                           = default;
    ICommand(const ICommand&)            = delete;
    ICommand& operator=(const ICommand&) = delete;
    ICommand(ICommand&&)                 = delete;
    ICommand& operator=(ICommand&&)      = delete;
    virtual ~ICommand()                  = default;

    /** @brief Apply the command's effect. */
    virtual void execute() = 0;

    /** @brief Reverse the most recent execute() exactly. */
    virtual void undo() = 0;

    /** @brief Human-readable description, for logging/auditing. */
    [[nodiscard]] virtual std::string description() const = 0;
};

} // namespace eval
