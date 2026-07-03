# PEBBLE FREEDROID TAKEOVER

Pebble Time 2 (emery) app recreating the takeover mini-game from Freedroid RPG.

## Build

```bash
# Build locally (requires pebble-tool + SDK)
export PATH="$HOME/.local/bin:$PATH"
make build

# Build with Docker
make docker-run
```

## Gameplay

Circuit-board takeover game. Place amplifiers (capsules) to propagate your signal through the board. The display column shows who controls each row — majority wins.

### Flow

1. **Player droid** silhouette shown (press Select)
2. **Enemy droid** silhouette shown (press Select)
3. **Color select**: Up/Down to choose side, Select to confirm
4. **Game**: Up/Down move cursor, Select places capsule
5. **Result**: Win → advance to next droid, Lose → reset to 001

### Progression

- Player starts as **001** (class 0, 3 capsules)
- Defeat 23 enemy types in sequence: 123→139→...→999
- Win → take enemy droid number, Lose → reset to 001
- Capsules = `3 + class` (player) / `4 + class` (enemy), capped at class 6/7

### Droid Classes

| Class | Player Max | Enemy Max | Player Caps | Enemy Caps |
| ----- | ---------- | --------- | ----------- | ---------- |
| 0-6   | 6          | 6         | 3-9         | 4-10       |
| 7     | -          | 7         | -           | 11         |

### Controls

| Button | Action                            |
| ------ | --------------------------------- |
| Up     | Move selector up / Switch color   |
| Down   | Move selector down / Switch color |
| Select | Place capsule / Confirm / Advance |
| Back   | Exit app                          |

### Persistence

- Current droid number saved via `persist_read/write_int`

### Platform

- **emery** (Pebble Time 2, 200×228, 64 colors, 128KB RAM)
- Circuit elements drawn procedurally (pixel-art style rectangles)
- Droid silhouettes drawn procedurally (8 distinct class shapes)
- No JavaScript

## Memory

- RAM: ~8KB footprint, ~123KB free heap
- Resources: ~4KB (menu icon only)
