/**
 * @file        test_Inventory.cpp
 * @brief       Unit tests for the eval::Inventory class.
 * @details     Verifies all public methods of Inventory:
 *              - Construction and value semantics
 *              - add() / add(Item) — insertion and duplicate merging
 *              - remove()
 *              - sortByName() / sortByQuantity()
 *              - findByName() / findBelowThreshold() / findByRange()
 *              - totalQuantity() / totalValue() / size()
 *              - transfer() / reserve() / clear() / merge()
 *              - save() / load() / writeReport()
 *              - setLowStockCallback()
 *              - executeCommand() / undo()
 *              - Iterators (begin/end, range-based for)
 *              - operator<<
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     2.0.0
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

#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

namespace {

// ---------------------------------------------------------------------------
// Helper: redirect std::cout to an ostringstream for the duration of a call.
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
// totalValue()
// ============================================================================

TEST(InventoryTest, TotalValueIsZeroForEmptyInventory)
{
    const eval::Inventory inv;
    EXPECT_DOUBLE_EQ(inv.totalValue(), 0.0);
}

TEST(InventoryTest, TotalValueSumsQtyTimesUnitPrice)
{
    eval::Inventory inv;
    inv.add(eval::Item{"A", 10, {}, {}, {}, 2.0}); // 20.0
    inv.add(eval::Item{"B",  5, {}, {}, {}, 4.0}); // 20.0
    EXPECT_DOUBLE_EQ(inv.totalValue(), 40.0);
}

TEST(InventoryTest, TotalValueIsZeroWhenNoPricesSet)
{
    eval::Inventory inv;
    inv.add("A", 10);
    inv.add("B",  5);
    EXPECT_DOUBLE_EQ(inv.totalValue(), 0.0);
}

// ============================================================================
// add(name, qty) — new-item branch
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
// add(name, qty) — merge branch
// ============================================================================

TEST(InventoryTest, AddDuplicateNameMergesQuantity)
{
    eval::Inventory inv;
    inv.add("Apple", 10);
    inv.add("Apple",  3);
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
    inv.add("Stock", -8);
    EXPECT_EQ(inv.totalQuantity(), 12);
}

// ============================================================================
// add(Item) — full-metadata overload
// ============================================================================

TEST(InventoryTest, AddItemInsertsWithFullMetadata)
{
    eval::Inventory inv;
    inv.add(eval::Item{"Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12});
    const eval::Item* item = inv.findByName("Lancet");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getCategory(),   "Consumable");
    EXPECT_EQ(item->getLotNumber(),  "LOT-001");
    EXPECT_EQ(item->getExpiryDate(), "2028-01-01");
    EXPECT_DOUBLE_EQ(item->getUnitPrice(), 0.12);
}

TEST(InventoryTest, AddItemMergePreservesExistingMetadata)
{
    eval::Inventory inv;
    inv.add(eval::Item{"Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12});
    // Merge with a name-only item (no metadata); existing metadata should survive.
    inv.add("Lancet", 50);
    const eval::Item* item = inv.findByName("Lancet");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getQuantity(),   150);
    EXPECT_EQ(item->getCategory(),   "Consumable"); // preserved
    EXPECT_EQ(item->getLotNumber(),  "LOT-001");    // preserved
}

// ============================================================================
// remove()
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
// ============================================================================

TEST(InventoryTest, SortByNameOrdersItemsLexicographically)
{
    eval::Inventory inv;
    inv.add("Zucchini", 3);
    inv.add("Mango",    2);
    inv.add("Apple",    1);
    inv.sortByName();

    const std::string expected =
        "  Apple: 1\n"
        "  Mango: 2\n"
        "  Zucchini: 3\n";
    EXPECT_EQ(captureStdout([&] { inv.print(); }), expected);
}

TEST(InventoryTest, SortByNameOnEmptyInventoryIsNoOp)
{
    eval::Inventory inv;
    inv.sortByName();
    EXPECT_EQ(inv.size(), 0u);
}

TEST(InventoryTest, SortByNameIsIdempotentWhenAlreadySorted)
{
    eval::Inventory inv;
    inv.add("Alpha", 1);
    inv.add("Beta",  2);
    inv.add("Gamma", 3);
    inv.sortByName();
    const std::string first  = captureStdout([&] { inv.print(); });
    inv.sortByName();
    const std::string second = captureStdout([&] { inv.print(); });
    EXPECT_EQ(first, second);
}

// ============================================================================
// sortByQuantity()
// ============================================================================

TEST(InventoryTest, SortByQuantityOrdersItemsAscending)
{
    eval::Inventory inv;
    inv.add("C", 30);
    inv.add("A", 10);
    inv.add("B", 20);
    inv.sortByQuantity();

    // After sort: A(10), B(20), C(30)
    const eval::Item* first = &(*inv.begin());
    EXPECT_EQ(first->getQuantity(), 10);
}

TEST(InventoryTest, SortByQuantityOnEmptyInventoryIsNoOp)
{
    eval::Inventory inv;
    inv.sortByQuantity();
    EXPECT_EQ(inv.size(), 0u);
}

// ============================================================================
// findByName()
// ============================================================================

TEST(InventoryTest, FindByNameReturnsNullptrForEmptyInventory)
{
    const eval::Inventory inv;
    EXPECT_EQ(inv.findByName("X"), nullptr);
}

TEST(InventoryTest, FindByNameReturnsNullptrForMissingItem)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    EXPECT_EQ(inv.findByName("Ghost"), nullptr);
}

TEST(InventoryTest, FindByNameReturnsPointerToExistingItem)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    EXPECT_NE(inv.findByName("Apple"), nullptr);
}

TEST(InventoryTest, FindByNameReturnsCorrectItem)
{
    eval::Inventory inv;
    inv.add("Apple",  5);
    inv.add("Banana", 3);
    const eval::Item* item = inv.findByName("Banana");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getName(),     "Banana");
    EXPECT_EQ(item->getQuantity(), 3);
}

// ============================================================================
// findBelowThreshold()
// ============================================================================

TEST(InventoryTest, FindBelowThresholdReturnsEmptyWhenAllAbove)
{
    eval::Inventory inv;
    inv.add("A", 10);
    inv.add("B", 20);
    EXPECT_TRUE(inv.findBelowThreshold(5).empty());
}

TEST(InventoryTest, FindBelowThresholdReturnsCorrectItems)
{
    eval::Inventory inv;
    inv.add("Low",  2);
    inv.add("High", 20);
    const auto result = inv.findBelowThreshold(5);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].getName(), "Low");
}

TEST(InventoryTest, FindBelowThresholdExcludesItemAtThreshold)
{
    eval::Inventory inv;
    inv.add("Exact", 5);
    // qty == threshold is NOT below — should be excluded
    EXPECT_TRUE(inv.findBelowThreshold(5).empty());
}

// ============================================================================
// findByRange()
// ============================================================================

TEST(InventoryTest, FindByRangeReturnsEmptyWhenNoneMatch)
{
    eval::Inventory inv;
    inv.add("A", 1);
    inv.add("B", 100);
    EXPECT_TRUE(inv.findByRange(10, 50).empty());
}

TEST(InventoryTest, FindByRangeReturnsCorrectItems)
{
    eval::Inventory inv;
    inv.add("Low",  2);
    inv.add("Mid", 15);
    inv.add("High", 80);
    const auto result = inv.findByRange(5, 20);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].getName(), "Mid");
}

TEST(InventoryTest, FindByRangeIsInclusiveOnBothEnds)
{
    eval::Inventory inv;
    inv.add("Min",  5);
    inv.add("Max", 10);
    const auto result = inv.findByRange(5, 10);
    EXPECT_EQ(result.size(), 2u);
}

// ============================================================================
// clear()
// ============================================================================

TEST(InventoryTest, ClearEmptiesInventory)
{
    eval::Inventory inv;
    inv.add("A", 1);
    inv.add("B", 2);
    inv.clear();
    EXPECT_EQ(inv.size(), 0u);
}

TEST(InventoryTest, ClearResetsTotalQuantity)
{
    eval::Inventory inv;
    inv.add("A", 50);
    inv.clear();
    EXPECT_EQ(inv.totalQuantity(), 0);
}

// ============================================================================
// merge()
// ============================================================================

TEST(InventoryTest, MergeAddsNewItemsFromOther)
{
    eval::Inventory primary;
    primary.add("A", 10);

    eval::Inventory other;
    other.add("B", 5);

    primary.merge(other);
    EXPECT_EQ(primary.size(), 2u);
    EXPECT_NE(primary.findByName("B"), nullptr);
}

TEST(InventoryTest, MergeCombinesQuantityForDuplicates)
{
    eval::Inventory primary;
    primary.add("A", 10);

    eval::Inventory other;
    other.add("A", 5);

    primary.merge(other);
    EXPECT_EQ(primary.size(), 1u);
    EXPECT_EQ(primary.findByName("A")->getQuantity(), 15);
}

TEST(InventoryTest, MergeDoesNotModifyOther)
{
    eval::Inventory primary;
    eval::Inventory other;
    other.add("X", 7);

    primary.merge(other);
    EXPECT_EQ(other.size(), 1u);
    EXPECT_EQ(other.findByName("X")->getQuantity(), 7);
}

// ============================================================================
// transfer()
// ============================================================================

TEST(InventoryTest, TransferReturnsFalseWhenSourceItemNotFound)
{
    eval::Inventory src;
    eval::Inventory dst;
    EXPECT_FALSE(src.transfer("Ghost", 5, dst));
}

TEST(InventoryTest, TransferReturnsFalseWhenQtyExceedsAvailable)
{
    eval::Inventory src;
    src.add("A", 5);
    eval::Inventory dst;
    EXPECT_FALSE(src.transfer("A", 10, dst));
}

TEST(InventoryTest, TransferReturnsFalseForZeroQty)
{
    eval::Inventory src;
    src.add("A", 5);
    eval::Inventory dst;
    EXPECT_FALSE(src.transfer("A", 0, dst));
}

TEST(InventoryTest, TransferReducesSourceQuantity)
{
    eval::Inventory src;
    src.add("A", 10);
    eval::Inventory dst;
    src.transfer("A", 3, dst);
    EXPECT_EQ(src.findByName("A")->getQuantity(), 7);
}

TEST(InventoryTest, TransferIncreasesDestinationQuantity)
{
    eval::Inventory src;
    src.add("A", 10);
    eval::Inventory dst;
    src.transfer("A", 3, dst);
    ASSERT_NE(dst.findByName("A"), nullptr);
    EXPECT_EQ(dst.findByName("A")->getQuantity(), 3);
}

TEST(InventoryTest, TransferPreservesSourceMetadataInDestination)
{
    eval::Inventory src;
    src.add(eval::Item{"A", 10, "Device", "LOT-1", "2030-01-01", 5.0});
    eval::Inventory dst;
    src.transfer("A", 3, dst);
    const eval::Item* item = dst.findByName("A");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getCategory(),  "Device");
    EXPECT_EQ(item->getLotNumber(), "LOT-1");
}

// ============================================================================
// reserve()
// ============================================================================

TEST(InventoryTest, ReserveReturnsFalseWhenItemNotFound)
{
    eval::Inventory inv;
    EXPECT_FALSE(inv.reserve("Ghost", 5));
}

TEST(InventoryTest, ReserveReturnsFalseWhenQtyExceedsAvailable)
{
    eval::Inventory inv;
    inv.add("A", 5);
    EXPECT_FALSE(inv.reserve("A", 10));
}

TEST(InventoryTest, ReserveReturnsFalseForZeroQty)
{
    eval::Inventory inv;
    inv.add("A", 5);
    EXPECT_FALSE(inv.reserve("A", 0));
}

TEST(InventoryTest, ReserveReducesQuantity)
{
    eval::Inventory inv;
    inv.add("A", 10);
    EXPECT_TRUE(inv.reserve("A", 3));
    EXPECT_EQ(inv.findByName("A")->getQuantity(), 7);
}

TEST(InventoryTest, ReserveDoesNotRemoveItem)
{
    eval::Inventory inv;
    inv.add("A", 5);
    inv.reserve("A", 5); // reserves all
    EXPECT_EQ(inv.size(), 1u); // item record stays
    EXPECT_EQ(inv.findByName("A")->getQuantity(), 0);
}

// ============================================================================
// setLowStockCallback()
// ============================================================================

TEST(InventoryTest, LowStockCallbackFiredWhenQuantityDropsBelowThreshold)
{
    eval::Inventory inv;
    inv.add("Widget", 10);

    bool fired = false;
    std::string firedName;
    int firedQty = 0;
    inv.setLowStockCallback([&](const std::string& name, int qty) {
        fired     = true;
        firedName = name;
        firedQty  = qty;
    }, 5);

    inv.add("Widget", -8); // qty → 2, below threshold 5
    EXPECT_TRUE(fired);
    EXPECT_EQ(firedName, "Widget");
    EXPECT_EQ(firedQty,  2);
}

TEST(InventoryTest, LowStockCallbackNotFiredWhenAboveThreshold)
{
    eval::Inventory inv;
    inv.add("Widget", 10);

    bool fired = false;
    inv.setLowStockCallback([&](const std::string&, int) { fired = true; }, 5);

    inv.add("Widget", -3); // qty → 7, still above 5
    EXPECT_FALSE(fired);
}

TEST(InventoryTest, LowStockCallbackNotFiredWhenNotSet)
{
    // No callback set — should not crash on quantity change.
    eval::Inventory inv;
    inv.add("Widget", 10);
    EXPECT_NO_THROW(inv.add("Widget", -9));
}

TEST(InventoryTest, LowStockCallbackFiredByReserve)
{
    eval::Inventory inv;
    inv.add("A", 10);

    bool fired = false;
    inv.setLowStockCallback([&](const std::string&, int) { fired = true; }, 5);
    inv.reserve("A", 8); // qty → 2
    EXPECT_TRUE(fired);
}

// ============================================================================
// executeCommand() / undo()
// ============================================================================

TEST(InventoryTest, ExecuteAddCommandInsertsItem)
{
    eval::Inventory inv;
    inv.executeCommand(std::make_unique<eval::AddCommand>(inv, "Widget", 5));
    EXPECT_EQ(inv.size(), 1u);
    ASSERT_NE(inv.findByName("Widget"), nullptr);
    EXPECT_EQ(inv.findByName("Widget")->getQuantity(), 5);
}

TEST(InventoryTest, UndoAddCommandRemovesInsertedItem)
{
    eval::Inventory inv;
    inv.executeCommand(std::make_unique<eval::AddCommand>(inv, "Widget", 5));
    EXPECT_TRUE(inv.undo());
    EXPECT_EQ(inv.size(), 0u);
}

TEST(InventoryTest, ExecuteRemoveCommandRemovesItem)
{
    eval::Inventory inv;
    inv.add("Widget", 5);
    inv.executeCommand(std::make_unique<eval::RemoveCommand>(inv, "Widget"));
    EXPECT_EQ(inv.size(), 0u);
}

TEST(InventoryTest, UndoRemoveCommandRestoresItem)
{
    eval::Inventory inv;
    inv.add(eval::Item{"Widget", 5, "Device", "LOT-1", "2030-01-01", 2.5});
    inv.executeCommand(std::make_unique<eval::RemoveCommand>(inv, "Widget"));
    EXPECT_TRUE(inv.undo());
    const eval::Item* item = inv.findByName("Widget");
    ASSERT_NE(item, nullptr);
    EXPECT_EQ(item->getQuantity(),  5);
    EXPECT_EQ(item->getCategory(),  "Device");
    EXPECT_EQ(item->getLotNumber(), "LOT-1");
}

TEST(InventoryTest, UndoReturnsFalseWhenHistoryEmpty)
{
    eval::Inventory inv;
    EXPECT_FALSE(inv.undo());
}

// ============================================================================
// Iterator
// ============================================================================

TEST(InventoryTest, RangeBasedForLoopTraversesAllItems)
{
    eval::Inventory inv;
    inv.add("A", 1);
    inv.add("B", 2);
    inv.add("C", 3);

    int count = 0;
    int total = 0;
    for (const eval::Item& item : inv)
    {
        ++count;
        total += item.getQuantity();
    }
    EXPECT_EQ(count, 3);
    EXPECT_EQ(total, 6);
}

TEST(InventoryTest, BeginEqualsEndForEmptyInventory)
{
    eval::Inventory inv;
    EXPECT_EQ(inv.begin(), inv.end());
}

TEST(InventoryTest, CbeginCendWorkOnConstInventory)
{
    eval::Inventory inv;
    inv.add("A", 1);
    const eval::Inventory& cinv = inv;
    int count = 0;
    for (auto it = cinv.cbegin(); it != cinv.cend(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 1);
}

// ============================================================================
// operator<<
// ============================================================================

TEST(InventoryTest, StreamOperatorProducesCorrectOutputNoMetadata)
{
    eval::Inventory inv;
    inv.add("Apple", 5);
    std::ostringstream oss;
    oss << inv;
    EXPECT_EQ(oss.str(), "  Apple: 5\n");
}

TEST(InventoryTest, StreamOperatorIncludesMetadataWhenPresent)
{
    eval::Inventory inv;
    inv.add(eval::Item{"Lancet", 10, "Consumable", "LOT-1", "2028-01-01", 0.12});
    std::ostringstream oss;
    oss << inv;
    const std::string out = oss.str();
    EXPECT_NE(out.find("Lancet"), std::string::npos);
    EXPECT_NE(out.find("Consumable"), std::string::npos);
    EXPECT_NE(out.find("LOT-1"), std::string::npos);
}

// ============================================================================
// print()
// ============================================================================

TEST(InventoryTest, PrintProducesNoOutputForEmptyInventory)
{
    const eval::Inventory inv;
    EXPECT_TRUE(captureStdout([&] { inv.print(); }).empty());
}

TEST(InventoryTest, PrintFormatsSingleItemCorrectly)
{
    eval::Inventory inv;
    inv.add("Sensor", 42);
    EXPECT_EQ(captureStdout([&] { inv.print(); }), "  Sensor: 42\n");
}

TEST(InventoryTest, PrintFormatsMultipleItemsCorrectly)
{
    eval::Inventory inv;
    inv.add("Alpha", 1);
    inv.add("Beta",  2);
    const std::string expected = "  Alpha: 1\n  Beta: 2\n";
    EXPECT_EQ(captureStdout([&] { inv.print(); }), expected);
}

TEST(InventoryTest, PrintRespectsInsertionOrderBeforeSort)
{
    eval::Inventory inv;
    inv.add("Zulu",  1);
    inv.add("Alpha", 2);
    const std::string expected = "  Zulu: 1\n  Alpha: 2\n";
    EXPECT_EQ(captureStdout([&] { inv.print(); }), expected);
}

// ============================================================================
// Persistence — CSV save / load / writeReport
// ============================================================================

class InventoryIOTest : public ::testing::Test
{
protected:
    const std::string csvPath_    = "test_inv_save.csv";
    const std::string reportPath_ = "test_inv_report.txt";

    void TearDown() override
    {
        std::remove(csvPath_.c_str());
        std::remove(reportPath_.c_str());
    }
};

TEST_F(InventoryIOTest, SaveCreatesFile)
{
    eval::Inventory inv;
    inv.add("A", 1);
    EXPECT_TRUE(inv.save(csvPath_));

    std::ifstream ifs(csvPath_);
    EXPECT_TRUE(ifs.is_open());
}

TEST_F(InventoryIOTest, SaveLoadRoundTrip)
{
    eval::Inventory inv;
    inv.add(eval::Item{"Lancet",  100, "Consumable", "LOT-001", "2028-01-01", 0.12});
    inv.add(eval::Item{"Meter",     3, "Device",     "LOT-002", "2031-12-31", 89.99});
    ASSERT_TRUE(inv.save(csvPath_));

    eval::Inventory loaded;
    ASSERT_TRUE(loaded.load(csvPath_));

    EXPECT_EQ(loaded.size(),          inv.size());
    EXPECT_EQ(loaded.totalQuantity(), inv.totalQuantity());

    const eval::Item* lancet = loaded.findByName("Lancet");
    ASSERT_NE(lancet, nullptr);
    EXPECT_EQ(lancet->getCategory(),   "Consumable");
    EXPECT_EQ(lancet->getLotNumber(),  "LOT-001");
    EXPECT_EQ(lancet->getExpiryDate(), "2028-01-01");
    EXPECT_DOUBLE_EQ(lancet->getUnitPrice(), 0.12);
}

TEST_F(InventoryIOTest, LoadReturnsFalseForMissingFile)
{
    eval::Inventory inv;
    EXPECT_FALSE(inv.load("nonexistent_file_xyz.csv"));
}

TEST_F(InventoryIOTest, LoadClearsExistingItems)
{
    eval::Inventory inv;
    inv.add("Existing", 99);

    eval::Inventory src;
    src.add("New", 1);
    ASSERT_TRUE(src.save(csvPath_));

    inv.load(csvPath_);
    EXPECT_EQ(inv.findByName("Existing"), nullptr);
    EXPECT_NE(inv.findByName("New"),      nullptr);
}

TEST_F(InventoryIOTest, WriteReportCreatesNonEmptyFile)
{
    eval::Inventory inv;
    inv.add("A", 1);
    EXPECT_TRUE(inv.writeReport(reportPath_));

    std::ifstream ifs(reportPath_);
    EXPECT_TRUE(ifs.is_open());
    std::string content((std::istreambuf_iterator<char>(ifs)),
                         std::istreambuf_iterator<char>());
    EXPECT_FALSE(content.empty());
}

// ============================================================================
// Value semantics – copy
// ============================================================================

TEST(InventoryTest, CopyConstructorCreatesIndependentInventory)
{
    eval::Inventory original;
    original.add("X", 5);

    eval::Inventory copy(original);
    copy.add("X", 10);

    EXPECT_EQ(original.totalQuantity(), 5);
    EXPECT_EQ(copy.totalQuantity(),     15);
}

TEST(InventoryTest, CopyAssignmentCreatesIndependentInventory)
{
    eval::Inventory src;
    src.add("Y", 3);

    eval::Inventory dst;
    dst = src;
    dst.add("Y", 7);

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
