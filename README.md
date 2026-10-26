# Pocket Pal 🎮

A tiny handheld toy built on an ESP32-S3 with a 2" color screen. Game Boy Color–style pixel art, a sliding carousel menu, and four apps — including **Pet**, a virtual pet (a curly black doodle) that lives in a pink pixel room.

Controlled with just three buttons: `<`, `>`, and `ENTER`.

---

## Apps

| App | What it does | Controls |
|-----|--------------|----------|
| **Dice** | Roll one or two dice with a spin animation | `ENTER` roll · `<` `>` switch 1/2 dice |
| **8-Ball** | Shake for one of 20 classic Magic 8-Ball answers | `ENTER` shake |
| **Fish Tank** | Goldfish swim, chase food, and blow bubbles | `ENTER` drop food · `<` remove fish · `>` add fish |
| **Pet** | Virtual pet in a pink pixel room | `<` pet · `ENTER` treat · `>` call your pet |

**Everywhere:** `<` `>` scroll the menu, `ENTER` opens an app, **hold `ENTER` (~0.7s) to go back** to the menu.

### About your pet

Your pet has three stats — **happy**, **hunger**, and **lonely** — shown as rows of pixel hearts. They slowly drain while your pet is on screen, so it needs attention:

- **Pet** → hearts float up, happiness goes up
- **Treat** → a bone and sparkles appear, hunger goes up
- **Call** → a phone rings, loneliness goes down

Your pet blinks, bounces, wags its tail (faster when you interact), sticks its tongue out when happy, and frowns when neglected. **Stats are saved to flash**, so your pet remembers how you've treated it after a power cycle. Stats don't drain while the device is off.

---

## Hardware

| Part | Notes |
|------|-------|
| **Waveshare ESP32-S3-Touch-LCD-2** (or non-touch ESP32-S3-LCD-2) | 2.0" 240×320 IPS, ST7789T3 driver, 8 MB PSRAM. Touch is not used. |
| 3× tactile push buttons | The common 4-leg kind |
| Jumper wires | For the buttons |
| *(optional)* 3.7V LiPo battery | JST PH 2.0 connector, 500–1200 mAh |
| *(optional)* SPST slide switch | Power on/off |

### Display pins (onboard — no wiring needed)

These are fixed by the board and already set in the code:

| Signal | GPIO |
|--------|------|
| SCLK | 39 |
| MOSI | 38 |
| CS | 45 |
| DC | 42 |
| RST | not connected (software reset) |
| Backlight | 1 |

### Button wiring

All three buttons go on four adjacent pads at the bottom of the **right-side header**:

| Button | Header pin | GPIO |
|--------|-----------|------|
| `<` left | pin 10 | GPIO12 |
| `>` right | pin 11 | GPIO14 |
| `ENTER` | pin 12 | GPIO9 |
| Ground (shared) | pin 13 | GND |

Each button connects its GPIO to GND. No resistors needed — the code uses internal pull-ups.

**4-leg buttons:** the legs come in two internally-connected pairs (legs on the same long side are joined). **Wire diagonal corners** — one to the GPIO, the other to GND — and ignore the remaining two legs. If a button acts permanently pressed, you've picked two legs from the same pair; swap to the other diagonal.

### Power (optional)

- **Battery:** plug a 3.7V LiPo into the **BAT** connector. The board charges it automatically whenever USB is plugged in. ⚠️ Check that the battery's red wire lines up with the **+** marking before plugging in — reversed polarity can damage the board.
- **On/off switch:** cut the **red (positive)** battery wire and put the switch in the gap. Never switch the ground wire.

---

## Software setup

### 1. Install the ESP32 boards

Arduino IDE → **Boards Manager** → install **esp32** by Espressif.

### 2. Install libraries

Arduino IDE → **Library Manager** → install:

- **Adafruit GFX Library**
- **Adafruit BusIO** (accept when prompted as a dependency)
- **Adafruit ST7735 and ST7789 Library**

