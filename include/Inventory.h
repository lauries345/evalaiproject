/**
 * @file        Inventory.h
 * @brief       Inventory container declaration.
 * @details     Declares the Inventory class, which manages an ordered
 *              collection of Item objects and exposes add, remove, sort,
 *              and query operations.
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
 *
 * @par Applicable Standards:
 * - IEC 62304: Medical device software – Software life cycle processes
 * - ISO 14971: Medical devices – Application of risk management to medical devices
 * - MISRA C++ 2008: Guidelines for the use of the C++ language in critical systems
 */

#pragma once

#include "Item.h"

#include <cstddef>
#include <string>
#include <vector>

namespace eval {

/**
 * @class   Inventory
 * @brief   Manages a mutable, ordered collection of Item objects.
 *
 * @details Inventory provides add, remove, sort, and aggregate-query
 *          operations over an internal std::vector of Item instances. Items
 *          are identified by name; duplicate adds accumulate quantity into the
 *          existing record rather than creating a second entry.
 *
 * @note    Not thread-safe. External synchronisation is required when multiple
 *          threads share the same Inventory instance.
 *
 * @note    Memory allocation may occur during add() and sortByName(). In
 *          safety-critical contexts, verify that the underlying allocator
 *          satisfies real-time and failure-mode requirements.
 */
class Inventory
{
public:
    // -----------------------------------------------------------------------
    // Construction / destruction
    // -----------------------------------------------------------------------

    Inventory()                              = default;
    Inventory(const Inventory&)              = default;
    Inventory(Inventory&&)                   = default;
    Inventory& operator=(const Inventory&)   = default;
    Inventory& operator=(Inventory&&)        = default;
    ~Inventory()                             = default;

    // -----------------------------------------------------------------------
    // Mutators
    // -----------------------------------------------------------------------

    /**
     * @brief   Adds stock to an existing item or inserts a new entry.
     * @details If an item whose name matches @p name already exists, its
     *          quantity is increased by @p quantity. Otherwise a new Item is
     *          appended to the internal list.
     * @param   name      Name of the item to add or update. Must not be empty.
     * @param   quantity  Amount of stock to add. May be zero or negative.
     */
    void add(const std::string& name, int quantity);

    /**
     * @brief   Removes the first item whose name matches @p name.
     * @param   name  Name of the item to remove.
     * @return  @c true  if an item was found and removed.
     * @return  @c false if no item with the given name exists.
     */
    bool remove(const std::string& name);

    /**
     * @brief   Sorts items in ascending lexicographic order by name.
     * @note    Uses std::sort (unstable). Relative order of items with
     *          identical names (if any) is unspecified after sorting.
     */
    void sortByName();

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    /**
     * @brief   Computes the sum of all item quantities.
     * @return  Total stock count across all items; 0 if the inventory is empty.
     */
    [[nodiscard]] int totalQuantity() const noexcept;

    /**
     * @brief   Returns the number of distinct item records.
     * @return  Count of items currently held.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    // -----------------------------------------------------------------------
    // Output
    // -----------------------------------------------------------------------

    /**
     * @brief   Writes all items and their quantities to standard output.
     * @details Each item is printed on a separate line in the format:
     *          @code
     *            <name>: <quantity>
     *          @endcode
     */
    void print() const;

private:
    /// Backing store. Insertion order is preserved unless sortByName() is called.
    std::vector<Item> m_items;
};

} // namespace eval
