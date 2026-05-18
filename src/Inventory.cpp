/**
 * @file        Inventory.cpp
 * @brief       Inventory container implementation.
 * @details     Provides definitions for all Inventory member functions and
 *              the operator<< free function declared in Inventory.h.
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
 * | 2.0.0   | 2026-05-15 | Seth Laurie | Add queries, persistence, commands,  |
 * |         |            |             | observer, iterator, transfer, merge  |
 *
 * @par Safety Classification:
 * Evaluation / non-production component. Must be formally reclassified as
 * IEC 62304 Class A, B, or C before integration into a regulated medical
 * device. All changes require documented change control.
 */

#include "Inventory.h"
#include "Command.h"    // Required here so ICommand is complete for ~Inventory().

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace eval {

// ===========================================================================
// Construction / destruction
// ===========================================================================

Inventory::Inventory() = default;

Inventory::Inventory(const Inventory& other)
    : m_items(other.m_items)
    , m_lowStockCallback(other.m_lowStockCallback)
    , m_lowStockThreshold(other.m_lowStockThreshold)
    // m_history intentionally omitted — command history is not duplicated.
{
}

Inventory::Inventory(Inventory&& other)
    : m_items(std::move(other.m_items))
    , m_lowStockCallback(std::move(other.m_lowStockCallback))
    , m_lowStockThreshold(other.m_lowStockThreshold)
    , m_history(std::move(other.m_history))
{
}

Inventory& Inventory::operator=(const Inventory& other)
{
    if (this != &other)
    {
        m_items              = other.m_items;
        m_lowStockCallback   = other.m_lowStockCallback;
        m_lowStockThreshold  = other.m_lowStockThreshold;
        m_history.clear(); // history is not duplicated on copy
    }
    return *this;
}

Inventory& Inventory::operator=(Inventory&& other)
{
    if (this != &other)
    {
        m_items             = std::move(other.m_items);
        m_lowStockCallback  = std::move(other.m_lowStockCallback);
        m_lowStockThreshold = other.m_lowStockThreshold;
        m_history           = std::move(other.m_history);
    }
    return *this;
}

// Defined here so that ICommand (forward-declared in Inventory.h) is a
// complete type at the point unique_ptr's deleter is instantiated.
Inventory::~Inventory() = default;

// ===========================================================================
// Private helpers
// ===========================================================================

void Inventory::triggerLowStockIfNeeded(const Item& item)
{
    if (m_lowStockCallback && item.getQuantity() < m_lowStockThreshold)
    {
        m_lowStockCallback(item.getName(), item.getQuantity());
    }
}

// ===========================================================================
// Mutators — primitive operations
// ===========================================================================

void Inventory::add(const std::string& name, int quantity)
{
    add(Item{name, quantity});
}

void Inventory::add(Item item)
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&item](const Item& i) { return i.getName() == item.getName(); });

    if (it != m_items.end())
    {
        it->adjustQuantity(item.getQuantity());
        triggerLowStockIfNeeded(*it);
    }
    else
    {
        m_items.push_back(std::move(item));
        triggerLowStockIfNeeded(m_items.back());
    }
}

bool Inventory::remove(const std::string& name)
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&name](const Item& item) { return item.getName() == name; });

    if (it == m_items.end())
    {
        return false;
    }

    m_items.erase(it);
    return true;
}

void Inventory::sortByName()
{
    std::sort(
        m_items.begin(), m_items.end(),
        [](const Item& lhs, const Item& rhs)
        {
            return lhs.getName() < rhs.getName();
        });
}

void Inventory::sortByQuantity()
{
    std::sort(
        m_items.begin(), m_items.end(),
        [](const Item& lhs, const Item& rhs)
        {
            return lhs.getQuantity() < rhs.getQuantity();
        });
}

void Inventory::clear()
{
    m_items.clear();
}

void Inventory::merge(const Inventory& other)
{
    for (const Item& item : other)
    {
        add(item); // copies item; existing records accumulate quantity
    }
}

