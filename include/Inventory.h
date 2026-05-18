/**
 * @file        Inventory.h
 * @brief       Inventory container declaration.
 * @details     Declares the Inventory class, which manages an ordered
 *              collection of Item objects and exposes querying, filtering,
 *              sorting, persistence, observer/callback, and undoable-command
 *              operations.
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     2.0.0
 *
 * @copyright   Copyright (c) 2026. All rights reserved.
 *
 * @par Revision History:
 * | Version | Date       | Author      | Description                          |
 * |---------|------------|-------------|--------------------------------------|
 * | 1.0.0   | 2026-05-15 | Seth Laurie | Initial release                      |
 * | 2.0.0   | 2026-05-15 | Seth Laurie | Add queries, persistence, commands,  |
 * |         |            |             | observer, iterator, transfer, merge  |
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
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace eval {

// Forward declaration — avoids circular include with Command.h.
// The undo history stores std::unique_ptr<ICommand>; ICommand must be
// complete only when Inventory's destructor is compiled (in Inventory.cpp).
class ICommand;

/**
 * @class   Inventory
 * @brief   Manages a mutable, ordered collection of Item objects.
 *
 * @details Inventory provides add, remove, sort, filter, transfer, reserve,
 *          merge, CSV persistence, structured-report output, an observer
 *          callback for low-stock events, range-based iteration, and an
 *          undoable command stack.
 *
 *          Items are identified by name; duplicate adds accumulate quantity
 *          into the existing record rather than creating a second entry.
 *
 * @note    Not thread-safe. External synchronisation is required when multiple
 *          threads share the same Inventory instance.
 */
class Inventory
{
public:
    // -----------------------------------------------------------------------
    // Iterator support — enables range-based for loops over Item objects.
    // -----------------------------------------------------------------------
    using iterator       = std::vector<Item>::iterator;
    using const_iterator = std::vector<Item>::const_iterator;

    // -----------------------------------------------------------------------
    // Observer callback type
    // -----------------------------------------------------------------------

    /**
     * @brief Callback signature for low-stock events.
     * @details Invoked after any quantity change that causes an item's stock
     *          to fall below the threshold set by setLowStockCallback().
     *          Parameters: item name, new quantity.
     */
    using LowStockCallback = std::function<void(const std::string&, int)>;

    // -----------------------------------------------------------------------
    // Construction / destruction
    // -----------------------------------------------------------------------

    Inventory();
    Inventory(const Inventory& other);              ///< Copies items; history is not duplicated.
    Inventory(Inventory&& other);
    Inventory& operator=(const Inventory& other);   ///< Copies items; history is cleared.
    Inventory& operator=(Inventory&& other);
    ~Inventory();   ///< Defined in .cpp where ICommand is a complete type.

    // -----------------------------------------------------------------------
    // Mutators — primitive operations (not recorded in undo history)
    // -----------------------------------------------------------------------

    /**
     * @brief   Adds stock to an existing item or inserts a new entry.
     * @details If an item whose name matches @p name already exists, its
     *          quantity is increased by @p quantity. Otherwise a new Item is
     *          appended with empty metadata fields.
     * @param   name      Name of the item. Must not be empty.
     * @param   quantity  Amount of stock to add (may be negative for a merge).
     */
    void add(const std::string& name, int quantity);

    /**
     * @brief   Inserts or merges a fully-specified Item.
     * @details If an item with the same name exists, its quantity is adjusted
     *          by the incoming item's quantity and all other metadata on the
     *          existing record is preserved. If no match is found, the item is
     *          inserted with all its fields intact.
     * @param   item  Item to insert or merge. Passed by value (moveable).
     */
    void add(Item item);

    /**
     * @brief   Removes the first item whose name matches @p name.
     * @return  @c true if found and removed; @c false otherwise.
     */
    bool remove(const std::string& name);

    /**
     * @brief   Sorts items in ascending lexicographic order by name.
     */
    void sortByName();

    /**
     * @brief   Sorts items in ascending order by quantity (lowest first).
     */
    void sortByQuantity();

    /**
     * @brief   Removes all items from the inventory.
     */
    void clear();

    /**
     * @brief   Merges all items from @p other into this inventory.
     * @details For each item in @p other, calls add(Item), so existing items
     *          have their quantities accumulated and new items are inserted.
     *          @p other is not modified.
     */
    void merge(const Inventory& other);

    /**
     * @brief   Transfers @p qty units of @p name from this inventory to @p destination.
     * @details Fails (returns false) if the item is not found, @p qty is
     *          non-positive, or @p qty exceeds the available quantity.
     *          The transferred item carries its source metadata into @p destination.
     * @return  @c true on success; @c false if the transfer cannot be completed.
     */
    bool transfer(const std::string& name, int qty, Inventory& destination);

    /**
     * @brief   Reserves (deducts) @p qty units without removing the item record.
     * @details Models the two-bin / reservation pattern. Returns false if the
     *          item is not found, @p qty is non-positive, or @p qty exceeds
     *          the available quantity.
     * @return  @c true on success; @c false otherwise.
     */
    bool reserve(const std::string& name, int qty);

