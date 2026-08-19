# wardrobe_organizer.ts
/**
 * 👗 Wardrobe Organizer – Smart Clothing Manager (TypeScript Edition)
 * Fully typed, advanced: add items, categories, color stats, outfit builder
 */

import * as fs from 'fs';
import * as path from 'path';
import * as os from 'os';
import * as readline from 'readline';
import { v4 as uuidv4 } from 'uuid';

// ─── Types ──────────────────────────────────────────────────────────────────

interface Item {
    id: string;
    name: string;
    category: string;
    color: string;
    season: string;
    photo: string;
    created: string;
}

interface Outfit {
    id: string;
    name: string;
    items: string[];
    created: string;
}

// ─── Colors ──────────────────────────────────────────────────────────────────

const colors = {
    reset: '\x1b[0m',
    bright: '\x1b[1m',
    dim: '\x1b[2m',
    red: '\x1b[31m',
    green: '\x1b[32m',
    yellow: '\x1b[33m',
    blue: '\x1b[34m',
    magenta: '\x1b[35m',
    cyan: '\x1b[36m',
};

const c = (str: string, color: string): string => `${color}${str}${colors.reset}`;

// ─── Constants ──────────────────────────────────────────────────────────────

const CATEGORIES = ['Tops', 'Bottoms', 'Shoes', 'Accessories', 'Outerwear', 'Dresses'];
const COLORS = ['Black', 'White', 'Red', 'Blue', 'Green', 'Yellow', 'Purple', 'Pink',
                'Orange', 'Brown', 'Grey', 'Navy', 'Beige', 'Olive', 'Burgundy'];
const SEASONS = ['Spring', 'Summer', 'Autumn', 'Winter', 'All'];
const COLOR_EMOJIS: Record<string, string> = {
    Black: '⚫', White: '⚪', Red: '🔴', Blue: '🔵', Green: '🟢',
    Yellow: '🟡', Purple: '🟣', Pink: '💗', Orange: '🟠',
    Brown: '🟤', Grey: '◻️', Navy: '💙', Beige: '🟫', Olive: '🫒'
};

// ─── Data Manager ──────────────────────────────────────────────────────────

class WardrobeData {
    private dataFile: string;
    public items: Item[] = [];
    public outfits: Outfit[] = [];

    constructor() {
        const dir = path.join(os.homedir(), '.wardrobe');
        if (!fs.existsSync(dir)) fs.mkdirSync(dir, { recursive: true });
        this.dataFile = path.join(dir, 'data.json');
        this._load();
    }

    private _load() {
        if (fs.existsSync(this.dataFile)) {
            try {
                const raw = fs.readFileSync(this.dataFile, 'utf8');
                const data = JSON.parse(raw);
                this.items = data.items || [];
                this.outfits = data.outfits || [];
            } catch (_) {}
        }
    }

    save() {
        fs.writeFileSync(this.dataFile, JSON.stringify({
            items: this.items,
            outfits: this.outfits
        }, null, 2));
    }

    addItem(name: string, category: string, color: string, season: string, photo: string = ''): Item {
        const item: Item = {
            id: uuidv4(),
            name,
            category,
            color,
            season,
            photo,
            created: new Date().toISOString()
        };
        this.items.push(item);
        this.save();
        return item;
    }

    deleteItem(id: string): boolean {
        const idx = this.items.findIndex(i => i.id === id);
        if (idx === -1) return false;
        this.outfits.forEach(o => {
            const i = o.items.indexOf(id);
            if (i !== -1) o.items.splice(i, 1);
        });
        this.items.splice(idx, 1);
        this.save();
        return true;
    }

    getItem(id: string): Item | null {
        return this.items.find(i => i.id === id) || null;
    }

    addOutfit(name: string, itemIds: string[]): Outfit {
        const outfit: Outfit = {
            id: uuidv4(),
            name,
            items: itemIds,
            created: new Date().toISOString()
        };
        this.outfits.push(outfit);
        this.save();
        return outfit;
    }

    searchItems(query: string = '', category: string = '', color: string = '', season: string = ''): Item[] {
        let results = this.items;
        if (query) {
            const q = query.toLowerCase();
            results = results.filter(i => i.name.toLowerCase().includes(q) || i.category.toLowerCase().includes(q));
        }
        if (category) results = results.filter(i => i.category.toLowerCase() === category.toLowerCase());
        if (color) results = results.filter(i => i.color.toLowerCase() === color.toLowerCase());
        if (season) results = results.filter(i => i.season.toLowerCase() === season.toLowerCase() || i.season.toLowerCase() === 'all');
        return results;
    }

    getColorStats(): Record<string, number> {
        const stats: Record<string, number> = {};
        this.items.forEach(i => {
            stats[i.color] = (stats[i.color] || 0) + 1;
        });
        return stats;
    }

