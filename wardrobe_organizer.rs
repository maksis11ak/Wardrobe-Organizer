# wardrobe_organizer.rs
/**
 * 👗 Wardrobe Organizer – Smart Clothing Manager (Rust Edition)
 * Features: add items, categories, color stats, outfit builder, random outfit
 * Dependencies: serde, serde_json, chrono, colored, rand, uuid
 */

use chrono::Utc;
use colored::*;
use rand::seq::SliceRandom;
use rand::thread_rng;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fs;
use std::io::{self, Write, BufRead};
use std::path::PathBuf;
use uuid::Uuid;

// ─── Types ──────────────────────────────────────────────────────────────────

#[derive(Debug, Serialize, Deserialize, Clone)]
struct Item {
    id: String,
    name: String,
    category: String,
    color: String,
    season: String,
    photo: String,
    created: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
struct Outfit {
    id: String,
    name: String,
    items: Vec<String>,
    created: String,
}

#[derive(Debug, Serialize, Deserialize)]
struct Data {
    items: Vec<Item>,
    outfits: Vec<Outfit>,
}

// ─── Colors ──────────────────────────────────────────────────────────────────

fn c(text: &str, color: &str) -> String {
    match color {
        "green" => text.green().to_string(),
        "red" => text.red().to_string(),
        "yellow" => text.yellow().to_string(),
        "cyan" => text.cyan().to_string(),
        "bright" => text.bright().to_string(),
        "dim" => text.dimmed().to_string(),
        _ => text.to_string(),
    }
}

// ─── Constants ──────────────────────────────────────────────────────────────

const CATEGORIES: &[&str] = &["Tops", "Bottoms", "Shoes", "Accessories", "Outerwear", "Dresses"];
const COLORS: &[&str] = &["Black", "White", "Red", "Blue", "Green", "Yellow", "Purple", "Pink",
                         "Orange", "Brown", "Grey", "Navy", "Beige", "Olive", "Burgundy"];
const SEASONS: &[&str] = &["Spring", "Summer", "Autumn", "Winter", "All"];

lazy_static! {
    static ref COLOR_EMOJIS: HashMap<String, String> = {
        let mut m = HashMap::new();
        m.insert("Black".to_string(), "⚫".to_string());
        m.insert("White".to_string(), "⚪".to_string());
        m.insert("Red".to_string(), "🔴".to_string());
        m.insert("Blue".to_string(), "🔵".to_string());
        m.insert("Green".to_string(), "🟢".to_string());
        m.insert("Yellow".to_string(), "🟡".to_string());
        m.insert("Purple".to_string(), "🟣".to_string());
        m.insert("Pink".to_string(), "💗".to_string());
        m.insert("Orange".to_string(), "🟠".to_string());
        m.insert("Brown".to_string(), "🟤".to_string());
        m.insert("Grey".to_string(), "◻️".to_string());
        m.insert("Navy".to_string(), "💙".to_string());
        m.insert("Beige".to_string(), "🟫".to_string());
        m.insert("Olive".to_string(), "🫒".to_string());
        m
    };
}

// ─── Data Manager ──────────────────────────────────────────────────────────

struct WardrobeData {
    file_path: PathBuf,
    items: Vec<Item>,
    outfits: Vec<Outfit>,
}

impl WardrobeData {
    fn new() -> Self {
        let home = std::env::var("HOME").or_else(|_| std::env::var("USERPROFILE")).unwrap_or_else(|_| ".".to_string());
        let dir = PathBuf::from(home).join(".wardrobe");
        fs::create_dir_all(&dir).unwrap();
        let file_path = dir.join("data.json");
        let mut wd = WardrobeData { file_path, items: Vec::new(), outfits: Vec::new() };
        wd.load();
        wd
    }

    fn load(&mut self) {
        if let Ok(raw) = fs::read_to_string(&self.file_path) {
            if let Ok(data) = serde_json::from_str::<Data>(&raw) {
                self.items = data.items;
                self.outfits = data.outfits;
                return;
            }
        }
    }