    // -----------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------

    /**
     * @brief   Returns a pointer to the item with the given name, or nullptr.
     * @note    The pointer is invalidated by any non-const Inventory operation.
     */
    [[nodiscard]] const Item* findByName(const std::string& name) const;

    /**
     * @brief   Returns copies of all items whose quantity is strictly below @p threshold.
     */
    [[nodiscard]] std::vector<Item> findBelowThreshold(int threshold) const;

    /**
     * @brief   Returns copies of all items whose quantity is in [@p minQty, @p maxQty].
     */
    [[nodiscard]] std::vector<Item> findByRange(int minQty, int maxQty) const;

    /**
     * @brief   Computes the sum of all item quantities.
     * @return  Total stock count; 0 for an empty inventory.
     */
    [[nodiscard]] int totalQuantity() const noexcept;

    /**
     * @brief   Computes the total stock value (sum of quantity × unitPrice for all items).
     * @return  Total value; 0.0 for an empty inventory or items with no unit price.
     */
    [[nodiscard]] double totalValue() const noexcept;

    /**
     * @brief   Returns the number of distinct item records.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    // -----------------------------------------------------------------------
    // Observer
    // -----------------------------------------------------------------------

    /**
     * @brief   Registers a callback to fire when an item's quantity drops below a threshold.
     * @param   cb         Callable invoked with (itemName, newQuantity).
     * @param   threshold  Fires when quantity < threshold.
     */
    void setLowStockCallback(LowStockCallback cb, int threshold);

    // -----------------------------------------------------------------------
    // Command pattern — recorded in the undo history
    // -----------------------------------------------------------------------

    /**
     * @brief   Executes @p cmd and pushes it onto the undo stack.
     * @param   cmd  Ownership of the command is transferred to the inventory.
     */
    void executeCommand(std::unique_ptr<ICommand> cmd);

    /**
     * @brief   Reverses the most recent command on the undo stack.
     * @return  @c true if a command was undone; @c false if history is empty.
     */
    bool undo();

    // -----------------------------------------------------------------------
    // Persistence
    // -----------------------------------------------------------------------

    /**
     * @brief   Serialises the inventory to a CSV file.
     * @details Format: header line + one data line per item. Fields:
     *          name, quantity, category, lotNumber, expiryDate, unitPrice.
     *          Commas within field values are not escaped; callers must ensure
     *          field values do not contain commas.
     * @param   filename  Path to the output file (created or overwritten).
     * @return  @c true on success; @c false if the file could not be opened.
     */
    bool save(const std::string& filename) const;

    /**
     * @brief   Deserialises the inventory from a CSV file produced by save().
     * @details Clears the current inventory before loading. Skips the header
     *          line. Returns false and leaves the inventory partially loaded
     *          if a parse error is encountered.
     * @param   filename  Path to the CSV file to read.
     * @return  @c true on success; @c false on file-open or parse failure.
     */
    bool load(const std::string& filename);

    /**
     * @brief   Writes a human-readable inventory report to a text file.
     * @details The report includes a timestamp, full item listing, total
     *          quantity, total value, and a low-stock section for items
     *          below @p reportThreshold.
     * @param   filename         Path to the output report file.
     * @param   reportThreshold  Items with quantity below this appear in
     *                           the low-stock section. Defaults to 5.
     * @return  @c true on success; @c false if the file could not be opened.
     */
    bool writeReport(const std::string& filename, int reportThreshold = 5) const;

    // -----------------------------------------------------------------------
    // Output
    // -----------------------------------------------------------------------

    /**
     * @brief   Writes all items to standard output.
     * @details Delegates to operator<<(std::cout, *this).
     */
    void print() const;

    // -----------------------------------------------------------------------
    // Iterator
    // -----------------------------------------------------------------------

    iterator       begin()  noexcept;
    iterator       end()    noexcept;
    const_iterator begin()  const noexcept;
    const_iterator end()    const noexcept;
    const_iterator cbegin() const noexcept;
    const_iterator cend()   const noexcept;

private:
    std::vector<Item>                         m_items;
    LowStockCallback                          m_lowStockCallback;
    int                                       m_lowStockThreshold{0};
    std::vector<std::unique_ptr<ICommand>>    m_history;

    // Fires m_lowStockCallback if set and item.getQuantity() < m_lowStockThreshold.
    void triggerLowStockIfNeeded(const Item& item);
};

// ---------------------------------------------------------------------------
// Stream insertion operator (free function, uses public iterators)
// ---------------------------------------------------------------------------

/**
 * @brief   Writes all items to @p os, one per line, with full metadata.
 * @details Format per item:
 *          @code
 *            <name>: <qty> [<category> | lot:<lot> | exp:<expiry> | $<price>]
 *          @endcode
 *          Metadata fields are omitted from the brackets when empty / zero.
 *          The brackets are omitted entirely when all metadata is absent.
 */
std::ostream& operator<<(std::ostream& os, const Inventory& inv);

} // namespace eval
