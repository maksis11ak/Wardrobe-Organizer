# WardrobeOrganizer.java
/**
 * 👗 Wardrobe Organizer – Smart Clothing Manager (Java Edition)
 * Features: add items, categories, color stats, outfit builder, random outfit
 * Requires: Java 17+
 */

import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.util.*;
import java.util.regex.*;

public class WardrobeOrganizer {
    // ─── Colors ────────────────────────────────────────────────────────────

    private static final String RESET = "\u001B[0m";
    private static final String BRIGHT = "\u001B[1m";
    private static final String DIM = "\u001B[2m";
    private static final String RED = "\u001B[31m";
    private static final String GREEN = "\u001B[32m";
    private static final String YELLOW = "\u001B[33m";
    private static final String BLUE = "\u001B[34m";
    private static final String MAGENTA = "\u001B[35m";
    private static final String CYAN = "\u001B[36m";

    private static String c(String text, String color) { return color + text + RESET; }

    // ─── Data Model ──────────────────────────────────────────────────────

    static class Item {
        String id, name, category, color, season, photo, created;
        Item(String id, String name, String category, String color, String season, String photo, String created) {
            this.id = id; this.name = name; this.category = category;
            this.color = color; this.season = season; this.photo = photo; this.created = created;
        }
    }

    static class Outfit {
        String id, name, created;
        List<String> items;
        Outfit(String id, String name, List<String> items, String created) {
            this.id = id; this.name = name; this.items = items; this.created = created;
        }
    }

    // ─── Constants ──────────────────────────────────────────────────────────

    private static final List<String> CATEGORIES = Arrays.asList("Tops", "Bottoms", "Shoes", "Accessories", "Outerwear", "Dresses");
    private static final List<String> COLORS = Arrays.asList("Black", "White", "Red", "Blue", "Green", "Yellow", "Purple", "Pink",
            "Orange", "Brown", "Grey", "Navy", "Beige", "Olive", "Burgundy");
    private static final List<String> SEASONS = Arrays.asList("Spring", "Summer", "Autumn", "Winter", "All");

    // ─── Config ────────────────────────────────────────────────────────────

    private static final String DATA_DIR = System.getProperty("user.home") + "/.wardrobe";
    private static final String DATA_FILE = DATA_DIR + "/data.json";

    // ─── Wardrobe Data ──────────────────────────────────────────────────

    private static class WardrobeData {
        private List<Item> items = new ArrayList<>();
        private List<Outfit> outfits = new ArrayList<>();
        private final Random random = new Random();

        WardrobeData() throws IOException {
            Files.createDirectories(Paths.get(DATA_DIR));
            load();
        }

        private void load() {
            Path path = Paths.get(DATA_FILE);
            if (!Files.exists(path)) return;
            try {
                String json = Files.readString(path);
                // Simple parse (demo only)
                // Parse items
                Pattern p = Pattern.compile("\"items\"\\s*:\\s*\\[([^\\]]*)\\]");
                Matcher m = p.matcher(json);
                if (m.find()) {
                    String itemsStr = m.group(1);
                    parseItemsFromJson(itemsStr);
                }
            } catch (Exception e) { /* ignore */ }
        }

        private void parseItemsFromJson(String itemsStr) {
            int brace = itemsStr.indexOf("{");
            while (brace != -1) {
                int braceEnd = itemsStr.indexOf("}", brace);
                if (braceEnd == -1) break;
                String obj = itemsStr.substring(brace, braceEnd + 1);
                String id = extractJsonString(obj, "id");
                String name = extractJsonString(obj, "name");
                String category = extractJsonString(obj, "category");
                String color = extractJsonString(obj, "color");
                String season = extractJsonString(obj, "season");
                String photo = extractJsonString(obj, "photo");
                String created = extractJsonString(obj, "created");
                if (!id.isEmpty() && !name.isEmpty()) {
                    items.add(new Item(id, name, category, color, season, photo, created));
                }
                brace = itemsStr.indexOf("{", braceEnd);
            }
        }

