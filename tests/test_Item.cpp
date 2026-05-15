/**
 * @file        test_Item.cpp
 * @brief       Unit tests for the eval::Item class.
 * @details     Verifies all public methods of Item:
 *              - Constructor (name + quantity storage)
 *              - getName()
 *              - getQuantity()
 *              - adjustQuantity() (positive, negative, and zero deltas)
 *              - Value semantics (copy and move construction/assignment)
 *
 *              Designed to achieve 100 % line and branch coverage of
 *              src/Item.cpp and include/Item.h.
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

#include "Item.h"

#include <gtest/gtest.h>

#include <string>
#include <utility>

namespace {

// ============================================================================
// Construction
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
    // Verify we get a reference, not a copy, by comparing addresses.
    const std::string& ref = item.getName();
    EXPECT_EQ(&ref, &item.getName());
}

TEST(ItemTest, GetQuantityReturnsCurrentValue)
{
    const eval::Item item("Counter", 42);
    EXPECT_EQ(item.getQuantity(), 42);
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
    // Expected: 0 + 10 - 3 + 7 = 14
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

TEST(ItemTest, CopyConstructorReplicatesNameAndQuantity)
{
    const eval::Item original("Original", 7);
    const eval::Item copy(original);
    EXPECT_EQ(copy.getName(),     original.getName());
    EXPECT_EQ(copy.getQuantity(), original.getQuantity());
}

TEST(ItemTest, CopyConstructorIsIndependentOfOriginal)
{
    eval::Item original("Independent", 5);
    eval::Item copy(original);
    copy.adjustQuantity(100);
    EXPECT_EQ(original.getQuantity(), 5);   // original must be unchanged
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
    EXPECT_EQ(src.getQuantity(), 20);   // source must be unchanged
    EXPECT_EQ(dst.getQuantity(), 15);
}

// ============================================================================
// Value semantics – move
// ============================================================================

TEST(ItemTest, MoveConstructorTransfersNameAndQuantity)
{
    eval::Item       source("Movable", 15);
    const eval::Item dest(std::move(source));
    EXPECT_EQ(dest.getName(),     "Movable");
    EXPECT_EQ(dest.getQuantity(), 15);
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
