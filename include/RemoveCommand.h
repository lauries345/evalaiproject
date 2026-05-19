#pragma once

#include "ICommand.h"
#include "Item.h"

#include <optional>
#include <string>

/**
 * @file RemoveCommand.h
 * @brief Concrete command that removes a named Item from an Inventory.
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
 * @brief Command: remove a named Item from an Inventory, with undo.
 *
 * On execute() the command snapshots the target record (if any) and
 * erases it; on undo() it re-inserts the snapshot. If no record existed
 * at execute time, undo() is a no-op.
 */
class RemoveCommand final : public ICommand
{
public:
    /**
     * @param inv  inventory to mutate; must outlive this command.
     * @param name non-empty product identifier to remove.
     */
    RemoveCommand(Inventory& inv, std::string name);

    RemoveCommand(const RemoveCommand&)            = delete;
    RemoveCommand& operator=(const RemoveCommand&) = delete;
    RemoveCommand(RemoveCommand&&)                 = delete;
    RemoveCommand& operator=(RemoveCommand&&)      = delete;
    ~RemoveCommand() override                      = default;

    void execute() override;
    void undo()    override;
    [[nodiscard]] std::string description() const override;

private:
    Inventory&          m_inv;
    std::string         m_name;
    std::optional<Item> m_snapshot;
};

} // namespace eval