        private String extractJsonString(String obj, String key) {
            Pattern p = Pattern.compile("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
            Matcher m = p.matcher(obj);
            return m.find() ? m.group(1) : "";
        }

        private void save() {
            try {
                StringBuilder sb = new StringBuilder();
                sb.append("{\n  \"items\": [\n");
                for (int i = 0; i < items.size(); i++) {
                    Item item = items.get(i);
                    sb.append("    {\n");
                    sb.append("      \"id\": \"").append(escapeJson(item.id)).append("\",\n");
                    sb.append("      \"name\": \"").append(escapeJson(item.name)).append("\",\n");
                    sb.append("      \"category\": \"").append(escapeJson(item.category)).append("\",\n");
                    sb.append("      \"color\": \"").append(escapeJson(item.color)).append("\",\n");
                    sb.append("      \"season\": \"").append(escapeJson(item.season)).append("\",\n");
                    sb.append("      \"photo\": \"").append(escapeJson(item.photo)).append("\",\n");
                    sb.append("      \"created\": \"").append(escapeJson(item.created)).append("\"\n");
                    sb.append("    }");
                    if (i < items.size() - 1) sb.append(",");
                    sb.append("\n");
                }
                sb.append("  ],\n  \"outfits\": [\n");
                for (int i = 0; i < outfits.size(); i++) {
                    Outfit outfit = outfits.get(i);
                    sb.append("    {\n");
                    sb.append("      \"id\": \"").append(escapeJson(outfit.id)).append("\",\n");
                    sb.append("      \"name\": \"").append(escapeJson(outfit.name)).append("\",\n");
                    sb.append("      \"items\": [");
                    for (int j = 0; j < outfit.items.size(); j++) {
                        sb.append("\"").append(escapeJson(outfit.items.get(j))).append("\"");
                        if (j < outfit.items.size() - 1) sb.append(",");
                    }
                    sb.append("],\n");
                    sb.append("      \"created\": \"").append(escapeJson(outfit.created)).append("\"\n");
                    sb.append("    }");
                    if (i < outfits.size() - 1) sb.append(",");
                    sb.append("\n");
                }
                sb.append("  ]\n}");
                Files.writeString(Paths.get(DATA_FILE), sb.toString());
            } catch (IOException e) { e.printStackTrace(); }
        }

        private String escapeJson(String s) {
            return s.replace("\\", "\\\\").replace("\"", "\\\"");
        }

        private String generateId() {
            return UUID.randomUUID().toString();
        }

        Item addItem(String name, String category, String color, String season, String photo) {
            Item item = new Item(generateId(), name, category, color, season, photo, Instant.now().toString());
            items.add(item);
            save();
            return item;
        }

        boolean deleteItem(String id) {
            Iterator<Item> it = items.iterator();
            while (it.hasNext()) {
                Item item = it.next();
                if (item.id.equals(id)) {
                    // Remove from outfits
                    for (Outfit o : outfits) {
                        o.items.remove(id);
                    }
                    it.remove();
                    save();
                    return true;
                }
            }
            return false;
        }

        Item getItem(String id) {
            return items.stream().filter(i -> i.id.equals(id)).findFirst().orElse(null);
        }

        Outfit addOutfit(String name, List<String> itemIds) {
            Outfit outfit = new Outfit(generateId(), name, new ArrayList<>(itemIds), Instant.now().toString());
            outfits.add(outfit);
            save();
            return outfit;
        }

        List<Item> searchItems(String query, String category, String color, String season) {
            return items.stream()
                .filter(i -> {
                    boolean match = true;
                    if (!query.isEmpty()) {
                        String q = query.toLowerCase();
                        match = match && (i.name.toLowerCase().contains(q) || i.category.toLowerCase().contains(q));
                    }
                    if (!category.isEmpty()) match = match && i.category.equalsIgnoreCase(category);
                    if (!color.isEmpty()) match = match && i.color.equalsIgnoreCase(color);
                    if (!season.isEmpty()) match = match && (i.season.equalsIgnoreCase(season) || i.season.equalsIgnoreCase("all"));
                    return match;
                })
                .collect(ArrayList::new, ArrayList::add, ArrayList::addAll);
        }

        Map<String, Integer> getColorStats() {
            Map<String, Integer> stats = new LinkedHashMap<>();
            items.forEach(i -> stats.put(i.color, stats.getOrDefault(i.color, 0) + 1));
            return stats;
        }

        Map<String, Integer> getCategoryStats() {
            Map<String, Integer> stats = new LinkedHashMap<>();
            items.forEach(i -> stats.put(i.category, stats.getOrDefault(i.category, 0) + 1));
            return stats;
        }

        Map<String, Integer> getSeasonStats() {
            Map<String, Integer> stats = new LinkedHashMap<>();
            items.forEach(i -> stats.put(i.season, stats.getOrDefault(i.season, 0) + 1));
            return stats;
        }

        List<Item> randomOutfit(String season) {
            List<Item> filtered = new ArrayList<>(items);
            if (!season.isEmpty() && !season.equalsIgnoreCase("all")) {
                filtered.removeIf(i -> !i.season.equalsIgnoreCase(season) && !i.season.equalsIgnoreCase("all"));
            }
            if (filtered.isEmpty()) return new ArrayList<>();
            List<Item> outfit = new ArrayList<>();
            for (String cat : CATEGORIES) {
                List<Item> catItems = new ArrayList<>();
                for (Item i : filtered) {
                    if (i.category.equals(cat)) catItems.add(i);
                }
                if (!catItems.isEmpty()) {
                    outfit.add(catItems.get(random.nextInt(catItems.size())));
                }
            }
            return outfit;
        }
    }

