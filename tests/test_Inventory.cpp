/**
 * @file        test_Inventory.cpp
 * @brief       Unit tests for the eval::Inventory class.
 * @details     Verifies all public methods of Inventory:
 *              - Default construction (empty state)
 *              - add()          – new item insertion and duplicate merging
 *              - remove()       – removal by name (found and not-found paths)
 *              - sortByName()   – lexicographic sort (both comparator branches)
 *              - totalQuantity() – empty and non-empty inventory
 *              - size()         – item count
 *              - print()        – formatted stdout output (empty and non-empty)
 *              - Value semantics (copy and move construction/assignment)
 *
 *              Designed to achieve 100 % line and branch coverage of
 *              src/Inventory.cpp and include/Inventory.h.
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     1.1.0
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

#include "Inventory.h"

#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace {

// ---------------------------------------------------------------------------
// Helper: redirect std::cout to an ostringstream for the duration of a call,
// then restore and return the captured text.
// ---------------------------------------------------------------------------
std::string captureStdout(const std::function<void()>& fn)
{
    std::ostringstream  oss;
    std::streambuf*     oldBuf = std::cout.rdbuf(oss.rdbuf());
    fn();
    std::cout.rdbuf(oldBuf);
    return oss.str();
}

// ============================================================================
// Construction
// ============================================================================

TEST(InventoryTest, DefaultConstructorCreatesEmptyInventory)
{
    const eval::Inventory inv;
    EXPECT_EQ(inv.size(),          0u);
    EXPECT_EQ(inv.totalQuantity(), 0);
}

// ============================================================================
// size()
// ============================================================================

TEST(InventoryTest, SizeIsZeroForEmptyInventory)
{
    const eval::Inventory inv;
    EXPECT_EQ(inv.size(), 0u);
}

TEST(InventoryTest, SizeIncreasesAfterAddingNewItems)
{
    eval::Inventory inv;
    inv.add("Alpha", 1);
    EXPECT_EQ(inv.size(), 1u);
    inv.add("Beta", 2);
    EXPECT_EQ(inv.size(), 2u);
}

// ============================================================================
// totalQuantity()
// ============================================================================

TEST(InventoryTest, TotalQuantityIsZeroForEmptyInventory)
{
    const eval::Inventory inv;
    EXPECT_EQ(inv.totalQuantity(), 0);
}

TEST(InventoryTest, TotalQuantitySumsAllItems)
{
    eval::Inventory inv;
    inv.add("A", 10);
    inv.add("B",  5);
    inv.add("C",  8);
    EXPECT_EQ(inv.totalQuantity(), 23);
}

// ============================================================================
// add() – new-item branch (item not found → emplace_back)
// ============================================================================

TEST(InventoryTest, AddNewItemIncreasesSize)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    EXPECT_EQ(inv.size(), 1u);
}

TEST(InventoryTest, AddNewItemStoresCorrectData)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    const std::string output = captureStdout([&] { inv.print(); });
    EXPECT_EQ(output, "  Apple: 5\n");
}

TEST(InventoryTest, AddMultipleDistinctItems)
{
    eval::Inventory inv;
    inv.add("Apple",  10);
    inv.add("Banana",  3);
    EXPECT_EQ(inv.size(),          2u);
    EXPECT_EQ(inv.totalQuantity(), 13);
}

// ============================================================================
// add() – merge branch (item already exists → adjustQuantity)
// ============================================================================

TEST(InventoryTest, AddDuplicateNameMergesQuantity)
{
    eval::Inventory inv;
    inv.add("Apple", 10);
    inv.add("Apple",  3);   // should merge, not insert
    EXPECT_EQ(inv.size(),          1u);
    EXPECT_EQ(inv.totalQuantity(), 13);
}

TEST(InventoryTest, AddDuplicateDoesNotIncreaseSize)
{
    eval::Inventory inv;
    inv.add("X", 1);
    inv.add("X", 1);
    EXPECT_EQ(inv.size(), 1u);
}

TEST(InventoryTest, AddDuplicateWithNegativeDeltaReducesQuantity)
{
    eval::Inventory inv;
    inv.add("Stock", 20);
    inv.add("Stock", -8);   // net: 12
    EXPECT_EQ(inv.totalQuantity(), 12);
}

// ============================================================================
// remove() – item-not-found branch (returns false)
// ============================================================================

TEST(InventoryTest, RemoveNonExistentItemReturnsFalse)
{
    eval::Inventory inv;
    EXPECT_FALSE(inv.remove("Ghost"));
}

TEST(InventoryTest, RemoveFromEmptyInventoryReturnsFalse)
{
    eval::Inventory inv;
    EXPECT_FALSE(inv.remove("Anything"));
}

TEST(InventoryTest, RemoveNonExistentItemDoesNotChangeSize)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    inv.remove("Ghost");
    EXPECT_EQ(inv.size(), 1u);
}

// ============================================================================
// remove() – item-found branch (erases and returns true)
// ============================================================================

TEST(InventoryTest, RemoveExistingItemReturnsTrue)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    EXPECT_TRUE(inv.remove("Apple"));
}

TEST(InventoryTest, RemoveExistingItemDecreasesSize)
{
    eval::Inventory inv;
    inv.add("Apple",  5);
    inv.add("Banana", 3);
    inv.remove("Apple");
    EXPECT_EQ(inv.size(), 1u);
}

TEST(InventoryTest, RemoveExistingItemReducesTotalQuantity)
{
    eval::Inventory inv;
    inv.add("Apple",  5);
    inv.add("Banana", 3);
    inv.remove("Apple");
    EXPECT_EQ(inv.totalQuantity(), 3);
}

TEST(InventoryTest, RemoveOnlyItemLeavesInventoryEmpty)
{
    eval::Inventory inv;
    inv.add("Solo", 1);
    inv.remove("Solo");
    EXPECT_EQ(inv.size(),          0u);
    EXPECT_EQ(inv.totalQuantity(), 0);
}

// ============================================================================
// sortByName()
// Exercises both the true branch (lhs < rhs) and the false branch of the
// sort comparator by providing items in reverse alphabetical order.
// ============================================================================

TEST(InventoryTest, SortByNameOrdersItemsLexicographically)
{
    eval::Inventory inv;
    inv.add("Zucchini", 3);   // added in reverse alpha order
    inv.add("Mango",    2);
    inv.add("Apple",    1);

    inv.sortByName();

    const std::string expected =
        "  Apple: 1\n"
        "  Mango: 2\n"
        "  Zucchini: 3\n";

    const std::string output = captureStdout([&] { inv.print(); });
    EXPECT_EQ(output, expected);
}

TEST(InventoryTest, SortByNameOnEmptyInventoryIsNoOp)
{
    eval::Inventory inv;
    inv.sortByName();
    EXPECT_EQ(inv.size(), 0u);
}

TEST(InventoryTest, SortByNameOnSingleItemIsNoOp)
{
    eval::Inventory inv;
    inv.add("Solo", 7);
    inv.sortByName();
    EXPECT_EQ(inv.size(),          1u);
    EXPECT_EQ(inv.totalQuantity(), 7);
}

TEST(InventoryTest, SortByNameIsIdempotentWhenAlreadySorted)
{
    eval::Inventory inv;
    inv.add("Alpha", 1);
    inv.add("Beta",  2);
    inv.add("Gamma", 3);

    inv.sortByName();   // first sort

    const std::string first  = captureStdout([&] { inv.print(); });
    inv.sortByName();   // second sort (already ordered)
    const std::string second = captureStdout([&] { inv.print(); });

    EXPECT_EQ(first, second);
}

// ============================================================================
// print() – empty and non-empty loops
// ============================================================================

TEST(InventoryTest, PrintProducesNoOutputForEmptyInventory)
{
    const eval::Inventory inv;
    const std::string output = captureStdout([&] { inv.print(); });
    EXPECT_TRUE(output.empty());
}

TEST(InventoryTest, PrintFormatsSingleItemCorrectly)
{
    eval::Inventory inv;
    inv.add("Sensor", 42);
    const std::string output = captureStdout([&] { inv.print(); });
    EXPECT_EQ(output, "  Sensor: 42\n");
}

TEST(InventoryTest, PrintFormatsMultipleItemsCorrectly)
{
    eval::Inventory inv;
    inv.add("Alpha", 1);
    inv.add("Beta",  2);
    const std::string expected =
        "  Alpha: 1\n"
        "  Beta: 2\n";
    const std::string output = captureStdout([&] { inv.print(); });
    EXPECT_EQ(output, expected);
}

TEST(InventoryTest, PrintRespectsInsertionOrderBeforeSort)
{
    eval::Inventory inv;
    inv.add("Zulu",  1);
    inv.add("Alpha", 2);
    const std::string expected =
        "  Zulu: 1\n"
        "  Alpha: 2\n";
    const std::string output = captureStdout([&] { inv.print(); });
    EXPECT_EQ(output, expected);
}

// ============================================================================
// Value semantics – copy
// ============================================================================

TEST(InventoryTest, CopyConstructorCreatesIndependentInventory)
{
    eval::Inventory original;
    original.add("X", 5);

    eval::Inventory copy(original);
    copy.add("X", 10);   // modifies copy only

    EXPECT_EQ(original.totalQuantity(), 5);
    EXPECT_EQ(copy.totalQuantity(),     15);
}

TEST(InventoryTest, CopyAssignmentCreatesIndependentInventory)
{
    eval::Inventory src;
    src.add("Y", 3);

    eval::Inventory dst;
    dst = src;
    dst.add("Y", 7);   // modifies dst only

    EXPECT_EQ(src.totalQuantity(), 3);
    EXPECT_EQ(dst.totalQuantity(), 10);
}

// ============================================================================
// Value semantics – move
// ============================================================================

TEST(InventoryTest, MoveConstructorTransfersContents)
{
    eval::Inventory source;
    source.add("Z", 9);

    const eval::Inventory dest(std::move(source));
    EXPECT_EQ(dest.size(),          1u);
    EXPECT_EQ(dest.totalQuantity(), 9);
}

TEST(InventoryTest, MoveAssignmentTransfersContents)
{
    eval::Inventory source;
    source.add("W", 4);

    eval::Inventory dest;
    dest = std::move(source);
    EXPECT_EQ(dest.size(),          1u);
    EXPECT_EQ(dest.totalQuantity(), 4);
}

} // namespace
