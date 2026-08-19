# wardrobe_organizer.go
/**
 * 👗 Wardrobe Organizer – Smart Clothing Manager (Go Edition)
 * Features: add items, categories, color stats, outfit builder, random outfit
 */

package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"strings"
	"time"
	"github.com/google/uuid"
)

// ─── Types ──────────────────────────────────────────────────────────────────

type Item struct {
	ID       string `json:"id"`
	Name     string `json:"name"`
	Category string `json:"category"`
	Color    string `json:"color"`
	Season   string `json:"season"`
	Photo    string `json:"photo"`
	Created  string `json:"created"`
}

type Outfit struct {
	ID      string   `json:"id"`
	Name    string   `json:"name"`
	Items   []string `json:"items"`
	Created string   `json:"created"`
}

type Data struct {
	Items   []Item   `json:"items"`
	Outfits []Outfit `json:"outfits"`
}

// ─── Colors ──────────────────────────────────────────────────────────────────

const (
	reset  = "\x1b[0m"
	bright = "\x1b[1m"
	dim    = "\x1b[2m"
	red    = "\x1b[31m"
	green  = "\x1b[32m"
	yellow = "\x1b[33m"
	blue   = "\x1b[34m"
	magenta = "\x1b[35m"
	cyan   = "\x1b[36m"
)

func c(str, color string) string {
	return color + str + reset
}

// ─── Constants ──────────────────────────────────────────────────────────────

var CATEGORIES = []string{"Tops", "Bottoms", "Shoes", "Accessories", "Outerwear", "Dresses"}
var COLORS = []string{"Black", "White", "Red", "Blue", "Green", "Yellow", "Purple", "Pink",
	"Orange", "Brown", "Grey", "Navy", "Beige", "Olive", "Burgundy"}
var SEASONS = []string{"Spring", "Summer", "Autumn", "Winter", "All"}
var COLOR_EMOJIS = map[string]string{
	"Black": "⚫", "White": "⚪", "Red": "🔴", "Blue": "🔵", "Green": "🟢",
	"Yellow": "🟡", "Purple": "🟣", "Pink": "💗", "Orange": "🟠",
	"Brown": "🟤", "Grey": "◻️", "Navy": "💙", "Beige": "🟫", "Olive": "🫒",
}

// ─── Data Manager ──────────────────────────────────────────────────────────

type WardrobeData struct {
	filePath string
	Items    []Item
	Outfits  []Outfit
}

func NewWardrobeData() *WardrobeData {
	home, _ := os.UserHomeDir()
	dir := filepath.Join(home, ".wardrobe")
	os.MkdirAll(dir, 0755)
	filePath := filepath.Join(dir, "data.json")
	wd := &WardrobeData{filePath: filePath}
	wd.load()
	return wd
}

func (wd *WardrobeData) load() {
	if _, err := os.Stat(wd.filePath); os.IsNotExist(err) {
		return
	}
	raw, err := os.ReadFile(wd.filePath)
	if err != nil {
		return
	}
	var data Data
	if err := json.Unmarshal(raw, &data); err != nil {
		return
	}
	wd.Items = data.Items
	wd.Outfits = data.Outfits
}

func (wd *WardrobeData) save() {
	data := Data{Items: wd.Items, Outfits: wd.Outfits}
	raw, _ := json.MarshalIndent(data, "", "  ")
	os.WriteFile(wd.filePath, raw, 0644)
}

func (wd *WardrobeData) AddItem(name, category, color, season, photo string) Item {
	item := Item{
		ID:       uuid.New().String(),
		Name:     name,
		Category: category,
		Color:    color,
		Season:   season,
		Photo:    photo,
		Created:  time.Now().Format(time.RFC3339),
	}
	wd.Items = append(wd.Items, item)
	wd.save()
	return item
}

