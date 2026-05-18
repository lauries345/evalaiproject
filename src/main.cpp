/**
 * @file        main.cpp
 * @brief       Application entry point – expanded inventory management demonstration.
 * @details     Exercises all Inventory and Item capabilities across seven sections:
 *
 *                1. Basic setup with extended Item metadata
 *                2. Querying and filtering
 *                3. Item model expansion (category, lot, expiry, price)
 *                4. Inventory operations (transfer, reserve, clear, merge)
 *                5. Persistence (CSV save/load, structured report)
 *                6. Error and edge-case driving
 *                7. Design patterns (command+undo, observer, iterator)
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     2.0.0
 *
 * @copyright   Copyright (c) 2026. All rights reserved.
 *
 * @par Revision History:
 * | Version | Date       | Author      | Description                          |
 * |---------|------------|-------------|--------------------------------------|
 * | 1.0.0   | 2026-05-15 | Seth Laurie | Initial release                      |
 * | 2.0.0   | 2026-05-15 | Seth Laurie | Expanded to demonstrate all features |
 *
 * @par Safety Classification:
 * Evaluation / non-production component. Must be formally reclassified as
 * IEC 62304 Class A, B, or C before integration into a regulated medical
 * device. All changes require documented change control.
 */

#include "Command.h"
#include "Inventory.h"

