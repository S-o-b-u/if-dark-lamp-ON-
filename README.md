# if (dark) lamp = ON;

> A minimalist smart lamp that decides when you need light — without cloud, WiFi, or nonsense.

---

## 🧠 Overview

This project is an **Arduino-based smart ambient lamp** that automatically turns ON in darkness and OFF in brightness using a **Light Dependent Resistor (LDR)**.

It features:
- Automatic & Manual modes
- Finite State Machine (FSM) based logic
- Hysteresis for stable sensor behavior
- Non-blocking timing (no `delay()` abuse)
- OLED UI with live sensor feedback
- Audible feedback via buzzer

Designed as a **first serious hardware project** with clean architecture and polished UX.

---

## ✨ Features

- 🌗 **Automatic light control** using ambient brightness
- 🔘 **Manual override button**
- 🧠 **Finite State Machine (FSM)** for predictable behavior
- 🛑 **Hysteresis** to avoid flickering near thresholds
- 📊 **Live LDR bar graph** on OLED
- 🔔 **Event-based buzzer feedback**
- 🚫 **No delay() blocking**
- 💡 **Clean OLED UI (no flicker)**

---

## 🧠 System States (FSM)

| State | Description |
|-----|------------|
| `AUTO_BRIGHT_OFF` | Bright environment → Lamp OFF |
| `AUTO_DARK_ON` | Dark environment → Lamp ON |
| `FORCE_ON` | Manual override → Lamp forced ON |
| `FORCE_OFF` | Manual override → Lamp forced OFF |

Transitions occur via:
- LDR threshold crossing (AUTO mode)
- Button press (Manual override)

---

## 🔌 Hardware Components

| Component | Quantity |
|---------|----------|
| Arduino Uno | 1 |
| LDR Module | 1 |
| OLED Display (SSD1306) | 1 |
| LED + Resistor | 1 |
| Push Button | 1 |
| Buzzer | 1 |
| Breadboard + Jumper Wires | — |

---

## 🖥 OLED UI Layout

- **Top:** Mode (AUTO / MANUAL)
- **Center:** Current system state (DARK / LIGHT / ON / OFF)
- **Bottom:** Real-time LDR bar graph
- **Flash feedback:** Brief screen inversion on state changes

---

## ⚙️ How It Works

1. LDR continuously measures ambient light
2. FSM evaluates state transitions using hysteresis
3. Lamp output updates based on state
4. OLED updates **only on change** (no flicker)
5. Button cycles override modes
6. Buzzer confirms every state transition

---

## 🧪 Why Hysteresis?

Without hysteresis, small light fluctuations cause rapid ON/OFF flicker.

This project uses:
- `DARK_ON` threshold to turn lamp ON
- `BRIGHT_OFF` threshold to turn lamp OFF

This creates a **stable dead zone**.

---

## 📂 Project Structure