bool Inventory::transfer(const std::string& name, int qty, Inventory& destination)
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&name](const Item& item) { return item.getName() == name; });

    if (it == m_items.end() || qty <= 0 || qty > it->getQuantity())
    {
        return false;
    }

    // Transfer qty units with source metadata into destination.
    destination.add(Item{it->getName(),
                         qty,
                         it->getCategory(),
                         it->getLotNumber(),
                         it->getExpiryDate(),
                         it->getUnitPrice()});

    it->adjustQuantity(-qty);
    triggerLowStockIfNeeded(*it);
    return true;
}

bool Inventory::reserve(const std::string& name, int qty)
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&name](const Item& item) { return item.getName() == name; });

    if (it == m_items.end() || qty <= 0 || qty > it->getQuantity())
    {
        return false;
    }

    it->adjustQuantity(-qty);
    triggerLowStockIfNeeded(*it);
    return true;
}

// ===========================================================================
// Queries
// ===========================================================================

const Item* Inventory::findByName(const std::string& name) const
{
    auto it = std::find_if(
        m_items.begin(), m_items.end(),
        [&name](const Item& item) { return item.getName() == name; });

    return (it != m_items.end()) ? &(*it) : nullptr;
}

std::vector<Item> Inventory::findBelowThreshold(int threshold) const
{
    std::vector<Item> result;
    for (const Item& item : m_items)
    {
        if (item.getQuantity() < threshold)
        {
            result.push_back(item);
        }
    }
    return result;
}

std::vector<Item> Inventory::findByRange(int minQty, int maxQty) const
{
    std::vector<Item> result;
    for (const Item& item : m_items)
    {
        if (item.getQuantity() >= minQty && item.getQuantity() <= maxQty)
        {
            result.push_back(item);
        }
    }
    return result;
}

int Inventory::totalQuantity() const noexcept
{
    int total = 0;
    for (const auto& item : m_items)
    {
        total += item.getQuantity();
    }
    return total;
}

double Inventory::totalValue() const noexcept
{
    double total = 0.0;
    for (const auto& item : m_items)
    {
        total += static_cast<double>(item.getQuantity()) * item.getUnitPrice();
    }
    return total;
}

std::size_t Inventory::size() const noexcept
{
    return m_items.size();
}

// ===========================================================================
// Observer
// ===========================================================================

void Inventory::setLowStockCallback(LowStockCallback cb, int threshold)
{
    m_lowStockCallback  = std::move(cb);
    m_lowStockThreshold = threshold;
}

// ===========================================================================
// Command pattern
// ===========================================================================

void Inventory::executeCommand(std::unique_ptr<ICommand> cmd)
{
    cmd->execute();
    m_history.push_back(std::move(cmd));
}

bool Inventory::undo()
{
    if (m_history.empty())
    {
        return false;
    }
    m_history.back()->undo();
    m_history.pop_back();
    return true;
}

// ===========================================================================
// Persistence
// ===========================================================================

bool Inventory::save(const std::string& filename) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        return false;
    }

    ofs << "name,quantity,category,lotNumber,expiryDate,unitPrice\n";
    for (const Item& item : m_items)
    {
        ofs << item.getName()       << ','
            << item.getQuantity()   << ','
            << item.getCategory()   << ','
            << item.getLotNumber()  << ','
            << item.getExpiryDate() << ','
            << std::fixed << std::setprecision(6) << item.getUnitPrice()
            << '\n';
    }
    return ofs.good();
}

bool Inventory::load(const std::string& filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open())
    {
        return false;
    }

    clear();

    std::string line;
    std::getline(ifs, line); // skip header

    while (std::getline(ifs, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::istringstream ss(line);
        std::string name, qtyStr, category, lotNumber, expiryDate, priceStr;

        if (!std::getline(ss, name,       ',') ||
            !std::getline(ss, qtyStr,     ',') ||
            !std::getline(ss, category,   ',') ||
            !std::getline(ss, lotNumber,  ',') ||
            !std::getline(ss, expiryDate, ',') ||
            !std::getline(ss, priceStr,   ','))
        {
            return false;
        }

        int    qty   = 0;
        double price = 0.0;
        try
        {
            qty   = std::stoi(qtyStr);
            price = std::stod(priceStr);
        }
        catch (const std::exception&)
        {
            return false;
        }

        add(Item{name, qty, category, lotNumber, expiryDate, price});
    }
    return true;
}

