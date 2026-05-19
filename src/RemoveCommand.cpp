#include "RemoveCommand.h"

#include "Inventory.h"

#include <utility>

/**
 * @file RemoveCommand.cpp
 * @brief Implementation of eval::RemoveCommand.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 */

namespace eval
{

RemoveCommand::RemoveCommand(Inventory& inv, std::string name)
    : m_inv(inv)
    , m_name(std::move(name))
{
}

void RemoveCommand::execute()
{
    m_snapshot = m_inv.find(m_name);
    if (m_snapshot.has_value())
    {
        m_inv.remove(m_name);
    }
}

void RemoveCommand::undo()
{
    if (m_snapshot.has_value())
    {
        m_inv.add(*m_snapshot);
    }
}

std::string RemoveCommand::description() const
{
    return "Remove " + m_name;
}

} // namespace eval
