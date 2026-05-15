/**
 * @file        Item.cpp
 * @brief       Item entity implementation.
 * @details     Provides definitions for all Item member functions declared
 *              in Item.h.
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

#include "Item.h"

#include <utility>

namespace eval {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Item::Item(std::string name, int quantity)
    : m_name(std::move(name))
    , m_quantity(quantity)
{
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

const std::string& Item::getName() const noexcept
{
    return m_name;
}

int Item::getQuantity() const noexcept
{
    return m_quantity;
}

// ---------------------------------------------------------------------------
// Mutators
// ---------------------------------------------------------------------------

void Item::adjustQuantity(int delta) noexcept
{
    m_quantity += delta;
}

} // namespace eval