    // ─── Main App ──────────────────────────────────────────────────────────

    private final Scanner scanner;
    private final WardrobeData data;

    public WardrobeOrganizer() throws IOException {
        scanner = new Scanner(System.in);
        data = new WardrobeData();
    }

    private String ask(String prompt) {
        System.out.print(prompt);
        return scanner.nextLine().trim();
    }

    private int askInt(String prompt) {
        while (true) {
            try {
                String ans = ask(prompt);
                if (ans.isEmpty()) return -1;
                return Integer.parseInt(ans);
            } catch (NumberFormatException e) {
                System.out.println(c("Please enter a number.", YELLOW));
            }
        }
    }

    private void showMenu() {
        int total = data.items.size();
        int categories = data.getCategoryStats().size();
        System.out.println("\n" + c("═".repeat(50), CYAN));
        System.out.println(c("👗 WARDROBE ORGANIZER", BRIGHT + CYAN));
        System.out.println(c("═".repeat(50), CYAN));
        System.out.println("  Items: " + total);
        System.out.println("  Categories: " + categories);
        System.out.println("  Outfits: " + data.outfits.size());
        System.out.println(c("═".repeat(50), CYAN));
        System.out.println("  1. 👕 Add clothing item");
        System.out.println("  2. 📋 List all items");
        System.out.println("  3. 🔍 Search items");
        System.out.println("  4. 📊 Statistics");
        System.out.println("  5. 🎭 Outfit Builder");
        System.out.println("  6. 🎲 Random Outfit");
        System.out.println("  7. ❤️  Favorites (coming soon)");
        System.out.println("  8. 🗑️  Delete item");
        System.out.println("  0. 🚪 Exit");
        System.out.println(c("═".repeat(50), CYAN));
    }

    private void listItems(List<Item> items) {
        if (items == null) items = data.items;
        if (items.isEmpty()) {
            System.out.println(c("No items found.", YELLOW));
            return;
        }
        System.out.println("\n👕 WARDROBE ITEMS");
        System.out.println(c("─".repeat(60), DIM));
        for (int i = 0; i < items.size(); i++) {
            Item item = items.get(i);
            System.out.printf("  %d. %s (%s) %s %s\n", i+1, item.name, item.category, item.color, item.season);
        }
    }

    private void addItem() {
        String name = ask("Item name: ");
        System.out.println("Categories: " + String.join(", ", CATEGORIES));
        String category = ask("Category: ");
        System.out.println("Colors: " + String.join(", ", COLORS));
        String color = ask("Color: ");
        System.out.println("Seasons: " + String.join(", ", SEASONS));
        String season = ask("Season: ");
        String photo = ask("Photo path (optional): ");
        Item item = data.addItem(name, category, color, season, photo);
        System.out.println(c("✅ Added " + item.name + " (" + item.category + ", " + item.color + ")", GREEN));
    }

    private void searchItems() {
        String query = ask("Search term (name/category): ");
        String category = ask("Filter by category (optional): ");
        String color = ask("Filter by color (optional): ");
        String season = ask("Filter by season (optional): ");
        List<Item> results = data.searchItems(query, category, color, season);
        if (results.isEmpty()) {
            System.out.println(c("No items match your search.", YELLOW));
        } else {
            listItems(results);
        }
    }

    private void showStats() {
        Map<String, Integer> colorStats = data.getColorStats();
        Map<String, Integer> categoryStats = data.getCategoryStats();
        Map<String, Integer> seasonStats = data.getSeasonStats();
        System.out.println("\n📊 STATISTICS");
        System.out.println(c("─".repeat(30), DIM));
        System.out.println("\n🎨 Colors:");
        for (Map.Entry<String, Integer> e : colorStats.entrySet()) {
            System.out.println("  " + e.getKey() + ": " + e.getValue());
        }
        System.out.println("\n📂 Categories:");
        for (Map.Entry<String, Integer> e : categoryStats.entrySet()) {
            System.out.println("  " + e.getKey() + ": " + e.getValue());
        }
        System.out.println("\n🌦️ Seasons:");
        for (Map.Entry<String, Integer> e : seasonStats.entrySet()) {
            System.out.println("  " + e.getKey() + ": " + e.getValue());
        }
    }

