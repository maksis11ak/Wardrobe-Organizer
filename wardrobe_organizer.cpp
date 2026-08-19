# wardrobe_organizer.cpp
/**
 * 👗 Wardrobe Organizer – Smart Clothing Manager (C++ Edition)
 * Features: add items, categories, color stats, outfit builder, random outfit
 * Uses only STL, no external libraries.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <filesystem>
#include <random>
#include <cctype>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#endif

// ─── Colors ──────────────────────────────────────────────────────────────────

#ifdef _WIN32
HANDLE hConsole;
void setColor(int color) { SetConsoleTextAttribute(hConsole, color); }
#define RESET_COLOR setColor(7)
#define COLOR_RED setColor(12)
#define COLOR_GREEN setColor(10)
#define COLOR_YELLOW setColor(14)
#define COLOR_BLUE setColor(9)
#define COLOR_MAGENTA setColor(13)
#define COLOR_CYAN setColor(11)
#define COLOR_BRIGHT setColor(15)
#define COLOR_DIM setColor(8)
#else
#define RESET_COLOR std::cout << "\x1b[0m"
#define COLOR_RED std::cout << "\x1b[31m"
#define COLOR_GREEN std::cout << "\x1b[32m"
#define COLOR_YELLOW std::cout << "\x1b[33m"
#define COLOR_BLUE std::cout << "\x1b[34m"
#define COLOR_MAGENTA std::cout << "\x1b[35m"
#define COLOR_CYAN std::cout << "\x1b[36m"
#define COLOR_BRIGHT std::cout << "\x1b[1m"
#define COLOR_DIM std::cout << "\x1b[2m"
#endif

#define C(str, color) color << str << RESET_COLOR

// ─── Helpers ─────────────────────────────────────────────────────────────────

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

std::string generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    ss << (dis(gen) % 4 + 8);
    for (int i = 0; i < 3; i++) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << dis(gen);
    return ss.str();
}

std::string get_timestamp() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

std::string get_home_dir() {
#ifdef _WIN32
    const char* h = std::getenv("USERPROFILE");
#else
    const char* h = std::getenv("HOME");
#endif
    return h ? std::string(h) : ".";
}

// ─── Data Model ─────────────────────────────────────────────────────────────

struct Item {
    std::string id;
    std::string name;
    std::string category;
    std::string color;
    std::string season;
    std::string photo;
    std::string created;
};

struct Outfit {
    std::string id;
    std::string name;
    std::vector<std::string> items;
    std::string created;
};

struct Data {
    std::vector<Item> items;
    std::vector<Outfit> outfits;
};

// ─── JSON (simplified) ─────────────────────────────────────────────────────

std::string escape_json(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else out += c;
    }
    return out;
}

std::string serialize_data(const Data& data) {
    std::ostringstream json;
    json << "{\n  \"items\": [\n";
    for (size_t i = 0; i < data.items.size(); ++i) {
        const auto& item = data.items[i];
        json << "    {\n";
        json << "      \"id\": \"" << escape_json(item.id) << "\",\n";
        json << "      \"name\": \"" << escape_json(item.name) << "\",\n";
        json << "      \"category\": \"" << escape_json(item.category) << "\",\n";
        json << "      \"color\": \"" << escape_json(item.color) << "\",\n";
        json << "      \"season\": \"" << escape_json(item.season) << "\",\n";
        json << "      \"photo\": \"" << escape_json(item.photo) << "\",\n";
        json << "      \"created\": \"" << escape_json(item.created) << "\"\n";
        json << "    }";
        if (i + 1 < data.items.size()) json << ",";
        json << "\n";
    }
    json << "  ],\n  \"outfits\": [\n";
    for (size_t i = 0; i < data.outfits.size(); ++i) {
        const auto& outfit = data.outfits[i];
        json << "    {\n";
        json << "      \"id\": \"" << escape_json(outfit.id) << "\",\n";
        json << "      \"name\": \"" << escape_json(outfit.name) << "\",\n";
        json << "      \"items\": [";
        for (size_t j = 0; j < outfit.items.size(); ++j) {
            json << "\"" << escape_json(outfit.items[j]) << "\"";
            if (j + 1 < outfit.items.size()) json << ",";
        }
        json << "],\n";
        json << "      \"created\": \"" << escape_json(outfit.created) << "\"\n";
        json << "    }";
        if (i + 1 < data.outfits.size()) json << ",";
        json << "\n";
    }
    json << "  ]\n}";
    return json.str();
}

bool deserialize_data(const std::string& json_str, Data& data) {
    // Very simple manual parse (demo only)
    data = Data{};
    // Parse items (simplified)
    size_t items_pos = json_str.find("\"items\":");
    if (items_pos != std::string::npos) {
        size_t arr_start = json_str.find("[", items_pos);
        size_t arr_end = json_str.rfind("]");
        if (arr_start != std::string::npos && arr_end != std::string::npos) {
            std::string items_str = json_str.substr(arr_start + 1, arr_end - arr_start - 1);
            size_t brace_pos = items_str.find("{");
            while (brace_pos != std::string::npos) {
                size_t brace_end = items_str.find("}", brace_pos);
                if (brace_end == std::string::npos) break;
                std::string obj = items_str.substr(brace_pos, brace_end - brace_pos + 1);
                Item item;
                auto extract = [&](const std::string& key) -> std::string {
                    size_t p = obj.find("\"" + key + "\":");
                    if (p == std::string::npos) return "";
                    p = obj.find(":", p) + 1;
                    while (p < obj.length() && (obj[p] == ' ' || obj[p] == '\n')) p++;
                    if (obj[p] == '"') {
                        p++;
                        size_t e = obj.find("\"", p);
                        if (e == std::string::npos) return "";
                        return obj.substr(p, e - p);
                    } else {
                        size_t e = obj.find_first_of(",}\n\r", p);
                        if (e == std::string::npos) return "";
                        return obj.substr(p, e - p);
                    }
                };
                item.id = extract("id");
                item.name = extract("name");
                item.category = extract("category");
                item.color = extract("color");
                item.season = extract("season");
                item.photo = extract("photo");
                item.created = extract("created");
                if (!item.id.empty() && !item.name.empty()) {
                    data.items.push_back(item);
                }
                brace_pos = items_str.find("{", brace_end);
            }
        }
    }
    // Parse outfits (simplified) - skip for brevity
    return true;
}

// ─── WardrobeData ──────────────────────────────────────────────────────────

class WardrobeData {
public:
    WardrobeData() {
        home = get_home_dir();
        data_dir = home + "/.wardrobe";
        std::filesystem::create_directories(data_dir);
        data_file = data_dir + "/data.json";
        load();
    }

    void load() {
        std::ifstream file(data_file);
        if (!file.is_open()) {
            data = Data{};
            return;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        deserialize_data(buffer.str(), data);
    }

    void save() {
        std::string json = serialize_data(data);
        std::string temp = data_file + ".tmp";
        std::ofstream out(temp);
        if (out.is_open()) {
            out << json;
            out.close();
            std::filesystem::rename(temp, data_file);
        }
    }

    Item addItem(const std::string& name, const std::string& category,
                 const std::string& color, const std::string& season,
                 const std::string& photo) {
        Item item;
        item.id = generate_uuid();
        item.name = name;
        item.category = category;
        item.color = color;
        item.season = season;
        item.photo = photo;
        item.created = get_timestamp();
        data.items.push_back(item);
        save();
        return item;
    }

    bool deleteItem(const std::string& id) {
        for (auto it = data.items.begin(); it != data.items.end(); ++it) {
            if (it->id == id) {
                // Remove from outfits
                for (auto& outfit : data.outfits) {
                    for (auto it2 = outfit.items.begin(); it2 != outfit.items.end(); ++it2) {
                        if (*it2 == id) {
                            outfit.items.erase(it2);
                            break;
                        }
                    }
                }
                data.items.erase(it);
                save();
                return true;
            }
        }
        return false;
    }

    Item* getItem(const std::string& id) {
        for (auto& item : data.items) {
            if (item.id == id) return &item;
        }
        return nullptr;
    }

    Outfit addOutfit(const std::string& name, const std::vector<std::string>& itemIds) {
        Outfit outfit;
        outfit.id = generate_uuid();
        outfit.name = name;
        outfit.items = itemIds;
        outfit.created = get_timestamp();
        data.outfits.push_back(outfit);
        save();
        return outfit;
    }

    std::vector<Item> searchItems(const std::string& query, const std::string& category,
                                  const std::string& color, const std::string& season) {
        std::vector<Item> results = data.items;
        if (!query.empty()) {
            std::string q = toLower(query);
            results.erase(std::remove_if(results.begin(), results.end(),
                [&](const Item& i) {
                    return !(toLower(i.name).find(q) != std::string::npos ||
                             toLower(i.category).find(q) != std::string::npos);
                }), results.end());
        }
        if (!category.empty()) {
            std::string c = toLower(category);
            results.erase(std::remove_if(results.begin(), results.end(),
                [&](const Item& i) { return toLower(i.category) != c; }), results.end());
        }
        if (!color.empty()) {
            std::string c = toLower(color);
            results.erase(std::remove_if(results.begin(), results.end(),
                [&](const Item& i) { return toLower(i.color) != c; }), results.end());
        }
        if (!season.empty()) {
            std::string s = toLower(season);
            results.erase(std::remove_if(results.begin(), results.end(),
                [&](const Item& i) { return toLower(i.season) != s && toLower(i.season) != "all"; }), results.end());
        }
        return results;
    }

    std::map<std::string, int> getColorStats() {
        std::map<std::string, int> stats;
        for (const auto& item : data.items) {
            stats[item.color]++;
        }
        return stats;
    }

    std::map<std::string, int> getCategoryStats() {
        std::map<std::string, int> stats;
        for (const auto& item : data.items) {
            stats[item.category]++;
        }
        return stats;
    }

    std::map<std::string, int> getSeasonStats() {
        std::map<std::string, int> stats;
        for (const auto& item : data.items) {
            stats[item.season]++;
        }
        return stats;
    }

    std::vector<Item> randomOutfit(const std::string& season) {
        std::vector<Item> items = data.items;
        if (!season.empty() && toLower(season) != "all") {
            std::string s = toLower(season);
            items.erase(std::remove_if(items.begin(), items.end(),
                [&](const Item& i) { return toLower(i.season) != s && toLower(i.season) != "all"; }), items.end());
        }
        if (items.empty()) return {};
        std::vector<Item> outfit;
        static std::random_device rd;
        static std::mt19937 gen(rd());
        const std::vector<std::string> categories = {"Tops", "Bottoms", "Shoes", "Accessories", "Outerwear", "Dresses"};
        for (const auto& cat : categories) {
            std::vector<Item> catItems;
            for (const auto& i : items) {
                if (i.category == cat) catItems.push_back(i);
            }
            if (!catItems.empty()) {
                std::uniform_int_distribution<> dist(0, catItems.size()-1);
                outfit.push_back(catItems[dist(gen)]);
            }
        }
        return outfit;
    }

    std::vector<Item>& getItems() { return data.items; }
    std::vector<Outfit>& getOutfits() { return data.outfits; }

private:
    std::string home, data_dir, data_file;
    Data data;
};

// ─── Main App ──────────────────────────────────────────────────────────────

class WardrobeApp {
public:
    WardrobeApp() : data() {}

    void run() {
        std::cout << "\033[2J\033[1;1H";
        std::cout << C("\n👗 Wardrobe Organizer – Smart Clothing Manager", COLOR_BRIGHT) << C("", COLOR_CYAN) << std::endl;
        std::cout << C("Catalog your wardrobe, plan outfits!", COLOR_DIM) << std::endl;

        while (true) {
            showMenu();
            std::string choice = ask("Your choice: ");
            if (choice == "1") addItem();
            else if (choice == "2") listItems();
            else if (choice == "3") searchItems();
            else if (choice == "4") showStats();
            else if (choice == "5") outfitBuilder();
            else if (choice == "6") randomOutfit();
            else if (choice == "7") std::cout << C("❤️ Favorites feature coming soon!", COLOR_DIM) << std::endl;
            else if (choice == "8") deleteItem();
            else if (choice == "0") {
                std::cout << C("👋 Stay stylish! Goodbye!", COLOR_CYAN) << std::endl;
                break;
            } else {
                std::cout << C("❌ Invalid choice.", COLOR_RED) << std::endl;
            }
            if (choice != "0") {
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
            }
        }
    }

private:
    WardrobeData data;

    std::string ask(const std::string& prompt) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        return trim(line);
    }

    void showMenu() {
        int total = data.getItems().size();
        int categories = data.getCategoryStats().size();
        std::cout << "\n" << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
        std::cout << C("👗 WARDROBE ORGANIZER", COLOR_BRIGHT) << C("", COLOR_CYAN) << std::endl;
        std::cout << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
        std::cout << "  Items: " << total << std::endl;
        std::cout << "  Categories: " << categories << std::endl;
        std::cout << "  Outfits: " << data.getOutfits().size() << std::endl;
        std::cout << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
        std::cout << "  1. 👕 Add clothing item" << std::endl;
        std::cout << "  2. 📋 List all items" << std::endl;
        std::cout << "  3. 🔍 Search items" << std::endl;
        std::cout << "  4. 📊 Statistics" << std::endl;
        std::cout << "  5. 🎭 Outfit Builder" << std::endl;
        std::cout << "  6. 🎲 Random Outfit" << std::endl;
        std::cout << "  7. ❤️  Favorites (coming soon)" << std::endl;
        std::cout << "  8. 🗑️  Delete item" << std::endl;
        std::cout << "  0. 🚪 Exit" << std::endl;
        std::cout << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
    }

    void listItems(const std::vector<Item>* items = nullptr) {
        const std::vector<Item>& target = items ? *items : data.getItems();
        if (target.empty()) {
            std::cout << C("No items found.", COLOR_YELLOW) << std::endl;
            return;
        }
        std::cout << "\n👕 WARDROBE ITEMS" << std::endl;
        std::cout << C(std::string(60, '─'), COLOR_DIM) << std::endl;
        for (size_t i = 0; i < target.size(); ++i) {
            const auto& item = target[i];
            std::cout << "  " << i+1 << ". " << item.name << " (" << item.category << ") " << item.color << " " << item.season << std::endl;
        }
    }

    void addItem() {
        std::string name = ask("Item name: ");
        std::cout << "Categories: Tops, Bottoms, Shoes, Accessories, Outerwear, Dresses" << std::endl;
        std::string category = ask("Category: ");
        std::cout << "Colors: Black, White, Red, Blue, Green, Yellow, Purple, Pink, Orange, Brown, Grey, Navy, Beige, Olive, Burgundy" << std::endl;
        std::string color = ask("Color: ");
        std::cout << "Seasons: Spring, Summer, Autumn, Winter, All" << std::endl;
        std::string season = ask("Season: ");
        std::string photo = ask("Photo path (optional): ");
        Item item = data.addItem(name, category, color, season, photo);
        std::cout << C("✅ Added " + item.name + " (" + item.category + ", " + item.color + ")", COLOR_GREEN) << std::endl;
    }

    void searchItems() {
        std::string query = ask("Search term (name/category): ");
        std::string category = ask("Filter by category (optional): ");
        std::string color = ask("Filter by color (optional): ");
        std::string season = ask("Filter by season (optional): ");
        auto results = data.searchItems(query, category, color, season);
        if (results.empty()) {
            std::cout << C("No items match your search.", COLOR_YELLOW) << std::endl;
        } else {
            listItems(&results);
        }
    }

    void showStats() {
        auto colorStats = data.getColorStats();
        auto categoryStats = data.getCategoryStats();
        auto seasonStats = data.getSeasonStats();
        std::cout << "\n📊 STATISTICS" << std::endl;
        std::cout << C(std::string(30, '─'), COLOR_DIM) << std::endl;
        std::cout << "\n🎨 Colors:" << std::endl;
        for (const auto& [color, count] : colorStats) {
            std::cout << "  " << color << ": " << count << std::endl;
        }
        std::cout << "\n📂 Categories:" << std::endl;
        for (const auto& [cat, count] : categoryStats) {
            std::cout << "  " << cat << ": " << count << std::endl;
        }
        std::cout << "\n🌦️ Seasons:" << std::endl;
        for (const auto& [season, count] : seasonStats) {
            std::cout << "  " << season << ": " << count << std::endl;
        }
    }

    void outfitBuilder() {
        if (data.getItems().empty()) {
            std::cout << C("No items. Add some clothing first!", COLOR_YELLOW) << std::endl;
            return;
        }
        std::string name = ask("Outfit name: ");
        std::cout << "Select items for your outfit:" << std::endl;
        listItems();
        std::vector<std::string> itemIds;
        while (true) {
            std::string choice = ask("Enter item number to add (or 'done'): ");
            if (toLower(choice) == "done") break;
            try {
                int idx = std::stoi(choice) - 1;
                if (idx >= 0 && idx < (int)data.getItems().size()) {
                    auto& item = data.getItems()[idx];
                    if (std::find(itemIds.begin(), itemIds.end(), item.id) == itemIds.end()) {
                        itemIds.push_back(item.id);
                        std::cout << C("Added " + item.name, COLOR_GREEN) << std::endl;
                    } else {
                        std::cout << C("Already added", COLOR_YELLOW) << std::endl;
                    }
                } else {
                    std::cout << C("Invalid number", COLOR_RED) << std::endl;
                }
            } catch (...) {
                std::cout << C("Invalid input", COLOR_RED) << std::endl;
            }
        }
        if (!itemIds.empty()) {
            Outfit outfit = data.addOutfit(name, itemIds);
            std::cout << C("✅ Outfit '" + outfit.name + "' created with " + std::to_string(itemIds.size()) + " items", COLOR_GREEN) << std::endl;
            showOutfit(outfit);
        }
    }

    void showOutfit(const Outfit& outfit) {
        std::cout << "\n🎭 " << outfit.name << std::endl;
        for (const auto& id : outfit.items) {
            Item* item = data.getItem(id);
            if (item) {
                std::cout << "  " << item->name << " (" << item->category << ") " << item->color << std::endl;
            }
        }
    }

    void randomOutfit() {
        if (data.getItems().empty()) {
            std::cout << C("No items. Add some clothing first!", COLOR_YELLOW) << std::endl;
            return;
        }
        std::string season = ask("Season (optional, press Enter for all): ");
        auto outfit = data.randomOutfit(season);
        if (outfit.empty()) {
            std::cout << C("No items for this season.", COLOR_YELLOW) << std::endl;
            return;
        }
        std::cout << "\n🎲 Random Outfit" << std::endl;
        for (const auto& item : outfit) {
            std::cout << "  " << item.name << " (" << item.category << ") " << item.color << std::endl;
        }
    }

    void deleteItem() {
        if (data.getItems().empty()) {
            std::cout << C("No items to delete.", COLOR_YELLOW) << std::endl;
            return;
        }
        listItems();
        std::string choice = ask("Enter item number to delete (or 'cancel'): ");
        if (toLower(choice) == "cancel") return;
        try {
            int idx = std::stoi(choice) - 1;
            if (idx >= 0 && idx < (int)data.getItems().size()) {
                auto& item = data.getItems()[idx];
                std::string confirm = ask("Delete '" + item.name + "'? (yes/no): ");
                if (toLower(confirm) == "yes") {
                    data.deleteItem(item.id);
                    std::cout << C("🗑️  Deleted " + item.name, COLOR_YELLOW) << std::endl;
                }
            } else {
                std::cout << C("Invalid number", COLOR_RED) << std::endl;
            }
        } catch (...) {
            std::cout << C("Invalid input", COLOR_RED) << std::endl;
        }
    }
};

int main() {
#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    try {
        WardrobeApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << C("❌ Unexpected error: ", COLOR_RED) << e.what() << std::endl;
        return 1;
    }
    return 0;
}
