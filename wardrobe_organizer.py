# wardrobe_organizer.py
#!/usr/bin/env python3
"""
👗 Wardrobe Organizer – Smart Clothing Manager (Python Edition)
Features: add items, categories, color analysis, outfit builder, seasonal suggestions
"""

import json
import os
import sys
import uuid
import random
from datetime import datetime
from pathlib import Path
from typing import List, Dict, Optional

try:
    from rich.console import Console
    from rich.table import Table
    from rich.panel import Panel
    from rich.prompt import Prompt, IntPrompt, Confirm
    from rich import box
    RICH_AVAILABLE = True
except ImportError:
    RICH_AVAILABLE = False
    print("⚠️  Install 'rich' for enhanced UI: pip install rich")


# ─── Colors ──────────────────────────────────────────────────────────────────

def c(text: str, color: str) -> str:
    colors = {
        "reset": "\033[0m", "bright": "\033[1m", "dim": "\033[2m",
        "red": "\033[31m", "green": "\033[32m", "yellow": "\033[33m",
        "blue": "\033[34m", "magenta": "\033[35m", "cyan": "\033[36m"
    }
    return f"{colors.get(color, '')}{text}{colors['reset']}"


# ─── Constants ──────────────────────────────────────────────────────────────

CATEGORIES = ["Tops", "Bottoms", "Shoes", "Accessories", "Outerwear", "Dresses"]
COLORS = ["Black", "White", "Red", "Blue", "Green", "Yellow", "Purple", "Pink",
          "Orange", "Brown", "Grey", "Navy", "Beige", "Olive", "Burgundy"]
SEASONS = ["Spring", "Summer", "Autumn", "Winter", "All"]
COLOR_EMOJIS = {
    "Black": "⚫", "White": "⚪", "Red": "🔴", "Blue": "🔵", "Green": "🟢",
    "Yellow": "🟡", "Purple": "🟣", "Pink": "💗", "Orange": "🟠",
    "Brown": "🟤", "Grey": "◻️", "Navy": "💙", "Beige": "🟫", "Olive": "🫒"
}

# ─── Data Manager ──────────────────────────────────────────────────────────

class WardrobeData:
    DATA_DIR = Path.home() / ".wardrobe"
    DATA_FILE = DATA_DIR / "data.json"

    def __init__(self):
        self.items: List[Dict] = []
        self.outfits: List[Dict] = []
        self._load()

    def _load(self):
        if self.DATA_FILE.exists():
            try:
                with open(self.DATA_FILE, 'r') as f:
                    data = json.load(f)
                    self.items = data.get("items", [])
                    self.outfits = data.get("outfits", [])
            except Exception:
                pass

    def save(self):
        self.DATA_DIR.mkdir(parents=True, exist_ok=True)
        with open(self.DATA_FILE, 'w') as f:
            json.dump({"items": self.items, "outfits": self.outfits}, f, indent=2)

    def add_item(self, name: str, category: str, color: str, season: str, photo: str = "") -> Dict:
        item = {
            "id": str(uuid.uuid4()),
            "name": name,
            "category": category,
            "color": color,
            "season": season,
            "photo": photo,
            "created": datetime.now().isoformat()
        }
        self.items.append(item)
        self.save()
        return item

    def delete_item(self, item_id: str) -> bool:
        for i, item in enumerate(self.items):
            if item["id"] == item_id:
                # Remove from outfits too
                for outfit in self.outfits:
                    if item_id in outfit.get("items", []):
                        outfit["items"].remove(item_id)
                del self.items[i]
                self.save()
                return True
        return False

    def get_item(self, item_id: str) -> Optional[Dict]:
        for item in self.items:
            if item["id"] == item_id:
                return item
        return None

    def add_outfit(self, name: str, item_ids: List[str]) -> Dict:
        outfit = {
            "id": str(uuid.uuid4()),
            "name": name,
            "items": item_ids,
            "created": datetime.now().isoformat()
        }
        self.outfits.append(outfit)
        self.save()
        return outfit

    def delete_outfit(self, outfit_id: str) -> bool:
        for i, outfit in enumerate(self.outfits):
            if outfit["id"] == outfit_id:
                del self.outfits[i]
                self.save()
                return True
        return False

    def search_items(self, query: str = "", category: str = "", color: str = "", season: str = "") -> List[Dict]:
        results = self.items
        if query:
            q = query.lower()
            results = [i for i in results if q in i["name"].lower() or q in i["category"].lower()]
        if category:
            results = [i for i in results if i["category"].lower() == category.lower()]
        if color:
            results = [i for i in results if i["color"].lower() == color.lower()]
        if season:
            results = [i for i in results if i["season"].lower() == season.lower() or i["season"].lower() == "all"]
        return results

    def get_color_stats(self) -> Dict[str, int]:
        stats = {}
        for item in self.items:
            stats[item["color"]] = stats.get(item["color"], 0) + 1
        return stats

    def get_category_stats(self) -> Dict[str, int]:
        stats = {}
        for item in self.items:
            stats[item["category"]] = stats.get(item["category"], 0) + 1
        return stats

    def get_season_stats(self) -> Dict[str, int]:
        stats = {}
        for item in self.items:
            stats[item["season"]] = stats.get(item["season"], 0) + 1
        return stats

    def random_outfit(self, season: str = "") -> List[Dict]:
        """Generate a random outfit with one item from each category."""
        if season and season.lower() != "all":
            items = [i for i in self.items if i["season"].lower() == season.lower() or i["season"].lower() == "all"]
        else:
            items = self.items
        if not items:
            return []
        outfit = []
        # Try to get one from each category
        for cat in CATEGORIES:
            cat_items = [i for i in items if i["category"] == cat]
            if cat_items:
                outfit.append(random.choice(cat_items))
        return outfit