func (wd *WardrobeData) DeleteItem(id string) bool {
	for i, item := range wd.Items {
		if item.ID == id {
			// Remove from outfits
			for j := range wd.Outfits {
				for k, itemID := range wd.Outfits[j].Items {
					if itemID == id {
						wd.Outfits[j].Items = append(wd.Outfits[j].Items[:k], wd.Outfits[j].Items[k+1:]...)
						break
					}
				}
			}
			wd.Items = append(wd.Items[:i], wd.Items[i+1:]...)
			wd.save()
			return true
		}
	}
	return false
}

func (wd *WardrobeData) GetItem(id string) *Item {
	for _, item := range wd.Items {
		if item.ID == id {
			return &item
		}
	}
	return nil
}

func (wd *WardrobeData) AddOutfit(name string, itemIDs []string) Outfit {
	outfit := Outfit{
		ID:      uuid.New().String(),
		Name:    name,
		Items:   itemIDs,
		Created: time.Now().Format(time.RFC3339),
	}
	wd.Outfits = append(wd.Outfits, outfit)
	wd.save()
	return outfit
}

func (wd *WardrobeData) SearchItems(query, category, color, season string) []Item {
	results := wd.Items
	if query != "" {
		q := strings.ToLower(query)
		results = filterItems(results, func(i Item) bool {
			return strings.Contains(strings.ToLower(i.Name), q) ||
				strings.Contains(strings.ToLower(i.Category), q)
		})
	}
	if category != "" {
		c := strings.ToLower(category)
		results = filterItems(results, func(i Item) bool {
			return strings.ToLower(i.Category) == c
		})
	}
	if color != "" {
		c := strings.ToLower(color)
		results = filterItems(results, func(i Item) bool {
			return strings.ToLower(i.Color) == c
		})
	}
	if season != "" {
		s := strings.ToLower(season)
		results = filterItems(results, func(i Item) bool {
			return strings.ToLower(i.Season) == s || strings.ToLower(i.Season) == "all"
		})
	}
	return results
}

func filterItems(items []Item, fn func(Item) bool) []Item {
	var res []Item
	for _, i := range items {
		if fn(i) {
			res = append(res, i)
		}
	}
	return res
}

func (wd *WardrobeData) GetColorStats() map[string]int {
	stats := make(map[string]int)
	for _, i := range wd.Items {
		stats[i.Color]++
	}
	return stats
}

func (wd *WardrobeData) GetCategoryStats() map[string]int {
	stats := make(map[string]int)
	for _, i := range wd.Items {
		stats[i.Category]++
	}
	return stats
}

func (wd *WardrobeData) GetSeasonStats() map[string]int {
	stats := make(map[string]int)
	for _, i := range wd.Items {
		stats[i.Season]++
	}
	return stats
}

func (wd *WardrobeData) RandomOutfit(season string) []Item {
	items := wd.Items
	if season != "" && strings.ToLower(season) != "all" {
		s := strings.ToLower(season)
		items = filterItems(items, func(i Item) bool {
			return strings.ToLower(i.Season) == s || strings.ToLower(i.Season) == "all"
		})
	}
	if len(items) == 0 {
		return []Item{}
	}
	var outfit []Item
	for _, cat := range CATEGORIES {
		var catItems []Item
		for _, i := range items {
			if i.Category == cat {
				catItems = append(catItems, i)
			}
		}
		if len(catItems) > 0 {
			outfit = append(outfit, catItems[rand.Intn(len(catItems))])
		}
	}
	return outfit
}

// ─── Main App ──────────────────────────────────────────────────────────────

type WardrobeApp struct {
	reader *bufio.Reader
	data   *WardrobeData
}

func NewWardrobeApp() *WardrobeApp {
	rand.Seed(time.Now().UnixNano())
	return &WardrobeApp{
		reader: bufio.NewReader(os.Stdin),
		data:   NewWardrobeData(),
	}
}

func (app *WardrobeApp) ask(prompt string) string {
	fmt.Print(prompt)
	line, _ := app.reader.ReadString('\n')
	return strings.TrimSpace(line)
}

