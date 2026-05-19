#include "Item.h"

#include <gtest/gtest.h>

#include <stdexcept>

/**
 * @file test_Item.cpp
 * @brief Unit tests for eval::Item. Targets 100% line and branch coverage.
 */

namespace
{

class ItemTest : public ::testing::Test
{
};

TEST_F(ItemTest, ConstructsWithCorrectFields)
{
    const eval::Item item{"Apples", 10, 3, 0.50};
    EXPECT_EQ(item.getName(), "Apples");
    EXPECT_EQ(item.getQuantity(), 10);
    EXPECT_EQ(item.getLowStockThreshold(), 3);
    EXPECT_DOUBLE_EQ(item.getUnitPrice(), 0.50);
}

TEST_F(ItemTest, RejectsEmptyName)
{
    EXPECT_THROW((eval::Item{"", 1, 0, 0.0}), std::invalid_argument);
}

TEST_F(ItemTest, RejectsNegativeQuantity)
{
    EXPECT_THROW((eval::Item{"X", -1, 0, 0.0}), std::invalid_argument);
}

TEST_F(ItemTest, RejectsNegativeLowStockThreshold)
{
    EXPECT_THROW((eval::Item{"X", 0, -1, 0.0}), std::invalid_argument);
}

TEST_F(ItemTest, RejectsNegativeUnitPrice)
{
    EXPECT_THROW((eval::Item{"X", 0, 0, -0.01}), std::invalid_argument);
}

TEST_F(ItemTest, AdjustQuantityIncrementsAndDecrements)
{
    eval::Item item{"X", 10, 0, 0.0};
    item.adjustQuantity(5);
    EXPECT_EQ(item.getQuantity(), 15);
    item.adjustQuantity(-12);
    EXPECT_EQ(item.getQuantity(), 3);
}

TEST_F(ItemTest, AdjustQuantityThrowsOnNegativeResult)
{
    eval::Item item{"X", 1, 0, 0.0};
    EXPECT_THROW(item.adjustQuantity(-2), std::out_of_range);
    EXPECT_EQ(item.getQuantity(), 1);
}

TEST_F(ItemTest, IsLowStockBoundaries)
{
    EXPECT_FALSE(eval::Item("X", 3, 2, 0.0).isLowStock());
    EXPECT_TRUE (eval::Item("X", 2, 2, 0.0).isLowStock());
    EXPECT_TRUE (eval::Item("X", 1, 2, 0.0).isLowStock());
}

} // namespace