    getCategoryStats(): Record<string, number> {
        const stats: Record<string, number> = {};
        this.items.forEach(i => {
            stats[i.category] = (stats[i.category] || 0) + 1;
        });
        return stats;
    }

    getSeasonStats(): Record<string, number> {
        const stats: Record<string, number> = {};
        this.items.forEach(i => {
            stats[i.season] = (stats[i.season] || 0) + 1;
        });
        return stats;
    }

    randomOutfit(season: string = ''): Item[] {
        let items = this.items;
        if (season && season.toLowerCase() !== 'all') {
            items = items.filter(i => i.season.toLowerCase() === season.toLowerCase() || i.season.toLowerCase() === 'all');
        }
        if (!items.length) return [];
        const outfit: Item[] = [];
        for (const cat of CATEGORIES) {
            const catItems = items.filter(i => i.category === cat);
            if (catItems.length) {
                outfit.push(catItems[Math.floor(Math.random() * catItems.length)]);
            }
        }
        return outfit;
    }
}

// ─── Main App ──────────────────────────────────────────────────────────────

class WardrobeApp {
    private rl: readline.Interface;
    private data: WardrobeData;

    constructor() {
        this.rl = readline.createInterface({ input: process.stdin, output: process.stdout });
        this.data = new WardrobeData();
    }

    private _ask(prompt: string): Promise<string> {
        return new Promise(resolve => this.rl.question(prompt, resolve));
    }

    private async showMenu() {
        const total = this.data.items.length;
        const categories = Object.keys(this.data.getCategoryStats()).length;
        console.log('\n' + c('═'.repeat(50), colors.cyan));
        console.log(c('👗 WARDROBE ORGANIZER', colors.bright + colors.cyan));
        console.log(c('═'.repeat(50), colors.cyan));
        console.log(`  Items: ${total}`);
        console.log(`  Categories: ${categories}`);
        console.log(`  Outfits: ${this.data.outfits.length}`);
        console.log(c('═'.repeat(50), colors.cyan));
        console.log('  1. 👕 Add clothing item');
        console.log('  2. 📋 List all items');
        console.log('  3. 🔍 Search items');
        console.log('  4. 📊 Statistics');
        console.log('  5. 🎭 Outfit Builder');
        console.log('  6. 🎲 Random Outfit');
        console.log('  7. ❤️  Favorites (coming soon)');
        console.log('  8. 🗑️  Delete item');
        console.log('  0. 🚪 Exit');
        console.log(c('═'.repeat(50), colors.cyan));
    }

    private listItems(items?: Item[]) {
        if (!items) items = this.data.items;
        if (!items.length) {
            console.log(c('No items found.', colors.yellow));
            return;
        }
        console.log('\n👕 WARDROBE ITEMS');
        console.log(c('─'.repeat(60), colors.dim));
        items.forEach((item, i) => {
            const emoji = COLOR_EMOJIS[item.color] || '';
            console.log(`  ${i+1}. ${emoji} ${item.name} (${item.category}) ${item.color} ${item.season}`);
        });
    }

    private async addItem() {
        const name = await this._ask('Item name: ');
        console.log(`Categories: ${CATEGORIES.join(', ')}`);
        const category = await this._ask('Category: ');
        console.log(`Colors: ${COLORS.join(', ')}`);
        const color = await this._ask('Color: ');
        console.log(`Seasons: ${SEASONS.join(', ')}`);
        const season = await this._ask('Season: ');
        const photo = await this._ask('Photo path (optional): ');
        const item = this.data.addItem(name, category, color, season, photo);
        const emoji = COLOR_EMOJIS[color] || '';
        console.log(c(`✅ Added ${emoji} ${item.name} (${item.category}, ${item.color})`, colors.green));
    }

    private async searchItems() {
        const query = await this._ask('Search term (name/category): ');
        const category = await this._ask('Filter by category (optional): ');
        const color = await this._ask('Filter by color (optional): ');
        const season = await this._ask('Filter by season (optional): ');
        const results = this.data.searchItems(query, category, color, season);
        if (results.length) this.listItems(results);
        else console.log(c('No items match your search.', colors.yellow));
    }

    private showStats() {
        const colorStats = this.data.getColorStats();
        const categoryStats = this.data.getCategoryStats();
        const seasonStats = this.data.getSeasonStats();
        console.log('\n📊 STATISTICS');
        console.log(c('─'.repeat(30), colors.dim));
        console.log('\n🎨 Colors:');
        Object.entries(colorStats).sort((a, b) => b[1] - a[1]).forEach(([color, count]) => {
            const emoji = COLOR_EMOJIS[color] || '';
            console.log(`  ${emoji} ${color}: ${count}`);
        });
        console.log('\n📂 Categories:');
        Object.entries(categoryStats).sort((a, b) => b[1] - a[1]).forEach(([cat, count]) => {
            console.log(`  ${cat}: ${count}`);
        });
        console.log('\n🌦️ Seasons:');
        Object.entries(seasonStats).sort((a, b) => b[1] - a[1]).forEach(([season, count]) => {
            console.log(`  ${season}: ${count}`);
        });
    }