bool Inventory::writeReport(const std::string& filename, int reportThreshold) const
{
    std::ofstream ofs(filename);
    if (!ofs.is_open())
    {
        return false;
    }

    // Timestamp
    std::time_t now = std::time(nullptr);
    char timeBuf[32];
    std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));

    ofs << "============================================================\n"
        << "  Inventory Report\n"
        << "  Generated: " << timeBuf << '\n'
        << "============================================================\n\n"
        << "  Items  : " << m_items.size()   << '\n'
        << "  Total Qty   : " << totalQuantity()   << '\n'
        << std::fixed << std::setprecision(2)
        << "  Total Value : $" << totalValue()     << "\n\n"
        << "------------------------------------------------------------\n"
        << std::left
        << std::setw(20) << "Name"
        << std::setw(8)  << "Qty"
        << std::setw(14) << "Category"
        << std::setw(14) << "Lot"
        << std::setw(12) << "Expiry"
        << std::setw(10) << "Unit $"
        << '\n'
        << "------------------------------------------------------------\n";

    for (const Item& item : m_items)
    {
        ofs << std::left
            << std::setw(20) << item.getName()
            << std::setw(8)  << item.getQuantity()
            << std::setw(14) << item.getCategory()
            << std::setw(14) << item.getLotNumber()
            << std::setw(12) << item.getExpiryDate()
            << std::fixed << std::setprecision(2) << item.getUnitPrice()
            << '\n';
    }

    // Low-stock section
    std::vector<Item> lowStock = findBelowThreshold(reportThreshold);
    ofs << "\n------------------------------------------------------------\n"
        << "  LOW STOCK (qty < " << reportThreshold << ")\n"
        << "------------------------------------------------------------\n";
    if (lowStock.empty())
    {
        ofs << "  (none)\n";
    }
    else
    {
        for (const Item& item : lowStock)
        {
            ofs << "  [!] " << item.getName()
                << " — qty: " << item.getQuantity() << '\n';
        }
    }
    ofs << "\n============================================================\n";

    return ofs.good();
}

// ===========================================================================
// Output
// ===========================================================================

void Inventory::print() const
{
    std::cout << *this;
}

// ===========================================================================
// Iterator
// ===========================================================================

Inventory::iterator Inventory::begin() noexcept
{
    return m_items.begin();
}

Inventory::iterator Inventory::end() noexcept
{
    return m_items.end();
}

Inventory::const_iterator Inventory::begin() const noexcept
{
    return m_items.begin();
}

Inventory::const_iterator Inventory::end() const noexcept
{
    return m_items.end();
}

Inventory::const_iterator Inventory::cbegin() const noexcept
{
    return m_items.cbegin();
}

Inventory::const_iterator Inventory::cend() const noexcept
{
    return m_items.cend();
}

// ===========================================================================
// Free function — stream insertion operator
// ===========================================================================

std::ostream& operator<<(std::ostream& os, const Inventory& inv)
{
    for (const Item& item : inv)
    {
        os << "  " << item.getName() << ": " << item.getQuantity();

        const bool hasMetadata =
            !item.getCategory().empty()   ||
            !item.getLotNumber().empty()  ||
            !item.getExpiryDate().empty() ||
            item.getUnitPrice() != 0.0;

        if (hasMetadata)
        {
            os << " [";
            bool first = true;
            auto sep = [&]() -> std::ostream& {
                if (!first) { os << " | "; }
                first = false;
                return os;
            };
            if (!item.getCategory().empty())   { sep() << item.getCategory(); }
            if (!item.getLotNumber().empty())   { sep() << "lot:" << item.getLotNumber(); }
            if (!item.getExpiryDate().empty())  { sep() << "exp:" << item.getExpiryDate(); }
            if (item.getUnitPrice() != 0.0)
            {
                const auto savedFlags = os.flags();
                const auto savedPrec  = os.precision();
                sep() << '$' << std::fixed << std::setprecision(2) << item.getUnitPrice();
                os.flags(savedFlags);
                os.precision(savedPrec);
            }
            os << ']';
        }
        os << '\n';
    }
    return os;
}

} // namespace eval
