#include "AddCommand.h"
#include "CommandHistory.h"
#include "IInventoryObserver.h"
#include "Inventory.h"
#include "Item.h"
#include "RemoveCommand.h"

#include <iostream>
#include <memory>

/**
 * @file main.cpp
 * @brief Demo driver wiring Inventory + CommandHistory + observer.
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 */

namespace
{

class ConsoleLogger final : public eval::IInventoryObserver
{
public:
    void onLowStock(const eval::Item& item) override
    {
        std::cout << "[LOW STOCK] " << item.getName()
                  << " qty=" << item.getQuantity()
                  << " <= threshold " << item.getLowStockThreshold() << '\n';
    }
};

void printInventory(const eval::Inventory& inv)
{
    for (const auto& item : inv.items())
    {
        std::cout << "  " << item.getName()
                  << "  qty=" << item.getQuantity()
                  << "  thr=" << item.getLowStockThreshold()
                  << "  $"   << item.getUnitPrice() << '\n';
    }
}

} // namespace

int main()
{
    eval::Inventory      inv;
    eval::CommandHistory history;
    ConsoleLogger        logger;
    inv.attach(&logger);

    history.execute(std::make_unique<eval::AddCommand>(
        inv, eval::Item{"Apples",  10, 3, 0.50}));
    history.execute(std::make_unique<eval::AddCommand>(
        inv, eval::Item{"Bananas",  5, 4, 0.25}));
    history.execute(std::make_unique<eval::AddCommand>(
        inv, eval::Item{"Oranges",  2, 5, 0.75})); // fires [LOW STOCK] for Oranges

    std::cout << "Initial:\n";
    printInventory(inv);

    history.execute(std::make_unique<eval::RemoveCommand>(inv, "Bananas"));
    std::cout << "\nAfter removing Bananas:\n";
    printInventory(inv);

    history.undo();
    std::cout << "\nAfter undo (Bananas restored):\n";
    printInventory(inv);

    history.redo();
    std::cout << "\nAfter redo (Bananas removed again):\n";
    printInventory(inv);

    inv.detach(&logger);
    return 0;
}