# ─── Main App ──────────────────────────────────────────────────────────────

class WardrobeApp:
    def __init__(self):
        self.console = Console() if RICH_AVAILABLE else None
        self.data = WardrobeData()

    def show_menu(self):
        stats = self.data.get_category_stats()
        total = len(self.data.items)
        if self.console:
            panel = Panel(
                f"[bold cyan]👗 Wardrobe Organizer[/bold cyan]\n"
                f"  Items: {total}\n"
                f"  Categories: {len(stats)}\n"
                f"  Outfits: {len(self.data.outfits)}",
                title="📋 Main Menu",
                border_style="blue"
            )
            self.console.print(panel)
            self.console.print(" [1] 👕 Add clothing item")
            self.console.print(" [2] 📋 List all items")
            self.console.print(" [3] 🔍 Search items")
            self.console.print(" [4] 📊 Statistics")
            self.console.print(" [5] 🎭 Outfit Builder")
            self.console.print(" [6] 🎲 Random Outfit")
            self.console.print(" [7] ❤️  Favorites (not implemented)")
            self.console.print(" [8] 🗑️  Delete item")
            self.console.print(" [0] 🚪 Exit")
        else:
            print("\n" + "="*50)
            print(c("👗 WARDROBE ORGANIZER", "bright"))
            print("="*50)
            print(f"  Items: {total}")
            print(f"  Categories: {len(stats)}")
            print(f"  Outfits: {len(self.data.outfits)}")
            print("="*50)
            print("  1. 👕 Add clothing item")
            print("  2. 📋 List all items")
            print("  3. 🔍 Search items")
            print("  4. 📊 Statistics")
            print("  5. 🎭 Outfit Builder")
            print("  6. 🎲 Random Outfit")
            print("  7. ❤️  Favorites (not implemented)")
            print("  8. 🗑️  Delete item")
            print("  0. 🚪 Exit")
            print("="*50)

    def add_item(self):
        if self.console:
            name = Prompt.ask("Item name")
            category = Prompt.ask("Category", choices=CATEGORIES)
            color = Prompt.ask("Color", choices=COLORS)
            season = Prompt.ask("Season", choices=SEASONS)
            photo = Prompt.ask("Photo path (optional)", default="")
        else:
            name = input("Item name: ").strip()
            print(f"Categories: {', '.join(CATEGORIES)}")
            category = input("Category: ").strip()
            print(f"Colors: {', '.join(COLORS)}")
            color = input("Color: ").strip()
            print(f"Seasons: {', '.join(SEASONS)}")
            season = input("Season: ").strip()
            photo = input("Photo path (optional): ").strip()
        item = self.data.add_item(name, category, color, season, photo)
        emoji = COLOR_EMOJIS.get(color, "")
        print(c(f"✅ Added {emoji} {item['name']} ({item['category']}, {item['color']})", "green"))

    def list_items(self, items: List[Dict] = None):
        if items is None:
            items = self.data.items
        if not items:
            print(c("No items found.", "yellow"))
            return
        if self.console:
            table = Table(title="👕 Wardrobe Items", box=box.ROUNDED)
            table.add_column("#", style="dim")
            table.add_column("Name", style="green")
            table.add_column("Category", style="cyan")
            table.add_column("Color", style="magenta")
            table.add_column("Season", style="yellow")
            table.add_column("Photo", style="dim")
            for i, item in enumerate(items, 1):
                emoji = COLOR_EMOJIS.get(item["color"], "")
                table.add_row(str(i), f"{emoji} {item['name']}", item["category"],
                              item["color"], item["season"], item["photo"][:20] + "..." if item["photo"] else "")
            self.console.print(table)
        else:
            print("\n👕 WARDROBE ITEMS")
            print(c("─"*60, "dim"))
            for i, item in enumerate(items, 1):
                emoji = COLOR_EMOJIS.get(item["color"], "")
                print(f"  {i}. {emoji} {item['name']} ({item['category']}) {item['color']} {item['season']}")

    def search_items(self):
        if self.console:
            query = Prompt.ask("🔍 Search term (name/category)", default="")
            category = Prompt.ask("Filter by category (optional)", default="", choices=[""] + CATEGORIES)
            color = Prompt.ask("Filter by color (optional)", default="", choices=[""] + COLORS)
            season = Prompt.ask("Filter by season (optional)", default="", choices=[""] + SEASONS)
        else:
            query = input("Search term (name/category): ").strip()
            category = input("Filter by category (optional): ").strip()
            color = input("Filter by color (optional): ").strip()
            season = input("Filter by season (optional): ").strip()
        results = self.data.search_items(query, category, color, season)
        if results:
            self.list_items(results)
        else:
            print(c("No items match your search.", "yellow"))

    def show_stats(self):
        color_stats = self.data.get_color_stats()
        category_stats = self.data.get_category_stats()
        season_stats = self.data.get_season_stats()
        if self.console:
            # Color stats
            col_table = Table(title="🎨 Colors", box=box.MINIMAL)
            col_table.add_column("Color", style="magenta")
            col_table.add_column("Count", justify="right")
            for color, count in sorted(color_stats.items(), key=lambda x: -x[1]):
                emoji = COLOR_EMOJIS.get(color, "")
                col_table.add_row(f"{emoji} {color}", str(count))
            self.console.print(col_table)
            # Category stats
            cat_table = Table(title="📂 Categories", box=box.MINIMAL)
            cat_table.add_column("Category", style="cyan")
            cat_table.add_column("Count", justify="right")
            for cat, count in sorted(category_stats.items(), key=lambda x: -x[1]):
                cat_table.add_row(cat, str(count))
            self.console.print(cat_table)
            # Season stats
            season_table = Table(title="🌦️ Seasons", box=box.MINIMAL)
            season_table.add_column("Season", style="yellow")
            season_table.add_column("Count", justify="right")
            for season, count in sorted(season_stats.items(), key=lambda x: -x[1]):
                season_table.add_row(season, str(count))
            self.console.print(season_table)
        else:
            print("\n📊 STATISTICS")
            print(c("─"*30, "dim"))
            print("\n🎨 Colors:")
            for color, count in sorted(color_stats.items(), key=lambda x: -x[1]):
                emoji = COLOR_EMOJIS.get(color, "")
                print(f"  {emoji} {color}: {count}")
            print("\n📂 Categories:")
            for cat, count in sorted(category_stats.items(), key=lambda x: -x[1]):
                print(f"  {cat}: {count}")
            print("\n🌦️ Seasons:")
            for season, count in sorted(season_stats.items(), key=lambda x: -x[1]):
                print(f"  {season}: {count}")

    def outfit_builder(self):
        if not self.data.items:
            print(c("No items. Add some clothing first!", "yellow"))
            return
        if self.console:
            name = Prompt.ask("Outfit name")
            self.console.print("[bold]Select items for your outfit:[/bold]")
            self.list_items()
            item_ids = []
            while True:
                choice = Prompt.ask("Enter item number to add (or 'done')", default="done")
                if choice.lower() == "done":
                    break
                try:
                    idx = int(choice) - 1
                    if 0 <= idx < len(self.data.items):
                        item = self.data.items[idx]
                        if item["id"] not in item_ids:
                            item_ids.append(item["id"])
                            self.console.print(f"[green]Added {item['name']}[/green]")
                        else:
                            self.console.print("[yellow]Already added[/yellow]")
                    else:
                        self.console.print("[red]Invalid number[/red]")
                except ValueError:
                    self.console.print("[red]Invalid input[/red]")
        else:
            name = input("Outfit name: ").strip()
            print("Select items for your outfit:")
            self.list_items()
            item_ids = []
            while True:
                choice = input("Enter item number to add (or 'done'): ").strip()
                if choice.lower() == "done":
                    break
                try:
                    idx = int(choice) - 1
                    if 0 <= idx < len(self.data.items):
                        item = self.data.items[idx]
                        if item["id"] not in item_ids:
                            item_ids.append(item["id"])
                            print(c(f"Added {item['name']}", "green"))
                        else:
                            print(c("Already added", "yellow"))
                    else:
                        print(c("Invalid number", "red"))
                except ValueError:
                    print(c("Invalid input", "red"))
        if item_ids:
            outfit = self.data.add_outfit(name, item_ids)
            print(c(f"✅ Outfit '{outfit['name']}' created with {len(item_ids)} items", "green"))
            self.show_outfit(outfit)

    def show_outfit(self, outfit: Dict):
        items = [self.data.get_item(id) for id in outfit["items"] if self.data.get_item(id)]
        if self.console:
            table = Table(title=f"🎭 {outfit['name']}", box=box.ROUNDED)
            table.add_column("Name", style="green")
            table.add_column("Category", style="cyan")
            table.add_column("Color", style="magenta")
            for item in items:
                if item:
                    emoji = COLOR_EMOJIS.get(item["color"], "")
                    table.add_row(f"{emoji} {item['name']}", item["category"], item["color"])
            self.console.print(table)
        else:
            print(f"\n🎭 {outfit['name']}")
            for item in items:
                if item:
                    emoji = COLOR_EMOJIS.get(item["color"], "")
                    print(f"  {emoji} {item['name']} ({item['category']}) {item['color']}")

    def random_outfit(self):
        if not self.data.items:
            print(c("No items. Add some clothing first!", "yellow"))
            return
        if self.console:
            season = Prompt.ask("Season (optional)", choices=[""] + SEASONS, default="")
        else:
            season = input("Season (optional, press Enter for all): ").strip()
        outfit = self.data.random_outfit(season)
        if not outfit:
            print(c("No items for this season.", "yellow"))
            return
        if self.console:
            table = Table(title="🎲 Random Outfit", box=box.ROUNDED)
            table.add_column("Name", style="green")
            table.add_column("Category", style="cyan")
            table.add_column("Color", style="magenta")
            for item in outfit:
                emoji = COLOR_EMOJIS.get(item["color"], "")
                table.add_row(f"{emoji} {item['name']}", item["category"], item["color"])
            self.console.print(table)
        else:
            print("\n🎲 Random Outfit")
            for item in outfit:
                emoji = COLOR_EMOJIS.get(item["color"], "")
                print(f"  {emoji} {item['name']} ({item['category']}) {item['color']}")

    def delete_item(self):
        if not self.data.items:
            print(c("No items to delete.", "yellow"))
            return
        self.list_items()
        if self.console:
            choice = Prompt.ask("Enter item number to delete (or 'cancel')")
        else:
            choice = input("Enter item number to delete (or 'cancel'): ").strip()
        if choice.lower() == "cancel":
            return
        try:
            idx = int(choice) - 1
            if 0 <= idx < len(self.data.items):
                item = self.data.items[idx]
                if self.console and not Confirm.ask(f"Delete '{item['name']}'?"):
                    return
                elif not self.console and input(f"Delete '{item['name']}'? (yes/no): ").lower() != "yes":
                    return
                self.data.delete_item(item["id"])
                print(c(f"🗑️  Deleted {item['name']}", "yellow"))
            else:
                print(c("Invalid number", "red"))
        except ValueError:
            print(c("Invalid input", "red"))

    def run(self):
        if self.console:
            self.console.print(Panel.fit("[bold cyan]👗 Wardrobe Organizer – Smart Clothing Manager[/bold cyan]", border_style="cyan"))
        else:
            print(c("\n👗 Wardrobe Organizer – Smart Clothing Manager", "bright"))
            print(c("Catalog your wardrobe, plan outfits!", "dim"))

        while True:
            self.show_menu()
            if self.console:
                choice = Prompt.ask("Your choice", choices=["0","1","2","3","4","5","6","7","8"])
            else:
                choice = input("Your choice: ").strip()

            if choice == "1":
                self.add_item()
            elif choice == "2":
                self.list_items()
            elif choice == "3":
                self.search_items()
            elif choice == "4":
                self.show_stats()
            elif choice == "5":
                self.outfit_builder()
            elif choice == "6":
                self.random_outfit()
            elif choice == "7":
                print(c("❤️ Favorites feature coming soon!", "dim"))
            elif choice == "8":
                self.delete_item()
            elif choice == "0":
                print(c("👋 Stay stylish! Goodbye!", "cyan"))
                break
            else:
                print(c("❌ Invalid choice.", "red"))

            if choice != "0":
                if self.console:
                    self.console.print("\n[dim]Press Enter to continue...[/dim]")
                    input()
                else:
                    input("\nPress Enter to continue...")


def main():
    try:
        app = WardrobeApp()
        app.run()
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")
        sys.exit(0)
    except Exception as e:
        print(c(f"❌ Unexpected error: {e}", "red"))
        sys.exit(1)

if __name__ == "__main__":
    main()
