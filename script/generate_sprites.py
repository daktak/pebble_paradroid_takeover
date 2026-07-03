"""Generate pixel-art sprite PNGs for the Pebble takeover app."""

from PIL import Image
import os, sys

OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "resources", "images")

def draw_droid_pixels(size):
    """Create a battle droid silhouette as pixel data."""
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    px = img.load()

    def fill(x1, y1, x2, y2, r, g, b, a=255):
        for y in range(y1, min(y2 + 1, size)):
            for x in range(x1, min(x2 + 1, size)):
                px[x, y] = (r, g, b, a)

    cx = size // 2
    cy = size // 2

    body = (180, 200, 220)
    eye = (100, 200, 255)
    weapon = (200, 100, 100)

    # Head
    fill(cx - 4, cy - 12, cx + 4, cy - 8, *body)
    # Eye glow
    fill(cx - 2, cy - 11, cx + 2, cy - 9, *eye)
    # Body
    fill(cx - 8, cy - 7, cx + 8, cy + 4, *body)
    # Shoulders
    fill(cx - 12, cy - 6, cx - 9, cy - 3, *body)
    fill(cx + 9, cy - 6, cx + 12, cy - 3, *body)
    # Arms
    fill(cx - 14, cy - 2, cx - 10, cy + 6, *body)
    fill(cx + 10, cy - 2, cx + 14, cy + 6, *body)
    # Weapon arm
    fill(cx + 15, cy - 4, cx + 17, cy + 2, *weapon)
    # Legs
    fill(cx - 6, cy + 5, cx - 2, cy + 14, *body)
    fill(cx + 2, cy + 5, cx + 6, cy + 14, *body)
    # Feet
    fill(cx - 8, cy + 15, cx - 1, cy + 16, *body)
    fill(cx + 1, cy + 15, cx + 8, cy + 16, *body)

    return img

def generate_droid_sprites():
    """Generate individual droid silhouette PNGs for each class."""
    classes = [
        ("droid_c0", 24, lambda px, cx, cy, x, y: [
            (cx - 4, cy - 14, 8, 10, (120, 140, 160)),
            (cx - 2, cy - 16, 4, 4, (120, 140, 160)),
        ]),
    ]
    for name, sz, rects_fn in classes:
        img = Image.new("RGBA", (sz, sz), (0, 0, 0, 0))
        p = img.load()
        cx, cy = sz // 2, sz // 2
        for r in rects_fn(p, cx, cy, sz, sz):
            x1, y1, w, h, col = r
            r2, g2, b2 = col
            for y in range(y1, y1 + h):
                for x in range(x1, x1 + w):
                    if 0 <= x < sz and 0 <= y < sz:
                        p[x, y] = (r2, g2, b2, 255)
        img.save(os.path.join(OUT_DIR, f"{name}.png"))
        print(f"  Generated {name}.png")

def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    # Menu icon
    icon = draw_droid_pixels(48)
    icon.save(os.path.join(OUT_DIR, "menu_icon.png"))
    print(f"Generated menu_icon.png")

if __name__ == "__main__":
    main()
