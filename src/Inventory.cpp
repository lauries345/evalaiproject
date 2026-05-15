/**
 * @file        Inventory.cpp
 * @brief       Inventory container implementation.
 * @details     Provides definitions for all Inventory member functions
 *              declared in Inventory.h.
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

#include "Inventory.h"

#include <algorithm>
#include <iostream>
#include <numeric>

namespace eval {

// ---------------------------------------------------------------------------
// Mutators
// ---------------------------------------------------------------------------

void Inventory::add(const std::string& name, int quantity)
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&name](const Item& item) { return item.getName() == name; });

    if (it != m_items.end())
    {
        it->adjustQuantity(quantity);
    }
    else
    {
        m_items.emplace_back(name, quantity);
    }
}

bool Inventory::remove(const std::string& name)
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&name](const Item& item) { return item.getName() == name; });

    if (it == m_items.end())
    {
        return false;
    }

    m_items.erase(it);
    return true;
}

void Inventory::sortByName()
{
    std::sort(
        m_items.begin(), m_items.end(),
        [](const Item& lhs, const Item& rhs)
        {
            return lhs.getName() < rhs.getName();
        });
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int Inventory::totalQuantity() const noexcept
{
    int total = 0;
    for (const auto& item : m_items)
    {
        total += item.getQuantity();
    }
    return total;
}

std::size_t Inventory::size() const noexcept
{
    return m_items.size();
}

// ---------------------------------------------------------------------------
// Output
// ---------------------------------------------------------------------------

void Inventory::print() const
{
    for (const auto& item : m_items)
    {
        std::cout << "  " << item.getName() << ": " << item.getQuantity() << "\n";
    }
}

} // namespace eval
