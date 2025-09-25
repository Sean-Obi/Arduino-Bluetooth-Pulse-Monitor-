# Heart Rate Monitor (Arduino) — LCD + Bluetooth + PulseSensor

A simple Arduino sketch that reads heart rate from a PulseSensor, smooths the BPM with a weighted moving average + low-pass blend, displays it on a 16×2 LCD, and streams both raw and filtered values over Serial and Bluetooth (HC-05).

> ❗️**Not a medical device.** This is for hobby/education only and must not be used for diagnosis or treatment.

---
## Features
- Reads heartbeats using **PulseSensorPlayground** with hardware LED blink on beat.
- **Two-stage smoothing** for stable BPM:
  - Weighted Moving Average (exponential weights across last *N* readings).
  - 80/20 low-pass blend with previous smoothed value.
- 16×2 **LCD output** (LiquidCrystal).
- **Bluetooth (HC-05)** broadcast of user-friendly text + numeric BPM (SoftwareSerial).
- **Serial Plotter** stream of `raw,smoothed` for live graphs.
- Adjustable **threshold**, filter size, and weighting behavior.

---

## Hardware
- Arduino Uno (or compatible 5V board).
- PulseSensor (Analog).
- 16×2 LCD with parallel interface (HD44780 compatible).
- HC-05 Bluetooth module.
- Jumper wires, breadboard, USB cable.

### Power
- Arduino via USB (5V).
- HC-05 from 5V and GND (some boards prefer 3.3V logic on RX—use a divider if required).
- PulseSensor from 5V and GND.

---

## Pinout / Wiring

| Component | Signal | Arduino Pin |
|---|---|---|
| PulseSensor | Signal (GREEN) | **A0** (`PulseWire = 0`) |
| PulseSensor | VCC / GND | 5V / GND |
| LCD | RS | **12** |
| LCD | EN | **11** |
| LCD | D4 | **5** |
| LCD | D5 | **4** |
| LCD | D6 | **3** |
| LCD | D7 | **2** |
| LCD | VCC / GND / VO | 5V / GND / contrast pot to VO |
| HC-05 | TXD → Arduino RX | **10** (SoftwareSerial RX) |
| HC-05 | RXD ← Arduino TX | **9**  (SoftwareSerial TX) *(use a divider if needed)* |
| HC-05 | VCC / GND | 5V / GND |
| On-board LED | Beat blink | `LED_BUILTIN` (Pin 13 on Uno) |

> Note: `SoftwareSerial BTSerial(10, 9);` maps **(RX, TX) = (10, 9)**.

---

## Software Requirements
Arduino IDE 1.8+ or Arduino IDE 2.x with the following libraries:

- **LiquidCrystal** (bundled with Arduino)
- **PulseSensorPlayground**
- **SoftwareSerial** (bundled with Arduino AVR boards)

Install **PulseSensorPlayground** via Library Manager (`Sketch → Include Library → Manage Libraries…`).

---

## Installation
1. Open the sketch in Arduino IDE.
2. Select the correct board and port.
3. **Verify/Compile** then **Upload** to the Arduino.
4. (Optional) Open **Tools → Serial Plotter** at **115200 baud**.

---

## Configuration

Edit these constants near the top of the sketch:

```cpp
const int PulseWire = 0;        // A0
const int LED = LED_BUILTIN;    // onboard LED
int Threshold = 250;            // beat detection threshold (lower = more sensitive)
const int filterSize = 20;      // window length for WMA smoothing
// Exponential weights: weights[i] = pow(0.9, filterSize - i - 1);
```

Other I/O setup already matches the pin table above:
```cpp
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
SoftwareSerial BTSerial(10, 9);
```

Baud rates:
- **Serial (USB):** `115200`
- **Bluetooth (HC-05):** `9600` (HC-05 default)

---

## How It Works

1. **Beat Detection**  
   `pulseSensor.sawStartOfBeat()` and `getBeatsPerMinute()` provide raw BPM when a beat starts; LED blinks on beat; a `Threshold` screens noise.

2. **Smoothing**  
   - **Weighted Moving Average (WMA):** Circular buffer of last `filterSize` BPM values. Exponential weights (base `0.9`) emphasize recent readings.  
   - **Low-Pass Blend:**  
     `smoothedBPM = 0.8 * smoothedBPM + 0.2 * WMA;`

3. **Output**  
   - **Serial Plotter:** prints `rawBPM,smoothedBPM` (CSV on one line).  
   - **Serial Monitor:** human-readable lines.  
   - **LCD:** first line label; second line filtered BPM with 1 decimal.  
   - **Bluetooth:** `"Your heart rate is: <smoothedBPM>
"` plus heartbeat events.

---

## Usage

1. Power the Arduino and attach the PulseSensor to a fingertip or earlobe. Stay still.
2. After the splash screen (“HR Monitor”), the LCD updates every beat.
3. Pair your phone/PC with **HC-05** (typical PIN `1234` or `0000`) and open a serial terminal at **9600** to view Bluetooth messages.
4. To see a live graph, open **Tools → Serial Plotter** at **115200**. You’ll see two lines: raw and smoothed.

---

## Data Output

### Serial Plotter (115200)
```
<raw_bpm>,<smoothed_bpm>
78,76.4
80,76.9
79,77.2
...
```

### Serial Monitor (115200)
```
Raw BPM78   Filtered BPM: 76.4
Raw BPM80   Filtered BPM: 76.9
...
```

### Bluetooth (9600)
```
♥  A HeartBeat Happened ! Your heart rate is: 76.9
```

---

## Troubleshooting

- **No beats detected / constant 0 or noise**
  - Ensure the PulseSensor signal wire is on **A0** and GND/VCC are correct.
  - Increase sensitivity: **lower** `Threshold` slightly (e.g., 230). If false beats occur, **raise** it.
  - Keep still; avoid motion and cable tugging.

- **LCD shows garbled characters**
  - Check pin order and contrast (potentiometer on VO).
  - Confirm `lcd.begin(16, 2)` and wiring match pins in the sketch.

- **Bluetooth not receiving data**
  - Verify `BTSerial` wiring: HC-05 TXD → Arduino **10**, RXD ← **9** (use a resistor divider to 3.3V logic on HC-05 RX).
  - Terminal set to **9600 baud**, **No line ending** or **\n** only.

- **Serial Plotter shows one line only**
  - Ensure lines are printed as `raw,smoothed` **on the same line** before newline.

- **Readings jump or lag**
  - Reduce `filterSize` (e.g., 12–16) for less latency.
  - Adjust exponential base (0.9 → 0.85 for stronger emphasis on newest samples).
  - Verify good sensor contact; warm fingertips help.

---

## Notes & Limitations
- Delay is set to **50 ms** for stability; lowering may increase responsiveness but also noise.
- WMA + low-pass add **intentional smoothing latency** (~several beats depending on `filterSize`).
- HC-05 RX pin is **not 5V tolerant** on many boards—use a level shifter or resistor divider from Arduino TX (pin 9).
- The sketch prints a heartbeat message from the PulseSensor library each beat; you can remove it to reduce Bluetooth chatter.

---

## Project Structure

```
HeartRateMonitor/
└── HeartRateMonitor.ino   # The sketch provided in this README
```
---

## Acknowledgements
- PulseSensor Playground Library  
- Arduino LiquidCrystal Library  
- SoftwareSerial Library  