    fn save(&self) {
        let data = Data { items: self.items.clone(), outfits: self.outfits.clone() };
        let raw = serde_json::to_string_pretty(&data).unwrap();
        let _ = fs::write(&self.file_path, raw);
    }

    fn add_item(&mut self, name: String, category: String, color: String, season: String, photo: String) -> Item {
        let item = Item {
            id: Uuid::new_v4().to_string(),
            name,
            category,
            color,
            season,
            photo,
            created: Utc::now().to_rfc3339(),
        };
        self.items.push(item.clone());
        self.save();
        item
    }

    fn delete_item(&mut self, id: &str) -> bool {
        let idx = self.items.iter().position(|i| i.id == id);
        if let Some(i) = idx {
            // Remove from outfits
            for outfit in &mut self.outfits {
                outfit.items.retain(|item_id| item_id != id);
            }
            self.items.remove(i);
            self.save();
            true
        } else {
            false
        }
    }

    fn get_item(&self, id: &str) -> Option<&Item> {
        self.items.iter().find(|i| i.id == id)
    }

    fn add_outfit(&mut self, name: String, item_ids: Vec<String>) -> Outfit {
        let outfit = Outfit {
            id: Uuid::new_v4().to_string(),
            name,
            items: item_ids,
            created: Utc::now().to_rfc3339(),
        };
        self.outfits.push(outfit.clone());
        self.save();
        outfit
    }

    fn search_items(&self, query: &str, category: &str, color: &str, season: &str) -> Vec<Item> {
        let mut results = self.items.clone();
        if !query.is_empty() {
            let q = query.to_lowercase();
            results.retain(|i| i.name.to_lowercase().contains(&q) || i.category.to_lowercase().contains(&q));
        }
        if !category.is_empty() {
            let c = category.to_lowercase();
            results.retain(|i| i.category.to_lowercase() == c);
        }
        if !color.is_empty() {
            let c = color.to_lowercase();
            results.retain(|i| i.color.to_lowercase() == c);
        }
        if !season.is_empty() {
            let s = season.to_lowercase();
            results.retain(|i| i.season.to_lowercase() == s || i.season.to_lowercase() == "all");
        }
        results
    }

    fn get_color_stats(&self) -> HashMap<String, usize> {
        let mut stats = HashMap::new();
        for item in &self.items {
            *stats.entry(item.color.clone()).or_insert(0) += 1;
        }
        stats
    }

    fn get_category_stats(&self) -> HashMap<String, usize> {
        let mut stats = HashMap::new();
        for item in &self.items {
            *stats.entry(item.category.clone()).or_insert(0) += 1;
        }
        stats
    }

    fn get_season_stats(&self) -> HashMap<String, usize> {
        let mut stats = HashMap::new();
        for item in &self.items {
            *stats.entry(item.season.clone()).or_insert(0) += 1;
        }
        stats
    }

    fn random_outfit(&self, season: &str) -> Vec<Item> {
        let mut items = self.items.clone();
        if !season.is_empty() && season.to_lowercase() != "all" {
            let s = season.to_lowercase();
            items.retain(|i| i.season.to_lowercase() == s || i.season.to_lowercase() == "all");
        }
        if items.is_empty() {
            return Vec::new();
        }
        let mut outfit = Vec::new();
        let mut rng = thread_rng();
        for cat in CATEGORIES {
            let cat_items: Vec<Item> = items.iter().filter(|i| i.category == *cat).cloned().collect();
            if !cat_items.is_empty() {
                if let Some(item) = cat_items.choose(&mut rng) {
                    outfit.push(item.clone());
                }
            }
        }
        outfit
    }
}

// ─── Main App ──────────────────────────────────────────────────────────────

struct WardrobeApp {
    data: WardrobeData,
}

impl WardrobeApp {
    fn new() -> Self {
        WardrobeApp { data: WardrobeData::new() }
    }

    fn ask(&self, prompt: &str) -> String {
        print!("{}", prompt);
        io::stdout().flush().unwrap();
        let mut line = String::new();
        io::stdin().read_line(&mut line).unwrap();
        line.trim().to_string()
    }

