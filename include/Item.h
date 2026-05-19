#pragma once

#include <string>

/**
 * @file Item.h
 * @brief Value type representing a single inventory record.
 * @details Pure-value semantics: copyable and movable, no resource ownership
 *          beyond std::string. Safe to store directly in std::vector.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 *
 * @par Revision History
 * | Ver | Date       | Author | Notes              |
 * |-----|------------|--------|--------------------|
 * | 1.0 | 2026-05-18 | init   | Initial extraction |
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
 * @brief Represents one stock-keeping unit (SKU).
 *
 * @invariant !m_name.empty()
 * @invariant m_quantity           >= 0
 * @invariant m_lowStockThreshold  >= 0
 * @invariant m_unitPrice          >= 0.0
 */
class Item
{
public:
    /**
     * @brief Construct a fully populated SKU record.
     * @param name              non-empty product identifier
     * @param quantity          current on-hand quantity (>= 0)
     * @param lowStockThreshold reorder threshold (>= 0)
     * @param unitPrice         per-unit price in major currency units (>= 0)
     * @throws std::invalid_argument if any precondition is violated.
     */
    Item(std::string name,
         int         quantity,
         int         lowStockThreshold,
         double      unitPrice);

    Item(const Item&)                = default;
    Item(Item&&) noexcept            = default;
    Item& operator=(const Item&)     = default;
    Item& operator=(Item&&) noexcept = default;
    ~Item()                          = default;

    [[nodiscard]] const std::string& getName()              const noexcept;
    [[nodiscard]] int                getQuantity()          const noexcept;
    [[nodiscard]] int                getLowStockThreshold() const noexcept;
    [[nodiscard]] double             getUnitPrice()         const noexcept;

    /** @brief True iff getQuantity() <= getLowStockThreshold(). */
    [[nodiscard]] bool isLowStock() const noexcept;

    /**
     * @brief Mutate quantity by a signed delta.
     * @param delta amount to add (negative values decrement).
     * @throws std::out_of_range if the resulting quantity would be negative.
     * @note On throw the object is unchanged (strong guarantee).
     */
    void adjustQuantity(int delta);

private:
    std::string m_name;
    int         m_quantity;
    int         m_lowStockThreshold;
    double      m_unitPrice;
};

} // namespace eval
