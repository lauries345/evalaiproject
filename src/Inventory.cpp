#include "Inventory.h"

#include "IInventoryObserver.h"

#include <algorithm>
#include <stdexcept>

/**
 * @file Inventory.cpp
 * @brief Implementation of eval::Inventory.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 *
 * @par Safety Classification
 * IEC 62304 Class A. Change-controlled per project SOP.
 */

namespace eval
{

bool Inventory::add(const Item& item)
{
    const auto it = std::find_if(m_items.begin(), m_items.end(),
        [&](const Item& current) { return current.getName() == item.getName(); });

    if (it == m_items.end())
    {
        m_items.push_back(item);
        if (m_items.back().isLowStock())
        {
            notifyLowStock(m_items.back());
        }
        return true;
    }

    it->adjustQuantity(item.getQuantity());
    return false;
}

bool Inventory::remove(const std::string& name)
{
    const auto it = std::find_if(m_items.begin(), m_items.end(),
        [&](const Item& current) { return current.getName() == name; });
    if (it == m_items.end())
    {
        return false;
    }
    m_items.erase(it);
    return true;
}

std::optional<Item> Inventory::find(const std::string& name) const
{
    const auto it = std::find_if(m_items.cbegin(), m_items.cend(),
        [&](const Item& current) { return current.getName() == name; });
    if (it == m_items.cend())
    {
        return std::nullopt;
    }
    return *it;
}

std::size_t Inventory::size() const noexcept
{
    return m_items.size();
}

const std::vector<Item>& Inventory::items() const noexcept
{
    return m_items;
}

void Inventory::attach(IInventoryObserver* obs)
{
    if (obs == nullptr)
    {
        throw std::invalid_argument("Inventory::attach: observer must not be null");
    }
    m_observers.push_back(obs);
}

void Inventory::detach(IInventoryObserver* obs) noexcept
{
    m_observers.erase(
        std::remove(m_observers.begin(), m_observers.end(), obs),
        m_observers.end());
}

void Inventory::notifyLowStock(const Item& item) const
{
    for (auto* obs : m_observers)
    {
        obs->onLowStock(item);
    }
}

} // namespace eval