    private async outfitBuilder() {
        if (!this.data.items.length) {
            console.log(c('No items. Add some clothing first!', colors.yellow));
            return;
        }
        const name = await this._ask('Outfit name: ');
        console.log('Select items for your outfit:');
        this.listItems();
        const itemIds: string[] = [];
        while (true) {
            const choice = await this._ask('Enter item number to add (or "done"): ');
            if (choice.toLowerCase() === 'done') break;
            const idx = parseInt(choice) - 1;
            if (idx >= 0 && idx < this.data.items.length) {
                const item = this.data.items[idx];
                if (!itemIds.includes(item.id)) {
                    itemIds.push(item.id);
                    console.log(c(`Added ${item.name}`, colors.green));
                } else {
                    console.log(c('Already added', colors.yellow));
                }
            } else {
                console.log(c('Invalid number', colors.red));
            }
        }
        if (itemIds.length) {
            const outfit = this.data.addOutfit(name, itemIds);
            console.log(c(`✅ Outfit '${outfit.name}' created with ${itemIds.length} items`, colors.green));
            this.showOutfit(outfit);
        }
    }

    private showOutfit(outfit: Outfit) {
        const items = outfit.items.map(id => this.data.getItem(id)).filter(i => i !== null) as Item[];
        console.log(`\n🎭 ${outfit.name}`);
        items.forEach(item => {
            const emoji = COLOR_EMOJIS[item.color] || '';
            console.log(`  ${emoji} ${item.name} (${item.category}) ${item.color}`);
        });
    }

    private async randomOutfit() {
        if (!this.data.items.length) {
            console.log(c('No items. Add some clothing first!', colors.yellow));
            return;
        }
        const season = await this._ask('Season (optional, press Enter for all): ');
        const outfit = this.data.randomOutfit(season);
        if (!outfit.length) {
            console.log(c('No items for this season.', colors.yellow));
            return;
        }
        console.log('\n🎲 Random Outfit');
        outfit.forEach(item => {
            const emoji = COLOR_EMOJIS[item.color] || '';
            console.log(`  ${emoji} ${item.name} (${item.category}) ${item.color}`);
        });
    }

    private async deleteItem() {
        if (!this.data.items.length) {
            console.log(c('No items to delete.', colors.yellow));
            return;
        }
        this.listItems();
        const choice = await this._ask('Enter item number to delete (or "cancel"): ');
        if (choice.toLowerCase() === 'cancel') return;
        const idx = parseInt(choice) - 1;
        if (idx >= 0 && idx < this.data.items.length) {
            const item = this.data.items[idx];
            const confirm = await this._ask(`Delete '${item.name}'? (yes/no): `);
            if (confirm.toLowerCase() === 'yes') {
                this.data.deleteItem(item.id);
                console.log(c(`🗑️  Deleted ${item.name}`, colors.yellow));
            }
        } else {
            console.log(c('Invalid number', colors.red));
        }
    }

    async run() {
        console.clear();
        console.log(c('\n👗 Wardrobe Organizer – Smart Clothing Manager', colors.bright + colors.cyan));
        console.log(c('Catalog your wardrobe, plan outfits!', colors.dim));

        while (true) {
            await this.showMenu();
            const choice = await this._ask('Your choice: ');
            switch (choice.trim()) {
                case '1': await this.addItem(); break;
                case '2': this.listItems(); break;
                case '3': await this.searchItems(); break;
                case '4': this.showStats(); break;
                case '5': await this.outfitBuilder(); break;
                case '6': await this.randomOutfit(); break;
                case '7': console.log(c('❤️ Favorites feature coming soon!', colors.dim)); break;
                case '8': await this.deleteItem(); break;
                case '0':
                    console.log(c('👋 Stay stylish! Goodbye!', colors.cyan));
                    this.rl.close();
                    return;
                default: console.log(c('❌ Invalid choice.', colors.red));
            }
            if (choice !== '0') {
                console.log('\nPress Enter to continue...');
                await this._ask('');
            }
        }
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────

const main = async (): Promise<void> => {
    try {
        const app = new WardrobeApp();
        await app.run();
    } catch (e: any) {
        console.error(c(`❌ Unexpected error: ${e.message}`, colors.red));
        process.exit(1);
    }
};

main();
