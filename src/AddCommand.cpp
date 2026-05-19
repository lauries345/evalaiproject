#include "AddCommand.h"

#include "Inventory.h"

#include <sstream>
#include <utility>

/**
 * @file AddCommand.cpp
 * @brief Implementation of eval::AddCommand.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 */

namespace eval
{

AddCommand::AddCommand(Inventory& inv, Item item)
    : m_inv(inv)
    , m_item(std::move(item))
{
}

void AddCommand::execute()
{
    m_priorSnapshot = m_inv.find(m_item.getName());
    m_inv.add(m_item);
}

void AddCommand::undo()
{
    m_inv.remove(m_item.getName());
    if (m_priorSnapshot.has_value())
    {
        m_inv.add(*m_priorSnapshot);
    }
}

std::string AddCommand::description() const
{
    std::ostringstream oss;
    oss << "Add " << m_item.getQuantity() << " " << m_item.getName();
    return oss.str();
}

} // namespace eval
