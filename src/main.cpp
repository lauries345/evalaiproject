/**
 * @file        main.cpp
 * @brief       Application entry point – inventory management demonstration.
 * @details     Exercises the Inventory and Item classes by performing a
 *              representative sequence of add, sort, and remove operations,
 *              then printing the results to standard output.
 *
 *              This file serves as the integration-level demonstration for
 *              the EvalAIProject evaluation build.
 *
 * @author      Seth Laurie
 * @date        2026-05-15
 * @version     1.0.0
 *
 * @copyright   Copyright (c) 2026. All rights reserved.
 *
 * @par Revision History:
 * | Version | Date       | Author      | Description     |
 * |---------|------------|-------------|-----------------|
 * | 1.0.0   | 2026-05-15 | Seth Laurie | Initial release |
 *
 * @par Safety Classification:
 * Evaluation / non-production component. Must be formally reclassified as
 * IEC 62304 Class A, B, or C before integration into a regulated medical
 * device. All changes require documented change control.
 *
 * @par Applicable Standards:
 * - IEC 62304: Medical device software – Software life cycle processes
 * - ISO 14971: Medical devices – Application of risk management to medical devices
 */

#include "Inventory.h"

#include <iostream>

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

/**
 * @brief   Application entry point.
 * @details Demonstrates core Inventory operations: add (with duplicate
 *          merging), total quantity computation, lexicographic sort, and
 *          item removal.
 * @return  0 on normal termination.
 */
int main()
{
    eval::Inventory inv;

    // Populate the inventory. The second "Apples" entry is merged into the
    // first, resulting in a combined quantity of 13.
    inv.add("Apples",  10);
    inv.add("Bananas",  5);
    inv.add("Oranges",  8);
    inv.add("Apples",   3);

    std::cout << "--- Initial inventory ---\n";
    inv.print();
    std::cout << "Total items: " << inv.totalQuantity() << "\n\n";

    inv.sortByName();
    std::cout << "--- Sorted by name ---\n";
    inv.print();
    std::cout << "\n";

    inv.remove("Bananas");
    std::cout << "--- After removing Bananas ---\n";
    inv.print();

    return 0;
}
