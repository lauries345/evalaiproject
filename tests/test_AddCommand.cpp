#include "AddCommand.h"
#include "Inventory.h"
#include "Item.h"

#include <gtest/gtest.h>

#include <string>

/**
 * @file test_AddCommand.cpp
 * @brief Unit tests for eval::AddCommand. Targets 100% line and branch coverage.
 */

namespace
{

class AddCommandTest : public ::testing::Test
{
protected:
    eval::Inventory inv;
};

TEST_F(AddCommandTest, ExecuteInsertsNewItem)
{
    eval::AddCommand cmd{inv, eval::Item{"Apples", 10, 3, 0.50}};
    cmd.execute();
    ASSERT_EQ(inv.size(), 1u);
    EXPECT_EQ(inv.find("Apples")->getQuantity(), 10);
}

TEST_F(AddCommandTest, UndoNewInsertionRemovesItem)
{
    eval::AddCommand cmd{inv, eval::Item{"Apples", 10, 3, 0.50}};
    cmd.execute();
    cmd.undo();
    EXPECT_EQ(inv.size(), 0u);
}

TEST_F(AddCommandTest, ExecuteMergesIntoExisting)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    eval::AddCommand cmd{inv, eval::Item{"Apples", 5, 3, 0.50}};
    cmd.execute();
    EXPECT_EQ(inv.find("Apples")->getQuantity(), 15);
}

TEST_F(AddCommandTest, UndoMergeRestoresExactPriorQuantity)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    eval::AddCommand cmd{inv, eval::Item{"Apples", 5, 3, 0.50}};
    cmd.execute();
    cmd.undo();
    ASSERT_TRUE(inv.find("Apples").has_value());
    EXPECT_EQ(inv.find("Apples")->getQuantity(), 10);
}

TEST_F(AddCommandTest, RedoViaSecondExecuteIsIdempotentForState)
{
    eval::AddCommand cmd{inv, eval::Item{"Apples", 10, 3, 0.50}};
    cmd.execute();
    cmd.undo();
    cmd.execute();
    ASSERT_EQ(inv.size(), 1u);
    EXPECT_EQ(inv.find("Apples")->getQuantity(), 10);
}

TEST_F(AddCommandTest, DescriptionContainsNameAndQuantity)
{
    eval::AddCommand cmd{inv, eval::Item{"Apples", 7, 3, 0.50}};
    const std::string desc = cmd.description();
    EXPECT_NE(desc.find("Apples"), std::string::npos);
    EXPECT_NE(desc.find("7"),      std::string::npos);
}

} // namespace
