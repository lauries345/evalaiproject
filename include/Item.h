/**
 * @file        Item.h
 * @brief       Item entity declaration.
 * @details     Declares the Item class, which represents a single named
 *              inventory entry with an associated integer quantity, product
 *              category, lot number, expiry date, and unit price.
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     2.0.0
 *
 * @copyright   Copyright (c) 2026. All rights reserved.
 *
 * @par Revision History:
 * | Version | Date       | Author      | Description                        |
 * |---------|------------|-------------|------------------------------------|
 * | 1.0.0   | 2026-05-15 | Seth Laurie | Initial release                    |
 * | 2.0.0   | 2026-05-15 | Seth Laurie | Add category, lotNumber, expiryDate, unitPrice |
 *
 * @par Safety Classification:
 * Evaluation / non-production component. Must be formally reclassified as
 * IEC 62304 Class A, B, or C before integration into a regulated medical
 * device. All changes require documented change control.
 *
 * @par Applicable Standards:
 * - IEC 62304: Medical device software – Software life cycle processes
 * - ISO 14971: Medical devices – Application of risk management to medical devices
 * - MISRA C++ 2008: Guidelines for the use of the C++ language in critical systems
 */

#pragma once

#include <string>

namespace eval {

/**
 * @class   Item
 * @brief   Represents a single inventory item with traceability metadata.
 *
 * @details An Item pairs a human-readable name with a signed integer quantity
 *          and optional traceability fields (category, lot number, expiry date,
 *          unit price). All metadata fields default to empty / zero when not
 *          supplied, preserving backward compatibility with callers that only
 *          need name and quantity.
 *
 * @invariant getName() returns a non-empty string after construction.
 * @invariant getQuantity() reflects the sum of all prior adjustQuantity()
 *            calls applied to the initial quantity supplied at construction.
 */
class Item
{
public:
    // -----------------------------------------------------------------------
    // Construction / destruction
    // -----------------------------------------------------------------------

    /**
     * @brief   Constructs an Item with name, quantity, and optional metadata.
     * @param   name        Human-readable item identifier. Must not be empty.
     * @param   quantity    Starting stock count (may be zero or negative).
     * @param   category    Product category (e.g. "Device", "Reagent"). May be empty.
     * @param   lotNumber   Manufacturer lot / batch number for traceability. May be empty.
     * @param   expiryDate  Expiry date in ISO 8601 format (YYYY-MM-DD). May be empty.
     * @param   unitPrice   Cost per unit in the account currency. Defaults to 0.0.
     */
    explicit Item(std::string name,
                  int         quantity,
                  std::string category   = {},
                  std::string lotNumber  = {},
                  std::string expiryDate = {},
                  double      unitPrice  = 0.0);

    // Compiler-generated copy, move, and destructor are sufficient.
    Item(const Item&)            = default;
    Item(Item&&)                 = default;
    Item& operator=(const Item&) = default;
    Item& operator=(Item&&)      = default;
    ~Item()                      = default;

    // -----------------------------------------------------------------------
    // Accessors
    // -----------------------------------------------------------------------

    /** @brief Returns the item's name. */
    [[nodiscard]] const std::string& getName()       const noexcept;

    /** @brief Returns the current quantity. */
    [[nodiscard]] int                getQuantity()   const noexcept;

    /** @brief Returns the product category (empty string if unset). */
    [[nodiscard]] const std::string& getCategory()   const noexcept;

    /** @brief Returns the lot / batch number (empty string if unset). */
    [[nodiscard]] const std::string& getLotNumber()  const noexcept;

    /** @brief Returns the expiry date string in YYYY-MM-DD format (empty if unset). */
    [[nodiscard]] const std::string& getExpiryDate() const noexcept;

    /** @brief Returns the unit price (0.0 if unset). */
    [[nodiscard]] double             getUnitPrice()  const noexcept;

    // -----------------------------------------------------------------------
    // Mutators
    // -----------------------------------------------------------------------

    /**
     * @brief   Adjusts the quantity by a signed delta.
     * @param   delta  Amount to add (positive) or subtract (negative).
     * @note    No overflow or domain checking is performed. Callers are
     *          responsible for ensuring the result remains semantically valid.
     */
    void adjustQuantity(int delta) noexcept;

private:
    std::string m_name;        ///< Human-readable identifier; immutable after construction.
    int         m_quantity;    ///< Current stock count; modified via adjustQuantity().
    std::string m_category;    ///< Product category for grouping and filtering.
    std::string m_lotNumber;   ///< Manufacturer lot number for IEC 62304 traceability.
    std::string m_expiryDate;  ///< Expiry date (YYYY-MM-DD); empty if not applicable.
    double      m_unitPrice;   ///< Cost per unit; used by Inventory::totalValue().
};

} // namespace eval
