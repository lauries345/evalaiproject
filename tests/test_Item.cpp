/**
 * @file        test_Item.cpp
 * @brief       Unit tests for the eval::Item class.
 * @details     Verifies all public methods of Item:
 *              - Constructor (name + quantity, and full six-argument form)
 *              - getName(), getQuantity()
 *              - getCategory(), getLotNumber(), getExpiryDate(), getUnitPrice()
 *              - adjustQuantity() (positive, negative, and zero deltas)
 *              - Value semantics (copy and move construction/assignment)
 *
 *              Designed to achieve 100 % line and branch coverage of
 *              src/Item.cpp and include/Item.h.
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

#include "Item.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace {

// ============================================================================
// Construction — two-argument form (backward-compatible)
// ============================================================================

TEST(ItemTest, ConstructorStoresName)
{
    const eval::Item item("Widget", 10);
    EXPECT_EQ(item.getName(), "Widget");
}

TEST(ItemTest, ConstructorStoresQuantity)
{
    const eval::Item item("Widget", 10);
    EXPECT_EQ(item.getQuantity(), 10);
}

TEST(ItemTest, ConstructorAcceptsZeroQuantity)
{
    const eval::Item item("EmptyBin", 0);
    EXPECT_EQ(item.getQuantity(), 0);
}

TEST(ItemTest, ConstructorAcceptsNegativeQuantity)
{
    // Negative stock is a valid domain value (e.g. back-order / deficit).
    const eval::Item item("Deficit", -5);
    EXPECT_EQ(item.getQuantity(), -5);
}

// ============================================================================
// Construction — six-argument form (extended metadata)
// ============================================================================

TEST(ItemTest, FullConstructorStoresCategory)
{
    const eval::Item item("Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12);
    EXPECT_EQ(item.getCategory(), "Consumable");
}

TEST(ItemTest, FullConstructorStoresLotNumber)
{
    const eval::Item item("Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12);
    EXPECT_EQ(item.getLotNumber(), "LOT-001");
}

TEST(ItemTest, FullConstructorStoresExpiryDate)
{
    const eval::Item item("Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12);
    EXPECT_EQ(item.getExpiryDate(), "2028-01-01");
}

TEST(ItemTest, FullConstructorStoresUnitPrice)
{
    const eval::Item item("Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12);
    EXPECT_DOUBLE_EQ(item.getUnitPrice(), 0.12);
}

// ============================================================================
// Construction — default metadata values when only name+qty provided
// ============================================================================

TEST(ItemTest, DefaultCategoryIsEmpty)
{
    const eval::Item item("Widget", 5);
    EXPECT_TRUE(item.getCategory().empty());
}

TEST(ItemTest, DefaultLotNumberIsEmpty)
{
    const eval::Item item("Widget", 5);
    EXPECT_TRUE(item.getLotNumber().empty());
}

TEST(ItemTest, DefaultExpiryDateIsEmpty)
{
    const eval::Item item("Widget", 5);
    EXPECT_TRUE(item.getExpiryDate().empty());
}

TEST(ItemTest, DefaultUnitPriceIsZero)
{
    const eval::Item item("Widget", 5);
    EXPECT_DOUBLE_EQ(item.getUnitPrice(), 0.0);
}

// ============================================================================
// Accessors
// ============================================================================

TEST(ItemTest, GetNameReturnsCorrectString)
{
    const eval::Item item("MedicalGrade", 1);
    EXPECT_EQ(item.getName(), "MedicalGrade");
}

TEST(ItemTest, GetNameReturnsConstReference)
{
    const eval::Item item("Ref", 1);
    const std::string& ref = item.getName();
    EXPECT_EQ(&ref, &item.getName());
}

TEST(ItemTest, GetQuantityReturnsCurrentValue)
{
    const eval::Item item("Counter", 42);
    EXPECT_EQ(item.getQuantity(), 42);
}

TEST(ItemTest, GetCategoryReturnsConstReference)
{
    const eval::Item item("X", 1, "Cat", "", "", 0.0);
    const std::string& ref = item.getCategory();
    EXPECT_EQ(&ref, &item.getCategory());
}

TEST(ItemTest, GetLotNumberReturnsConstReference)
{
    const eval::Item item("X", 1, "", "LOT-99", "", 0.0);
    const std::string& ref = item.getLotNumber();
    EXPECT_EQ(&ref, &item.getLotNumber());
}

TEST(ItemTest, GetExpiryDateReturnsConstReference)
{
    const eval::Item item("X", 1, "", "", "2030-01-01", 0.0);
    const std::string& ref = item.getExpiryDate();
    EXPECT_EQ(&ref, &item.getExpiryDate());
}

// ============================================================================
// adjustQuantity
// ============================================================================

TEST(ItemTest, AdjustQuantityPositiveDeltaIncreases)
{
    eval::Item item("Stock", 10);
    item.adjustQuantity(5);
    EXPECT_EQ(item.getQuantity(), 15);
}

TEST(ItemTest, AdjustQuantityNegativeDeltaDecreases)
{
    eval::Item item("Stock", 10);
    item.adjustQuantity(-3);
    EXPECT_EQ(item.getQuantity(), 7);
}

TEST(ItemTest, AdjustQuantityZeroDeltaLeavesValueUnchanged)
{
    eval::Item item("Stock", 10);
    item.adjustQuantity(0);
    EXPECT_EQ(item.getQuantity(), 10);
}

TEST(ItemTest, AdjustQuantityMultipleCallsAccumulate)
{
    eval::Item item("Accumulator", 0);
    item.adjustQuantity(10);
    item.adjustQuantity(-3);
    item.adjustQuantity(7);
    EXPECT_EQ(item.getQuantity(), 14);
}

TEST(ItemTest, AdjustQuantityFromNegativeBase)
{
    eval::Item item("Deficit", -5);
    item.adjustQuantity(8);
    EXPECT_EQ(item.getQuantity(), 3);
}

// ============================================================================
// Value semantics – copy
// ============================================================================

TEST(ItemTest, CopyConstructorReplicatesAllFields)
{
    const eval::Item original("Lancet", 100, "Consumable", "LOT-001", "2028-01-01", 0.12);
    const eval::Item copy(original);
    EXPECT_EQ(copy.getName(),       original.getName());
    EXPECT_EQ(copy.getQuantity(),   original.getQuantity());
    EXPECT_EQ(copy.getCategory(),   original.getCategory());
    EXPECT_EQ(copy.getLotNumber(),  original.getLotNumber());
    EXPECT_EQ(copy.getExpiryDate(), original.getExpiryDate());
    EXPECT_DOUBLE_EQ(copy.getUnitPrice(), original.getUnitPrice());
}

TEST(ItemTest, CopyConstructorIsIndependentOfOriginal)
{
    eval::Item original("Independent", 5);
    eval::Item copy(original);
    copy.adjustQuantity(100);
    EXPECT_EQ(original.getQuantity(), 5);
    EXPECT_EQ(copy.getQuantity(),     105);
}

TEST(ItemTest, CopyAssignmentReplicatesNameAndQuantity)
{
    const eval::Item src("Source", 3);
    eval::Item       dst("Placeholder", 99);
    dst = src;
    EXPECT_EQ(dst.getName(),     "Source");
    EXPECT_EQ(dst.getQuantity(), 3);
}

TEST(ItemTest, CopyAssignmentIsIndependentOfSource)
{
    eval::Item src("Src", 20);
    eval::Item dst("Dst", 0);
    dst = src;
    dst.adjustQuantity(-5);
    EXPECT_EQ(src.getQuantity(), 20);
    EXPECT_EQ(dst.getQuantity(), 15);
}

// ============================================================================
// Value semantics – move
// ============================================================================

TEST(ItemTest, MoveConstructorTransfersAllFields)
{
    eval::Item       source("Movable", 15, "Device", "LOT-M", "2029-01-01", 5.0);
    const eval::Item dest(std::move(source));
    EXPECT_EQ(dest.getName(),       "Movable");
    EXPECT_EQ(dest.getQuantity(),   15);
    EXPECT_EQ(dest.getCategory(),   "Device");
    EXPECT_EQ(dest.getLotNumber(),  "LOT-M");
    EXPECT_EQ(dest.getExpiryDate(), "2029-01-01");
    EXPECT_DOUBLE_EQ(dest.getUnitPrice(), 5.0);
}

TEST(ItemTest, MoveAssignmentTransfersNameAndQuantity)
{
    eval::Item source("MoveAssign", 22);
    eval::Item dest("Placeholder", 0);
    dest = std::move(source);
    EXPECT_EQ(dest.getName(),     "MoveAssign");
    EXPECT_EQ(dest.getQuantity(), 22);
}

} // namespace
