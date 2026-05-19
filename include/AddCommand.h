#pragma once

#include "ICommand.h"
#include "Item.h"

#include <optional>
#include <string>

/**
 * @file AddCommand.h
 * @brief Concrete command that adds (inserts-or-merges) an Item into an Inventory.
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

class Inventory;

/**
 * @brief Command: add an Item to an Inventory, with undo support.
 *
 * On execute() the command snapshots any pre-existing record sharing the
 * Item's name; on undo() it removes the post-execute record and (if a
 * snapshot exists) re-inserts the snapshot, restoring the prior state.
 */
class AddCommand final : public ICommand
{
public:
    /**
     * @param inv  inventory to mutate; must outlive this command.
     * @param item record to add (copied/moved into the command).
     */
    AddCommand(Inventory& inv, Item item);

    AddCommand(const AddCommand&)            = delete;
    AddCommand& operator=(const AddCommand&) = delete;
    AddCommand(AddCommand&&)                 = delete;
    AddCommand& operator=(AddCommand&&)      = delete;
    ~AddCommand() override                   = default;

    void execute() override;
    void undo()    override;
    [[nodiscard]] std::string description() const override;

private:
    Inventory&          m_inv;
    Item                m_item;
    std::optional<Item> m_priorSnapshot;
};

} // namespace eval