func (app *WardrobeApp) showMenu() {
	total := len(app.data.Items)
	categories := len(app.data.GetCategoryStats())
	fmt.Println("\n" + c(strings.Repeat("═", 50), cyan))
	fmt.Println(c("👗 WARDROBE ORGANIZER", bright+cyan))
	fmt.Println(c(strings.Repeat("═", 50), cyan))
	fmt.Printf("  Items: %d\n", total)
	fmt.Printf("  Categories: %d\n", categories)
	fmt.Printf("  Outfits: %d\n", len(app.data.Outfits))
	fmt.Println(c(strings.Repeat("═", 50), cyan))
	fmt.Println("  1. 👕 Add clothing item")
	fmt.Println("  2. 📋 List all items")
	fmt.Println("  3. 🔍 Search items")
	fmt.Println("  4. 📊 Statistics")
	fmt.Println("  5. 🎭 Outfit Builder")
	fmt.Println("  6. 🎲 Random Outfit")
	fmt.Println("  7. ❤️  Favorites (coming soon)")
	fmt.Println("  8. 🗑️  Delete item")
	fmt.Println("  0. 🚪 Exit")
	fmt.Println(c(strings.Repeat("═", 50), cyan))
}

func (app *WardrobeApp) listItems(items []Item) {
	if items == nil {
		items = app.data.Items
	}
	if len(items) == 0 {
		fmt.Println(c("No items found.", yellow))
		return
	}
	fmt.Println("\n👕 WARDROBE ITEMS")
	fmt.Println(c(strings.Repeat("─", 60), dim))
	for i, item := range items {
		emoji := COLOR_EMOJIS[item.Color]
		fmt.Printf("  %d. %s %s (%s) %s %s\n", i+1, emoji, item.Name, item.Category, item.Color, item.Season)
	}
}

func (app *WardrobeApp) addItem() {
	name := app.ask("Item name: ")
	fmt.Printf("Categories: %s\n", strings.Join(CATEGORIES, ", "))
	category := app.ask("Category: ")
	fmt.Printf("Colors: %s\n", strings.Join(COLORS, ", "))
	color := app.ask("Color: ")
	fmt.Printf("Seasons: %s\n", strings.Join(SEASONS, ", "))
	season := app.ask("Season: ")
	photo := app.ask("Photo path (optional): ")
	item := app.data.AddItem(name, category, color, season, photo)
	emoji := COLOR_EMOJIS[color]
	fmt.Printf("%s\n", c(fmt.Sprintf("✅ Added %s %s (%s, %s)", emoji, item.Name, item.Category, item.Color), green))
}

func (app *WardrobeApp) searchItems() {
	query := app.ask("Search term (name/category): ")
	category := app.ask("Filter by category (optional): ")
	color := app.ask("Filter by color (optional): ")
	season := app.ask("Filter by season (optional): ")
	results := app.data.SearchItems(query, category, color, season)
	if len(results) > 0 {
		app.listItems(results)
	} else {
		fmt.Println(c("No items match your search.", yellow))
	}
}

func (app *WardrobeApp) showStats() {
	colorStats := app.data.GetColorStats()
	categoryStats := app.data.GetCategoryStats()
	seasonStats := app.data.GetSeasonStats()
	fmt.Println("\n📊 STATISTICS")
	fmt.Println(c(strings.Repeat("─", 30), dim))
	fmt.Println("\n🎨 Colors:")
	for color, count := range colorStats {
		emoji := COLOR_EMOJIS[color]
		fmt.Printf("  %s %s: %d\n", emoji, color, count)
	}
	fmt.Println("\n📂 Categories:")
	for cat, count := range categoryStats {
		fmt.Printf("  %s: %d\n", cat, count)
	}
	fmt.Println("\n🌦️ Seasons:")
	for season, count := range seasonStats {
		fmt.Printf("  %s: %d\n", season, count)
	}
}