    private void outfitBuilder() {
        if (data.items.isEmpty()) {
            System.out.println(c("No items. Add some clothing first!", YELLOW));
            return;
        }
        String name = ask("Outfit name: ");
        System.out.println("Select items for your outfit:");
        listItems(null);
        List<String> itemIds = new ArrayList<>();
        while (true) {
            String choice = ask("Enter item number to add (or 'done'): ");
            if (choice.equalsIgnoreCase("done")) break;
            try {
                int idx = Integer.parseInt(choice) - 1;
                if (idx >= 0 && idx < data.items.size()) {
                    Item item = data.items.get(idx);
                    if (!itemIds.contains(item.id)) {
                        itemIds.add(item.id);
                        System.out.println(c("Added " + item.name, GREEN));
                    } else {
                        System.out.println(c("Already added", YELLOW));
                    }
                } else {
                    System.out.println(c("Invalid number", RED));
                }
            } catch (NumberFormatException e) {
                System.out.println(c("Invalid input", RED));
            }
        }
        if (!itemIds.isEmpty()) {
            Outfit outfit = data.addOutfit(name, itemIds);
            System.out.println(c("✅ Outfit '" + outfit.name + "' created with " + itemIds.size() + " items", GREEN));
            showOutfit(outfit);
        }
    }

    private void showOutfit(Outfit outfit) {
        System.out.println("\n🎭 " + outfit.name);
        for (String id : outfit.items) {
            Item item = data.getItem(id);
            if (item != null) {
                System.out.println("  " + item.name + " (" + item.category + ") " + item.color);
            }
        }
    }

    private void randomOutfit() {
        if (data.items.isEmpty()) {
            System.out.println(c("No items. Add some clothing first!", YELLOW));
            return;
        }
        String season = ask("Season (optional, press Enter for all): ");
        List<Item> outfit = data.randomOutfit(season);
        if (outfit.isEmpty()) {
            System.out.println(c("No items for this season.", YELLOW));
            return;
        }
        System.out.println("\n🎲 Random Outfit");
        for (Item item : outfit) {
            System.out.println("  " + item.name + " (" + item.category + ") " + item.color);
        }
    }

    private void deleteItem() {
        if (data.items.isEmpty()) {
            System.out.println(c("No items to delete.", YELLOW));
            return;
        }
        listItems(null);
        String choice = ask("Enter item number to delete (or 'cancel'): ");
        if (choice.equalsIgnoreCase("cancel")) return;
        try {
            int idx = Integer.parseInt(choice) - 1;
            if (idx >= 0 && idx < data.items.size()) {
                Item item = data.items.get(idx);
                String confirm = ask("Delete '" + item.name + "'? (yes/no): ");
                if (confirm.equalsIgnoreCase("yes")) {
                    data.deleteItem(item.id);
                    System.out.println(c("🗑️  Deleted " + item.name, YELLOW));
                }
            } else {
                System.out.println(c("Invalid number", RED));
            }
        } catch (NumberFormatException e) {
            System.out.println(c("Invalid input", RED));
        }
    }

    public void run() {
        System.out.print("\033[H\033[2J");
        System.out.flush();
        System.out.println(c("\n👗 Wardrobe Organizer – Smart Clothing Manager", BRIGHT + CYAN));
        System.out.println(c("Catalog your wardrobe, plan outfits!", DIM));

        while (true) {
            showMenu();
            String choice = ask("Your choice: ");
            switch (choice) {
                case "1": addItem(); break;
                case "2": listItems(null); break;
                case "3": searchItems(); break;
                case "4": showStats(); break;
                case "5": outfitBuilder(); break;
                case "6": randomOutfit(); break;
                case "7": System.out.println(c("❤️ Favorites feature coming soon!", DIM)); break;
                case "8": deleteItem(); break;
                case "0":
                    System.out.println(c("👋 Stay stylish! Goodbye!", CYAN));
                    return;
                default:
                    System.out.println(c("❌ Invalid choice.", RED));
            }
            if (!choice.equals("0")) {
                System.out.print("\nPress Enter to continue...");
                scanner.nextLine();
            }
        }
    }

    public static void main(String[] args) {
        try {
            new WardrobeOrganizer().run();
        } catch (Exception e) {
            System.err.println(c("❌ Unexpected error: " + e.getMessage(), RED));
            e.printStackTrace();
            System.exit(1);
        }
    }
}
