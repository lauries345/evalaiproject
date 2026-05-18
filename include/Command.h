/**
 * @file        Command.h
 * @brief       Command pattern interface and concrete commands.
 * @details     Declares the ICommand abstract interface and two concrete
 *              command types — AddCommand and RemoveCommand — that wrap
 *              Inventory mutations and provide undo support.
 *
 *              The undo stack is owned by Inventory::executeCommand() /
 *              Inventory::undo(). Callers create commands via std::make_unique
 *              and transfer ownership to the inventory.
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
 * @par Design Pattern:
 * GoF Command pattern. Each concrete command captures its receiver (Inventory&)
 * and all parameters needed to both execute and reverse the operation.
 */

#pragma once

#include <string>

namespace eval {

// Forward declaration — avoids circular include between Command.h and Inventory.h.
class Inventory;

// ===========================================================================
// ICommand — abstract command interface
// ===========================================================================

/**
 * @class   ICommand
 * @brief   Abstract base for all undoable inventory commands.
 * @details Implementations must be self-contained: execute() performs the
 *          operation, undo() reverses it exactly. Both may be called
 *          multiple times (e.g. redo support), so state captured during
 *          execute() must be preserved until the command object is destroyed.
 */
class ICommand
{
public:
    virtual ~ICommand() = default;

    /** @brief Performs the command operation on the target Inventory. */
    virtual void execute() = 0;

    /** @brief Reverses the effect of the most recent execute() call. */
    virtual void undo() = 0;
};

// ===========================================================================
// AddCommand
// ===========================================================================

/**
 * @class   AddCommand
 * @brief   Wraps Inventory::add(name, quantity) with undo support.
 * @details execute() calls add(). If the item was newly inserted, undo()
 *          removes it. If the item already existed (quantity merged),
 *          undo() subtracts the same delta to restore the prior quantity.
 */
class AddCommand : public ICommand
{
public:
    /**
     * @brief Constructs an AddCommand targeting @p inv.
     * @param inv   Inventory to operate on. Must outlive this command.
     * @param name  Name of the item to add or update.
     * @param qty   Quantity delta to apply.
     */
    AddCommand(Inventory& inv, std::string name, int qty);

    void execute() override;
    void undo()    override;

private:
    Inventory&  m_inv;
    std::string m_name;
    int         m_qty;
    bool        m_wasNew{false}; ///< Set during execute(); drives undo() strategy.
};

// ===========================================================================
// RemoveCommand
// ===========================================================================

/**
 * @class   RemoveCommand
 * @brief   Wraps Inventory::remove(name) with full item restoration on undo.
 * @details execute() snapshots all item fields before removing. undo()
 *          re-inserts the item with its original quantity and metadata.
 */
class RemoveCommand : public ICommand
{
public:
    /**
     * @brief Constructs a RemoveCommand targeting @p inv.
     * @param inv   Inventory to operate on. Must outlive this command.
     * @param name  Name of the item to remove.
     */
    RemoveCommand(Inventory& inv, std::string name);

    void execute() override;
    void undo()    override;

private:
    Inventory&  m_inv;
    std::string m_name;

    // Item snapshot captured during execute() — used to restore on undo().
    int         m_savedQty{0};
    std::string m_savedCategory;
    std::string m_savedLotNumber;
    std::string m_savedExpiryDate;
    double      m_savedUnitPrice{0.0};
    bool        m_found{false}; ///< false when item did not exist; undo() is a no-op.
};

} // namespace eval
