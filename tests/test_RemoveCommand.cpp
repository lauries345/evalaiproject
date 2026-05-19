#include "Inventory.h"
#include "Item.h"
#include "RemoveCommand.h"

#include <gtest/gtest.h>

#include <string>

/**
 * @file test_RemoveCommand.cpp
 * @brief Unit tests for eval::RemoveCommand. Targets 100% line and branch coverage.
 */

namespace
{

class RemoveCommandTest : public ::testing::Test
{
protected:
    eval::Inventory inv;
};

TEST_F(RemoveCommandTest, ExecuteRemovesExistingItem)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    eval::RemoveCommand cmd{inv, "Apples"};
    cmd.execute();
    EXPECT_EQ(inv.size(), 0u);
}

TEST_F(RemoveCommandTest, UndoRestoresAllFieldsExactly)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.75});
    eval::RemoveCommand cmd{inv, "Apples"};
    cmd.execute();
    cmd.undo();
    ASSERT_TRUE(inv.find("Apples").has_value());
    const auto restored = *inv.find("Apples");
    EXPECT_EQ(restored.getName(),              "Apples");
    EXPECT_EQ(restored.getQuantity(),          10);
    EXPECT_EQ(restored.getLowStockThreshold(), 3);
    EXPECT_DOUBLE_EQ(restored.getUnitPrice(),  0.75);
}

TEST_F(RemoveCommandTest, ExecuteOnMissingIsNoOpAndUndoIsNoOp)
{
    eval::RemoveCommand cmd{inv, "Ghost"};
    cmd.execute();
    EXPECT_EQ(inv.size(), 0u);
    cmd.undo();
    EXPECT_EQ(inv.size(), 0u);
}

TEST_F(RemoveCommandTest, DescriptionContainsName)
{
    eval::RemoveCommand cmd{inv, "Apples"};
    EXPECT_NE(cmd.description().find("Apples"), std::string::npos);
}

} // namespace