`Preferences` (used for saving your pet's stats) comes with the ESP32 package.

### 3. Board settings (important!)

| Setting | Value |
|---------|-------|
| Board | **ESP32S3 Dev Module** |
| PSRAM | **OPI PSRAM** |
| USB CDC On Boot | **Enabled** |
| USB Mode | **Hardware CDC and JTAG** |
| Port | `/dev/cu.usbmodem…` (Mac) or `COM…` (Windows) |

> ⚠️ **Do not select "ESP32-S3-USB-OTG"** as the board. It's a different Espressif board, and it will compile but the screen won't update.

### 4. Upload

Open `pocket_pal_color.ino` and hit Upload.

---

## Files

| File | Purpose |
|------|---------|
| `pocket_pal_color.ino` | **The main firmware** — all four apps |
| `display_test.ino` | Blinks the screen red/blue. Use it to confirm the display works. |
| `button_test_screen.ino` | Shows button presses as screen colors (left = red, right = blue, enter = green) |
| `button_test.ino` | Same test via Serial Monitor at 115200 baud (the screen version is more reliable) |
| `pocket_pal_full_preview.html` | Clickable browser mock of the whole device — open in any browser, no hardware needed |

---

## How it works

Every frame is drawn into an off-screen framebuffer (`GFXcanvas16`, 320×240, ~150 KB) and pushed to the display in one go, which keeps animations flicker-free. That buffer lives in PSRAM — which is why PSRAM must be enabled. If it isn't, the screen shows a red "Enable PSRAM" message instead of crashing.

Your pet is a hand-made 36×40 pixel sprite stored as text rows (one character per color) and drawn at 3× scale. The tail is a separate sprite so it can wag independently.

---

## Troubleshooting

| Problem | Likely cause / fix |
|---------|--------------------|
| `Adafruit_GFX.h: No such file` | Libraries not installed — see step 2 |
| `Missing FQBN` | No board selected, or wrong board — choose **ESP32S3 Dev Module** |
| `no upload port provided` | Pick the port under **Tools → Port**. If none appears: replug, try another cable (some are charge-only), or hold **BOOT**, tap **RST**, release **BOOT** |
| Screen still shows the factory demo | Wrong board selected (often ESP32-S3-USB-OTG) |
| Screen is black | Check the display pins match the table above; run `display_test.ino` |
| Red "Enable PSRAM" screen | Set **Tools → PSRAM → OPI PSRAM** |
| Colors look inverted or washed out | Change `tft.invertDisplay(true)` to `false` in `setup()` |
| Glitchy or corrupted image | Lower `tft.setSPISpeed(40000000)` to `27000000` |
| Menu scrolls nonstop / stuck on "back" | A button is wired across same-pair legs — use the diagonal |
| Button does nothing | Short the GPIO pad to GND with a wire to test; check you're on header pins 10–13 |
| Serial Monitor shows nothing | Set baud to 115200 and enable USB CDC On Boot, or just use `button_test_screen.ino` |

---

## Customizing

Safe things to tweak:

- **Colors** — `initColors()` near the top. Each is `tft.color565(R, G, B)`.
- **8-Ball answers** — the `ANSWERS[]` array.
- **How fast your pet gets needy** — the decay amounts in `renderPet()` (e.g. `petHunger - 2.0f`) and the `3000` ms interval.
- **How much pet/treat/call helps** — the `+15`, `+20`, `+25` values in `handleInput()`.
- **Fish** — starting `nFish`, max of 8.
- **Carousel slide speed** — `slideX*7/10` in `renderMenu()` (lower = snappier).

Tip: **File → Save As** a numbered copy before big changes so you always have a working version to go back to.

---

## Ideas for later

- Touch support (the board's CST816D touch chip has an Arduino driver)
- Deep sleep on long-press to save battery
- A fourth button for a dedicated "back"
- More apps — coin flip, stopwatch, tiny games

---

## Credits

Display pin configuration from [mrRobot62/esp32-s3-touch-lcd-2](https://github.com/mrRobot62/esp32-s3-touch-lcd-2).

Made with love 🖤🐾
