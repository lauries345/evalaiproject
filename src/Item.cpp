#include "Item.h"

#include <stdexcept>
#include <utility>

/**
 * @file Item.cpp
 * @brief Implementation of eval::Item.
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

Item::Item(std::string name,
           int         quantity,
           int         lowStockThreshold,
           double      unitPrice)
    : m_name(std::move(name))
    , m_quantity(quantity)
    , m_lowStockThreshold(lowStockThreshold)
    , m_unitPrice(unitPrice)
{
    if (m_name.empty())
    {
        throw std::invalid_argument("Item: name must not be empty");
    }
    if (m_quantity < 0)
    {
        throw std::invalid_argument("Item: quantity must be non-negative");
    }
    if (m_lowStockThreshold < 0)
    {
        throw std::invalid_argument("Item: lowStockThreshold must be non-negative");
    }
    if (m_unitPrice < 0.0)
    {
        throw std::invalid_argument("Item: unitPrice must be non-negative");
    }
}

const std::string& Item::getName() const noexcept
{
    return m_name;
}

int Item::getQuantity() const noexcept
{
    return m_quantity;
}

int Item::getLowStockThreshold() const noexcept
{
    return m_lowStockThreshold;
}

double Item::getUnitPrice() const noexcept
{
    return m_unitPrice;
}

bool Item::isLowStock() const noexcept
{
    return m_quantity <= m_lowStockThreshold;
}

void Item::adjustQuantity(int delta)
{
    const int updated = m_quantity + delta;
    if (updated < 0)
    {
        throw std::out_of_range("Item::adjustQuantity would yield a negative quantity");
    }
    m_quantity = updated;
}

} // namespace eval
