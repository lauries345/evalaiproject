#pragma once

/**
 * @file IInventoryObserver.h
 * @brief Observer interface for Inventory state changes (GoF Observer).
 *
 * @author  EvalAIProject team
 * @date    2026-05-18
 * @version 1.0.0
 *
 * @par Safety Classification
 * IEC 62304 Class A. Change-controlled per project SOP.
 *
 * @par Applicable Standards
 * IEC 62304, ISO 14971, MISRA C++ 2008, ISO/IEC 14882:2017
 */

namespace eval
{

class Item;

/**
 * @brief Subscribe to Inventory state events.
 *
 * Observers are attached via Inventory::attach() and detached via
 * Inventory::detach(). The observer object must outlive its registration.
 */
class IInventoryObserver
{
public:
    IInventoryObserver()                                     = default;
    IInventoryObserver(const IInventoryObserver&)            = delete;
    IInventoryObserver& operator=(const IInventoryObserver&) = delete;
    IInventoryObserver(IInventoryObserver&&)                 = delete;
    IInventoryObserver& operator=(IInventoryObserver&&)      = delete;
    virtual ~IInventoryObserver()                            = default;

    /**
     * @brief Fired when an item is inserted at quantity <= threshold.
     * @param item the offending record (read-only view).
     */
    virtual void onLowStock(const Item& item) = 0;
};

} // namespace eval
