/**
 * @file        test_Command.cpp
 * @brief       Unit tests for eval::AddCommand and eval::RemoveCommand.
 * @details     Verifies both the execute() and undo() paths for each concrete
 *              command, including edge cases (item absent, merge vs insert,
 *              full-metadata restoration).
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     1.0.0
 *
 * @copyright   Copyright (c) 2026. All rights reserved.
 *
 * @par Safety Classification:
 * Evaluation / non-production component. Must be formally reclassified before
 * integration into a regulated medical device.
 *
 * @par Applicable Standards:
 * - IEC 62304 §5.5: Software unit testing
 * - ISO/IEC 14882:2017: C++17 language standard
 */

#include "Command.h"
#include "Inventory.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

namespace {

// ============================================================================
// AddCommand — execute: new item
// ============================================================================

TEST(AddCommandTest, ExecuteInsertsNewItem)
{
    eval::Inventory inv;
    eval::AddCommand cmd(inv, "Widget", 10);
    cmd.execute();
    ASSERT_NE(inv.findByName("Widget"), nullptr);
    EXPECT_EQ(inv.findByName("Widget")->getQuantity(), 10);
}

TEST(AddCommandTest, ExecuteNewItemIncreasesSize)
{
    eval::Inventory inv;
    eval::AddCommand cmd(inv, "Widget", 10);
    cmd.execute();
    EXPECT_EQ(inv.size(), 1u);
}

// ============================================================================
// AddCommand — undo: new item (removes)
// ============================================================================

TEST(AddCommandTest, UndoNewItemRemovesIt)
{
    eval::Inventory inv;
    eval::AddCommand cmd(inv, "Widget", 10);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(inv.findByName("Widget"), nullptr);
    EXPECT_EQ(inv.size(), 0u);
}

// ============================================================================
// AddCommand — execute: merge (item already exists)
// ============================================================================

TEST(AddCommandTest, ExecuteMergesIntoExistingItem)
{
    eval::Inventory inv;
    inv.add("Widget", 5);
    eval::AddCommand cmd(inv, "Widget", 10);
    cmd.execute();
    EXPECT_EQ(inv.size(), 1u); // no new item
    EXPECT_EQ(inv.findByName("Widget")->getQuantity(), 15);
}

// ============================================================================
// AddCommand — undo: merge (reverses delta)
// ============================================================================

TEST(AddCommandTest, UndoMergeRestoresPreviousQuantity)
{
    eval::Inventory inv;
    inv.add("Widget", 5);
    eval::AddCommand cmd(inv, "Widget", 10);
    cmd.execute(); // qty → 15
    cmd.undo();    // qty → 5
    ASSERT_NE(inv.findByName("Widget"), nullptr);
    EXPECT_EQ(inv.findByName("Widget")->getQuantity(), 5);
}

TEST(AddCommandTest, UndoMergeDoesNotRemoveItem)
{
    eval::Inventory inv;
    inv.add("Widget", 5);
    eval::AddCommand cmd(inv, "Widget", 10);
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(inv.size(), 1u); // item still present
}

// ============================================================================
// RemoveCommand — execute: item exists
// ============================================================================

TEST(RemoveCommandTest, ExecuteRemovesExistingItem)
{
    eval::Inventory inv;
    inv.add("Widget", 5);
    eval::RemoveCommand cmd(inv, "Widget");
    cmd.execute();
    EXPECT_EQ(inv.findByName("Widget"), nullptr);
    EXPECT_EQ(inv.size(), 0u);
}

// ============================================================================
// RemoveCommand — execute: item does not exist (no-op)
// ============================================================================

TEST(RemoveCommandTest, ExecuteIsNoOpWhenItemMissing)
{
    eval::Inventory inv;
    eval::RemoveCommand cmd(inv, "Ghost");
    EXPECT_NO_THROW(cmd.execute());
    EXPECT_EQ(inv.size(), 0u);
}

// ============================================================================
// RemoveCommand — undo: restores item
// ============================================================================

TEST(RemoveCommandTest, UndoRestoresItemQuantity)
{
    eval::Inventory inv;
    inv.add("Widget", 7);
    eval::RemoveCommand cmd(inv, "Widget");
    cmd.execute();
    cmd.undo();
    ASSERT_NE(inv.findByName("Widget"), nullptr);
    EXPECT_EQ(inv.findByName("Widget")->getQuantity(), 7);
}

TEST(RemoveCommandTest, UndoRestoresFullItemMetadata)
{
    eval::Inventory inv;
    inv.add(eval::Item{"Widget", 7, "Device", "LOT-X", "2030-06-01", 12.50});
    eval::RemoveCommand cmd(inv, "Widget");
    cmd.execute();
    cmd.undo();
    const eval::Item* item = inv.findByName("Widget");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getCategory(),   "Device");
    EXPECT_EQ(item->getLotNumber(),  "LOT-X");
    EXPECT_EQ(item->getExpiryDate(), "2030-06-01");
    EXPECT_DOUBLE_EQ(item->getUnitPrice(), 12.50);
}

// ============================================================================
// RemoveCommand — undo: item was missing (undo is a no-op, no crash)
// ============================================================================

TEST(RemoveCommandTest, UndoIsNoOpWhenItemWasMissing)
{
    eval::Inventory inv;
    eval::RemoveCommand cmd(inv, "Ghost");
    cmd.execute(); // no-op (item absent)
    EXPECT_NO_THROW(cmd.undo()); // must not insert a ghost item
    EXPECT_EQ(inv.size(), 0u);
}

// ============================================================================
// Inventory::executeCommand / undo integration
// ============================================================================

TEST(CommandIntegrationTest, ExecuteCommandRecordsInHistory)
{
    eval::Inventory inv;
    inv.executeCommand(std::make_unique<eval::AddCommand>(inv, "A", 1));
    inv.executeCommand(std::make_unique<eval::AddCommand>(inv, "B", 2));
    EXPECT_EQ(inv.size(), 2u);

    inv.undo(); // undo B
    EXPECT_EQ(inv.size(), 1u);
    EXPECT_EQ(inv.findByName("A")->getQuantity(), 1);

    inv.undo(); // undo A
    EXPECT_EQ(inv.size(), 0u);
}

TEST(CommandIntegrationTest, MultipleUndosWorkInLIFOOrder)
{
    eval::Inventory inv;
    inv.executeCommand(std::make_unique<eval::AddCommand>(inv, "First",  5));
    inv.executeCommand(std::make_unique<eval::AddCommand>(inv, "Second", 3));

    inv.undo(); // removes Second
    EXPECT_EQ(inv.findByName("Second"), nullptr);
    EXPECT_NE(inv.findByName("First"),  nullptr);
}

} // namespace
