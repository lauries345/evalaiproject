#include "IInventoryObserver.h"
#include "Inventory.h"
#include "Item.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

/**
 * @file test_Inventory.cpp
 * @brief Unit tests for eval::Inventory. Targets 100% line and branch coverage.
 */

namespace
{

class RecordingObserver final : public eval::IInventoryObserver
{
public:
    void onLowStock(const eval::Item& item) override
    {
        m_events.push_back(item.getName());
    }

    [[nodiscard]] const std::vector<std::string>& events() const noexcept
    {
        return m_events;
    }

private:
    std::vector<std::string> m_events;
};

class InventoryTest : public ::testing::Test
{
protected:
    eval::Inventory inv;
};

TEST_F(InventoryTest, AddNewReturnsTrueAndIncreasesSize)
{
    EXPECT_TRUE(inv.add(eval::Item{"Apples", 10, 3, 0.50}));
    EXPECT_EQ(inv.size(), 1u);
}

TEST_F(InventoryTest, AddExistingMergesQuantityReturnsFalse)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    EXPECT_FALSE(inv.add(eval::Item{"Apples", 5, 3, 0.50}));
    EXPECT_EQ(inv.size(), 1u);
    EXPECT_EQ(inv.find("Apples")->getQuantity(), 15);
}

TEST_F(InventoryTest, RemoveExistingReturnsTrue)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    EXPECT_TRUE(inv.remove("Apples"));
    EXPECT_EQ(inv.size(), 0u);
}

TEST_F(InventoryTest, RemoveMissingReturnsFalse)
{
    EXPECT_FALSE(inv.remove("Ghost"));
}

TEST_F(InventoryTest, FindMissingReturnsNullopt)
{
    EXPECT_FALSE(inv.find("Ghost").has_value());
}

TEST_F(InventoryTest, FindExistingReturnsItem)
{
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    const auto found = inv.find("Apples");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->getQuantity(), 10);
}

TEST_F(InventoryTest, ItemsAccessorExposesContents)
{
    inv.add(eval::Item{"A", 1, 0, 0.0});
    inv.add(eval::Item{"B", 2, 0, 0.0});
    EXPECT_EQ(inv.items().size(), 2u);
}

TEST_F(InventoryTest, AttachNullObserverThrows)
{
    EXPECT_THROW(inv.attach(nullptr), std::invalid_argument);
}

TEST_F(InventoryTest, DetachUnknownObserverIsNoOp)
{
    RecordingObserver obs;
    EXPECT_NO_THROW(inv.detach(&obs));
}

TEST_F(InventoryTest, DetachPreviouslyAttachedObserverStopsNotifications)
{
    RecordingObserver obs;
    inv.attach(&obs);
    inv.detach(&obs);
    inv.add(eval::Item{"Apples", 1, 3, 0.50});
    EXPECT_TRUE(obs.events().empty());
}

TEST_F(InventoryTest, ObserverFiresOnInsertingLowStockItem)
{
    RecordingObserver obs;
    inv.attach(&obs);
    inv.add(eval::Item{"Apples", 1, 3, 0.50});
    ASSERT_EQ(obs.events().size(), 1u);
    EXPECT_EQ(obs.events().front(), "Apples");
}

TEST_F(InventoryTest, ObserverDoesNotFireOnInsertingAboveThreshold)
{
    RecordingObserver obs;
    inv.attach(&obs);
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    EXPECT_TRUE(obs.events().empty());
}

TEST_F(InventoryTest, ObserverDoesNotFireOnMerge)
{
    RecordingObserver obs;
    inv.add(eval::Item{"Apples", 10, 3, 0.50});
    inv.attach(&obs);
    inv.add(eval::Item{"Apples", 5, 3, 0.50});
    EXPECT_TRUE(obs.events().empty());
}

} // namespace