    fn show_menu(&self) {
        let total = self.data.items.len();
        let categories = self.data.get_category_stats().len();
        println!("\n{}", "═".repeat(50).cyan());
        println!("{}", c("👗 WARDROBE ORGANIZER", "bright cyan"));
        println!("{}", "═".repeat(50).cyan());
        println!("  Items: {}", total);
        println!("  Categories: {}", categories);
        println!("  Outfits: {}", self.data.outfits.len());
        println!("{}", "═".repeat(50).cyan());
        println!("  1. 👕 Add clothing item");
        println!("  2. 📋 List all items");
        println!("  3. 🔍 Search items");
        println!("  4. 📊 Statistics");
        println!("  5. 🎭 Outfit Builder");
        println!("  6. 🎲 Random Outfit");
        println!("  7. ❤️  Favorites (coming soon)");
        println!("  8. 🗑️  Delete item");
        println!("  0. 🚪 Exit");
        println!("{}", "═".repeat(50).cyan());
    }

    fn list_items(&self, items: Option<&[Item]>) {
        let target = match items {
            Some(i) => i,
            None => &self.data.items,
        };
        if target.is_empty() {
            println!("{}", c("No items found.", "yellow"));
            return;
        }
        println!("\n👕 WARDROBE ITEMS");
        println!("{}", "─".repeat(60).dimmed());
        for (i, item) in target.iter().enumerate() {
            let emoji = COLOR_EMOJIS.get(&item.color).unwrap_or(&"".to_string());
            println!("  {}. {} {} ({}) {} {}", i+1, emoji, item.name, item.category, item.color, item.season);
        }
    }

    fn add_item(&mut self) {
        let name = self.ask("Item name: ");
        println!("Categories: {}", CATEGORIES.join(", "));
        let category = self.ask("Category: ");
        println!("Colors: {}", COLORS.join(", "));
        let color = self.ask("Color: ");
        println!("Seasons: {}", SEASONS.join(", "));
        let season = self.ask("Season: ");
        let photo = self.ask("Photo path (optional): ");
        let item = self.data.add_item(name, category, color, season, photo);
        let emoji = COLOR_EMOJIS.get(&item.color).unwrap_or(&"".to_string());
        println!("{}", c(&format!("✅ Added {} {} ({}, {})", emoji, item.name, item.category, item.color), "green"));
    }

    fn search_items(&self) {
        let query = self.ask("Search term (name/category): ");
        let category = self.ask("Filter by category (optional): ");
        let color = self.ask("Filter by color (optional): ");
        let season = self.ask("Filter by season (optional): ");
        let results = self.data.search_items(&query, &category, &color, &season);
        if results.is_empty() {
            println!("{}", c("No items match your search.", "yellow"));
        } else {
            self.list_items(Some(&results));
        }
    }

    fn show_stats(&self) {
        let color_stats = self.data.get_color_stats();
        let category_stats = self.data.get_category_stats();
        let season_stats = self.data.get_season_stats();
        println!("\n📊 STATISTICS");
        println!("{}", "─".repeat(30).dimmed());
        println!("\n🎨 Colors:");
        for (color, count) in color_stats {
            let emoji = COLOR_EMOJIS.get(&color).unwrap_or(&"".to_string());
            println!("  {} {}: {}", emoji, color, count);
        }
        println!("\n📂 Categories:");
        for (cat, count) in category_stats {
            println!("  {}: {}", cat, count);
        }
        println!("\n🌦️ Seasons:");
        for (season, count) in season_stats {
            println!("  {}: {}", season, count);
        }
    }

