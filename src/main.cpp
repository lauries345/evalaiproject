#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

class Item {
public:
    Item(std::string name, int quantity)
        : name(std::move(name)), quantity(quantity) {}

    const std::string& getName() const { return name; }
    int getQuantity() const { return quantity; }
    void adjustQuantity(int delta) { quantity += delta; }

private:
    std::string name;
    int quantity;
};

class Inventory {
public:
    void add(const std::string& name, int quantity) {
        auto it = std::find_if(items.begin(), items.end(),
            [&name](const Item& i) { return i.getName() == name; });

        if (it != items.end()) {
            it->adjustQuantity(quantity);
        } else {
            items.emplace_back(name, quantity);
        }
    }

    bool remove(const std::string& name) {
        auto it = std::find_if(items.begin(), items.end(),
            [&name](const Item& i) { return i.getName() == name; });

        if (it == items.end()) return false;
        items.erase(it);
        return true;
    }

    void sortByName() {
        std::sort(items.begin(), items.end(),
            [](const Item& a, const Item& b) { return a.getName() < b.getName(); });
    }

    void print() const {
        for (const auto& item : items) {
            std::cout << "  " << item.getName() << ": " << item.getQuantity() << "\n";
        }
    }

    int totalQuantity() const {
        int sum = 0;
        for (const auto& item : items) sum += item.getQuantity();
        return sum;
    }

private:
    std::vector<Item> items;
};

int main() {
    Inventory inv;

    inv.add("Apples",  10);
    inv.add("Bananas",  5);
    inv.add("Oranges",  8);
    inv.add("Apples",   3);  // merges with existing entry

    std::cout << "Inventory:\n";
    inv.print();
    std::cout << "Total: " << inv.totalQuantity() << "\n\n";

    inv.sortByName();
    std::cout << "Sorted:\n";
    inv.print();

    inv.remove("Bananas");
    std::cout << "\nAfter removing Bananas:\n";
    inv.print();

    return 0;
}