func (app *WardrobeApp) outfitBuilder() {
	if len(app.data.Items) == 0 {
		fmt.Println(c("No items. Add some clothing first!", yellow))
		return
	}
	name := app.ask("Outfit name: ")
	fmt.Println("Select items for your outfit:")
	app.listItems(nil)
	var itemIDs []string
	for {
		choice := app.ask("Enter item number to add (or 'done'): ")
		if strings.ToLower(choice) == "done" {
			break
		}
		var idx int
		fmt.Sscanf(choice, "%d", &idx)
		idx--
		if idx >= 0 && idx < len(app.data.Items) {
			item := app.data.Items[idx]
			found := false
			for _, id := range itemIDs {
				if id == item.ID {
					found = true
					break
				}
			}
			if !found {
				itemIDs = append(itemIDs, item.ID)
				fmt.Printf("%s\n", c("Added "+item.Name, green))
			} else {
				fmt.Println(c("Already added", yellow))
			}
		} else {
			fmt.Println(c("Invalid number", red))
		}
	}
	if len(itemIDs) > 0 {
		outfit := app.data.AddOutfit(name, itemIDs)
		fmt.Printf("%s\n", c(fmt.Sprintf("✅ Outfit '%s' created with %d items", outfit.Name, len(itemIDs)), green))
		app.showOutfit(outfit)
	}
}

func (app *WardrobeApp) showOutfit(outfit Outfit) {
	fmt.Printf("\n🎭 %s\n", outfit.Name)
	for _, id := range outfit.Items {
		item := app.data.GetItem(id)
		if item != nil {
			emoji := COLOR_EMOJIS[item.Color]
			fmt.Printf("  %s %s (%s) %s\n", emoji, item.Name, item.Category, item.Color)
		}
	}
}

func (app *WardrobeApp) randomOutfit() {
	if len(app.data.Items) == 0 {
		fmt.Println(c("No items. Add some clothing first!", yellow))
		return
	}
	season := app.ask("Season (optional, press Enter for all): ")
	outfit := app.data.RandomOutfit(season)
	if len(outfit) == 0 {
		fmt.Println(c("No items for this season.", yellow))
		return
	}
	fmt.Println("\n🎲 Random Outfit")
	for _, item := range outfit {
		emoji := COLOR_EMOJIS[item.Color]
		fmt.Printf("  %s %s (%s) %s\n", emoji, item.Name, item.Category, item.Color)
	}
}

func (app *WardrobeApp) deleteItem() {
	if len(app.data.Items) == 0 {
		fmt.Println(c("No items to delete.", yellow))
		return
	}
	app.listItems(nil)
	choice := app.ask("Enter item number to delete (or 'cancel'): ")
	if strings.ToLower(choice) == "cancel" {
		return
	}
	var idx int
	fmt.Sscanf(choice, "%d", &idx)
	idx--
	if idx >= 0 && idx < len(app.data.Items) {
		item := app.data.Items[idx]
		confirm := app.ask(fmt.Sprintf("Delete '%s'? (yes/no): ", item.Name))
		if strings.ToLower(confirm) == "yes" {
			app.data.DeleteItem(item.ID)
			fmt.Printf("%s\n", c("🗑️  Deleted "+item.Name, yellow))
		}
	} else {
		fmt.Println(c("Invalid number", red))
	}
}

func (app *WardrobeApp) run() {
	fmt.Print("\033[H\033[2J")
	fmt.Printf("%s\n", c("\n👗 Wardrobe Organizer – Smart Clothing Manager", bright+cyan))
	fmt.Printf("%s\n", c("Catalog your wardrobe, plan outfits!", dim))

	for {
		app.showMenu()
		choice := app.ask("Your choice: ")
		switch choice {
		case "1":
			app.addItem()
		case "2":
			app.listItems(nil)
		case "3":
			app.searchItems()
		case "4":
			app.showStats()
		case "5":
			app.outfitBuilder()
		case "6":
			app.randomOutfit()
		case "7":
			fmt.Println(c("❤️ Favorites feature coming soon!", dim))
		case "8":
			app.deleteItem()
		case "0":
			fmt.Printf("%s\n", c("👋 Stay stylish! Goodbye!", cyan))
			return
		default:
			fmt.Println(c("❌ Invalid choice.", red))
		}
		if choice != "0" {
			fmt.Print("\nPress Enter to continue...")
			app.reader.ReadString('\n')
		}
	}
}

func main() {
	app := NewWardrobeApp()
	app.run()
}