    fn outfit_builder(&mut self) {
        if self.data.items.is_empty() {
            println!("{}", c("No items. Add some clothing first!", "yellow"));
            return;
        }
        let name = self.ask("Outfit name: ");
        println!("Select items for your outfit:");
        self.list_items(None);
        let mut item_ids = Vec::new();
        loop {
            let choice = self.ask("Enter item number to add (or 'done'): ");
            if choice.to_lowercase() == "done" { break; }
            if let Ok(idx) = choice.parse::<usize>() {
                let idx = idx - 1;
                if idx < self.data.items.len() {
                    let item = &self.data.items[idx];
                    if !item_ids.contains(&item.id) {
                        item_ids.push(item.id.clone());
                        println!("{}", c(&format!("Added {}", item.name), "green"));
                    } else {
                        println!("{}", c("Already added", "yellow"));
                    }
                } else {
                    println!("{}", c("Invalid number", "red"));
                }
            } else {
                println!("{}", c("Invalid input", "red"));
            }
        }
        if !item_ids.is_empty() {
            let outfit = self.data.add_outfit(name, item_ids);
            println!("{}", c(&format!("✅ Outfit '{}' created with {} items", outfit.name, outfit.items.len()), "green"));
            self.show_outfit(&outfit);
        }
    }

    fn show_outfit(&self, outfit: &Outfit) {
        println!("\n🎭 {}", outfit.name);
        for id in &outfit.items {
            if let Some(item) = self.data.get_item(id) {
                let emoji = COLOR_EMOJIS.get(&item.color).unwrap_or(&"".to_string());
                println!("  {} {} ({}) {}", emoji, item.name, item.category, item.color);
            }
        }
    }

    fn random_outfit(&self) {
        if self.data.items.is_empty() {
            println!("{}", c("No items. Add some clothing first!", "yellow"));
            return;
        }
        let season = self.ask("Season (optional, press Enter for all): ");
        let outfit = self.data.random_outfit(&season);
        if outfit.is_empty() {
            println!("{}", c("No items for this season.", "yellow"));
            return;
        }
        println!("\n🎲 Random Outfit");
        for item in outfit {
            let emoji = COLOR_EMOJIS.get(&item.color).unwrap_or(&"".to_string());
            println!("  {} {} ({}) {}", emoji, item.name, item.category, item.color);
        }
    }

    fn delete_item(&mut self) {
        if self.data.items.is_empty() {
            println!("{}", c("No items to delete.", "yellow"));
            return;
        }
        self.list_items(None);
        let choice = self.ask("Enter item number to delete (or 'cancel'): ");
        if choice.to_lowercase() == "cancel" { return; }
        if let Ok(idx) = choice.parse::<usize>() {
            let idx = idx - 1;
            if idx < self.data.items.len() {
                let item = &self.data.items[idx];
                let confirm = self.ask(&format!("Delete '{}'? (yes/no): ", item.name));
                if confirm.to_lowercase() == "yes" {
                    self.data.delete_item(&item.id);
                    println!("{}", c(&format!("🗑️  Deleted {}", item.name), "yellow"));
                }
            } else {
                println!("{}", c("Invalid number", "red"));
            }
        } else {
            println!("{}", c("Invalid input", "red"));
        }
    }

    fn run(&mut self) {
        println!("{}", "\n👗 Wardrobe Organizer – Smart Clothing Manager".bright().cyan());
        println!("{}", "Catalog your wardrobe, plan outfits!".dimmed());

        loop {
            self.show_menu();
            let choice = self.ask("Your choice: ");
            match choice.as_str() {
                "1" => self.add_item(),
                "2" => self.list_items(None),
                "3" => self.search_items(),
                "4" => self.show_stats(),
                "5" => self.outfit_builder(),
                "6" => self.random_outfit(),
                "7" => println!("{}", c("❤️ Favorites feature coming soon!", "dim")),
                "8" => self.delete_item(),
                "0" => {
                    println!("{}", c("👋 Stay stylish! Goodbye!", "cyan"));
                    return;
                }
                _ => println!("{}", c("❌ Invalid choice.", "red")),
            }
            if choice != "0" {
                print!("\nPress Enter to continue...");
                io::stdout().flush().unwrap();
                let mut _dummy = String::new();
                io::stdin().read_line(&mut _dummy).unwrap();
            }
        }
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────

#[macro_use] extern crate lazy_static;

fn main() {
    let mut app = WardrobeApp::new();
    app.run();
}
