/**
 * @file        Command.cpp
 * @brief       Command pattern implementation (AddCommand, RemoveCommand).
 * @details     Provides definitions for all ICommand subclass member functions
 *              declared in Command.h. Each concrete command calls through the
 *              public Inventory interface so no friend access is required.
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     1.0.0
 *
 * @copyright   Copyright (c) 2026. All rights reserved.
 *
 * @par Revision History:
 * | Version | Date       | Author      | Description     |
 * |---------|------------|-------------|-----------------|
 * | 1.0.0   | 2026-05-15 | Seth Laurie | Initial release |
 *
 * @par Safety Classification:
 * Evaluation / non-production component. Must be formally reclassified as
 * IEC 62304 Class A, B, or C before integration into a regulated medical
 * device. All changes require documented change control.
 */

#include "Command.h"
#include "Inventory.h"

#include <utility>

namespace eval {

// ===========================================================================
// AddCommand
// ===========================================================================

AddCommand::AddCommand(Inventory& inv, std::string name, int qty)
    : m_inv(inv)
    , m_name(std::move(name))
    , m_qty(qty)
{
}

void AddCommand::execute()
{
    // Determine if this will be a new insertion or a merge, so undo() knows
    // which strategy to use.
    m_wasNew = (m_inv.findByName(m_name) == nullptr);
    m_inv.add(m_name, m_qty);
}

void AddCommand::undo()
{
    if (m_wasNew)
    {
        // The item was freshly inserted — remove it entirely.
        m_inv.remove(m_name);
    }
    else
    {
        // The item existed and had its quantity merged — reverse the delta.
        m_inv.add(m_name, -m_qty);
    }
}

// ===========================================================================
// RemoveCommand
// ===========================================================================

RemoveCommand::RemoveCommand(Inventory& inv, std::string name)
    : m_inv(inv)
    , m_name(std::move(name))
{
}

void RemoveCommand::execute()
{
    // Snapshot the item's full state before removal so undo() can restore it.
    const Item* item = m_inv.findByName(m_name);
    if (item != nullptr)
    {
        m_found           = true;
        m_savedQty        = item->getQuantity();
        m_savedCategory   = item->getCategory();
        m_savedLotNumber  = item->getLotNumber();
        m_savedExpiryDate = item->getExpiryDate();
        m_savedUnitPrice  = item->getUnitPrice();
    }
    m_inv.remove(m_name);
}

void RemoveCommand::undo()
{
    if (!m_found)
    {
        return; // Item did not exist before execute() — nothing to restore.
    }

    // Re-insert the item with all its original fields preserved.
    m_inv.add(Item{m_name,
                   m_savedQty,
                   m_savedCategory,
                   m_savedLotNumber,
                   m_savedExpiryDate,
                   m_savedUnitPrice});
}

} // namespace eval