#include <iostream>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static void printSection(const std::string& title)
{
    std::cout << "\n=== " << title << " ===\n";
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int main()
{
    // =========================================================================
    // 1. Basic setup — items with full metadata
    // =========================================================================
    printSection("1. Basic setup with extended Item metadata");

    eval::Inventory inv;

    // Items constructed with category, lot number, expiry date, and unit price.
    inv.add(eval::Item{"Glucose Strips",  50, "Reagent", "LOT-GS-001", "2027-06-30", 0.45});
    inv.add(eval::Item{"Lancets",        200, "Consumable", "LOT-LC-042", "2028-01-15", 0.12});
    inv.add(eval::Item{"Control Solution", 8, "Reagent", "LOT-CS-007", "2026-11-01", 3.75});
    inv.add(eval::Item{"Test Meter",       3, "Device",  "LOT-TM-019", "2031-12-31", 89.99});

    // Duplicate add — merges quantity into existing Glucose Strips record.
    inv.add("Glucose Strips", 10);

    std::cout << "Initial inventory (operator<<):\n" << inv;
    std::cout << "Total qty   : " << inv.totalQuantity() << '\n';
    std::cout << "Total value : $" << inv.totalValue()   << '\n';

    // =========================================================================
    // 2. Querying and filtering
    // =========================================================================
    printSection("2. Querying and filtering");

    // findByName — returns a pointer or nullptr
    const eval::Item* meter = inv.findByName("Test Meter");
    if (meter != nullptr)
    {
        std::cout << "Found: " << meter->getName()
                  << " (lot " << meter->getLotNumber() << ")\n";
    }

    // findBelowThreshold — low-stock alert (qty < 10)
    std::cout << "\nItems with qty < 10 (low-stock):\n";
    for (const eval::Item& item : inv.findBelowThreshold(10))
    {
        std::cout << "  [!] " << item.getName() << ": " << item.getQuantity() << '\n';
    }

    // findByRange — items with qty in [5, 60]
    std::cout << "\nItems with qty in [5, 60]:\n";
    for (const eval::Item& item : inv.findByRange(5, 60))
    {
        std::cout << "  " << item.getName() << ": " << item.getQuantity() << '\n';
    }

    // sortByQuantity — ascending
    inv.sortByQuantity();
    std::cout << "\nSorted by quantity (ascending):\n" << inv;

    // sortByName — lexicographic
    inv.sortByName();
    std::cout << "Sorted by name:\n" << inv;

    // =========================================================================
    // 3. Item model expansion — demonstrate metadata accessors
    // =========================================================================
    printSection("3. Item model expansion — traceability metadata");

    std::cout << "Full item details:\n";
    for (const eval::Item& item : inv)
    {
        std::cout << "  Name       : " << item.getName()       << '\n'
                  << "  Category   : " << item.getCategory()   << '\n'
                  << "  Lot Number : " << item.getLotNumber()  << '\n'
                  << "  Expiry Date: " << item.getExpiryDate() << '\n'
                  << "  Unit Price : $" << item.getUnitPrice() << '\n'
                  << "  Quantity   : " << item.getQuantity()   << '\n'
                  << "  ---\n";
    }

    // =========================================================================
    // 4. Inventory operations
    // =========================================================================
    printSection("4. Inventory operations");

    // transfer — move 30 Lancets to a secondary inventory
    eval::Inventory secondary;
    bool ok = inv.transfer("Lancets", 30, secondary);
    std::cout << "Transfer 30 Lancets to secondary: " << (ok ? "OK" : "FAILED") << '\n';
    std::cout << "Primary Lancets qty  : " << inv.findByName("Lancets")->getQuantity() << '\n';
    std::cout << "Secondary inventory:\n" << secondary;

    // reserve — reserve 5 Control Solutions for an upcoming procedure
    ok = inv.reserve("Control Solution", 5);
    std::cout << "\nReserve 5 Control Solutions: " << (ok ? "OK" : "FAILED") << '\n';
    std::cout << "Control Solution qty after reserve: "
              << inv.findByName("Control Solution")->getQuantity() << '\n';

    // merge — combine secondary back into primary
    inv.merge(secondary);
    std::cout << "\nAfter merging secondary back into primary:\n" << inv;

    // clear — wipe a secondary copy and confirm it is empty
    secondary.clear();
    std::cout << "Secondary size after clear: " << secondary.size() << '\n';

    // =========================================================================
    // 5. Persistence — CSV save/load and structured report
    // =========================================================================
    printSection("5. Persistence");

    const std::string csvPath    = "inventory_snapshot.csv";
    const std::string reportPath = "inventory_report.txt";

    // Save
    ok = inv.save(csvPath);
    std::cout << "Save to '" << csvPath << "': " << (ok ? "OK" : "FAILED") << '\n';

    // Load into a fresh inventory and compare totals
    eval::Inventory loaded;
    ok = loaded.load(csvPath);
    std::cout << "Load from '" << csvPath << "': " << (ok ? "OK" : "FAILED") << '\n';
    std::cout << "Loaded inventory:\n" << loaded;
    std::cout << "Total qty matches: "
              << (loaded.totalQuantity() == inv.totalQuantity() ? "YES" : "NO") << '\n';

    // Structured report
    ok = inv.writeReport(reportPath, 10);
    std::cout << "Write report to '" << reportPath << "': " << (ok ? "OK" : "FAILED") << '\n';

    // =========================================================================
    // 6. Error and edge-case driving
    // =========================================================================
    printSection("6. Error and edge-case driving");

    // Guard: caller validates before calling add() with non-positive quantity
    const int newQty = 0;
    if (newQty > 0)
    {
        inv.add("Phantom Item", newQty);
    }
    else
    {
        std::cout << "Refused to add new item with qty=" << newQty
                  << " (caller guard applied)\n";
    }

    // remove — item that does not exist returns false
    bool removed = inv.remove("NonExistentItem");
    std::cout << "Remove 'NonExistentItem': " << (removed ? "true" : "false (expected)\n");

    // transfer — more than available
    ok = inv.transfer("Test Meter", 999, secondary);
    std::cout << "Transfer 999 Test Meters (exceeds stock): "
              << (ok ? "OK" : "FAILED (expected)\n");

    // reserve — more than available
    ok = inv.reserve("Control Solution", 9999);
    std::cout << "Reserve 9999 Control Solutions (exceeds stock): "
              << (ok ? "OK" : "FAILED (expected)\n");

    // Round-trip: add then immediately remove
    inv.add("TempItem", 1);
    removed = inv.remove("TempItem");
    std::cout << "Add then remove 'TempItem': "
              << (removed ? "round-trip OK" : "FAILED") << '\n';
    std::cout << "'TempItem' still present: "
              << (inv.findByName("TempItem") != nullptr ? "yes" : "no (expected)\n");

    // =========================================================================
    // 7. Design patterns
    // =========================================================================
    printSection("7. Design patterns");

    // --- Observer / callback ---
    std::cout << "-- Observer: low-stock callback (threshold = 10) --\n";
    inv.setLowStockCallback(
        [](const std::string& name, int qty)
        {
            std::cout << "  [ALERT] Low stock: " << name << " = " << qty << '\n';
        },
        10);

    // Trigger callback by reducing Lancets below 10
    inv.add("Lancets", -200); // drives qty very low

    // --- Command pattern with undo ---
    std::cout << "\n-- Command pattern with undo --\n";
    eval::Inventory cmdInv;
    cmdInv.add(eval::Item{"Syringe", 100, "Consumable", "LOT-SY-001", "2028-06-01", 0.30});

    std::cout << "Before command: Syringe qty = "
              << cmdInv.findByName("Syringe")->getQuantity() << '\n';

    // Execute an add command (recorded in undo stack)
    cmdInv.executeCommand(std::make_unique<eval::AddCommand>(cmdInv, "Syringe", 50));
    std::cout << "After AddCommand(+50): Syringe qty = "
              << cmdInv.findByName("Syringe")->getQuantity() << '\n';

    // Execute a remove command
    cmdInv.executeCommand(std::make_unique<eval::RemoveCommand>(cmdInv, "Syringe"));
    std::cout << "After RemoveCommand: Syringe present = "
              << (cmdInv.findByName("Syringe") != nullptr ? "yes" : "no") << '\n';

    // Undo remove — restores Syringe with all original metadata
    cmdInv.undo();
    const eval::Item* syringe = cmdInv.findByName("Syringe");
    std::cout << "After undo (RemoveCommand): Syringe qty = "
              << (syringe ? std::to_string(syringe->getQuantity()) : "missing") << '\n';

    // Undo add — quantity returns to 100
    cmdInv.undo();
    syringe = cmdInv.findByName("Syringe");
    std::cout << "After undo (AddCommand): Syringe qty = "
              << (syringe ? std::to_string(syringe->getQuantity()) : "missing") << '\n';

    // Undo on empty history returns false
    bool undone = cmdInv.undo();
    std::cout << "Undo with empty history: " << (undone ? "true" : "false (expected)\n");

    // --- Iterator — range-based for loop ---
    std::cout << "\n-- Iterator: range-based for loop --\n";
    int total = 0;
    for (const eval::Item& item : cmdInv)
    {
        total += item.getQuantity();
        std::cout << "  iterating: " << item.getName() << '\n';
    }
    std::cout << "Iterator total qty: " << total << '\n';

    return 0;
}
