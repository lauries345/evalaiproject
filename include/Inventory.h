#pragma once

#include "Item.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

/**
 * @file Inventory.h
 * @brief Container/manager of Item records with observer notification.
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

class IInventoryObserver;
class ICommand; ///< forward-declared per plan to avoid circular include

/**
 * @brief Owns a collection of Item records keyed by name.
 *
 * Non-copyable to make ownership unambiguous; moves are allowed.
 *
 * @invariant No two stored items share the same name.
 */
class Inventory
{
public:
    Inventory()                                = default;
    Inventory(const Inventory&)                = delete;
    Inventory& operator=(const Inventory&)     = delete;
    Inventory(Inventory&&) noexcept            = default;
    Inventory& operator=(Inventory&&) noexcept = default;
    ~Inventory()                               = default;

    /**
     * @brief Insert a new Item or merge its quantity into an existing record.
     * @param item the record to add; non-empty name and non-negative fields.
     * @return true if a brand-new record was created;
     *         false if an existing record's quantity was incremented.
     * @note Fires onLowStock() iff a new record is inserted at low stock.
     */
    bool add(const Item& item);

    /**
     * @brief Erase a record by name.
     * @param name product identifier to remove.
     * @return true if an item was removed; false if no such name existed.
     */
    bool remove(const std::string& name);

    /** @brief Look up by name. */
    [[nodiscard]] std::optional<Item> find(const std::string& name) const;

    [[nodiscard]] std::size_t              size()  const noexcept;
    [[nodiscard]] const std::vector<Item>& items() const noexcept;

    /**
     * @brief Attach a low-stock observer.
     * @throws std::invalid_argument if obs is nullptr.
     */
    void attach(IInventoryObserver* obs);

    /** @brief Detach an observer; no-op if not currently attached. */
    void detach(IInventoryObserver* obs) noexcept;

private:
    std::vector<Item>                m_items;
    std::vector<IInventoryObserver*> m_observers;

    void notifyLowStock(const Item& item) const;
};

} // namespace eval
