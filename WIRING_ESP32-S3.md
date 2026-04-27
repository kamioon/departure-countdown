# ESP32-S3-DevKitC-1 Wiring Guide

## Pin Mapping

### ESP32-S3 vs ESP32 Differences

The ESP32-S3 has a different GPIO layout compared to the classic ESP32:
- **No GPIO 23-25** (these don't exist on ESP32-S3)
- **GPIO 26-32** are not exposed on DevKitC-1
- **GPIO 33-37** are used for internal flash/PSRAM
- **GPIO 19-20** are USB D-/D+ (avoid if using USB)
- **Native USB** support (can use USB CDC for Serial)

## Complete Wiring Table

| Component | Function | ESP32-S3 GPIO | ESP32-S3 Pin | Notes |
|-----------|----------|---------------|--------------|-------|
| **MAX7219 Display** | | | | |
| | MOSI (DIN) | GPIO 11 | Pin 11 | SPI Data In |
| | SCK (CLK) | GPIO 12 | Pin 12 | SPI Clock |
| | CS (LOAD) | GPIO 10 | Pin 10 | Chip Select |
| | VCC | 5V | 5V | Power |
| | GND | GND | GND | Ground |
| **Rotary Encoder** | | | | |
| | CLK | GPIO 4 | Pin 4 | Clock signal |
| | DT | GPIO 5 | Pin 5 | Data signal |
| | SW | GPIO 6 | Pin 6 | Button press |
| | + | 3.3V | 3V3 | Power |
| | GND | GND | GND | Ground |
| **Buttons** | | | | |
| | Start Button | GPIO 7 | Pin 7 | Pull-up enabled |
| | Reset Button | GPIO 15 | Pin 15 | Pull-up enabled |
| | Mode Button | GPIO 16 | Pin 16 | Pull-up enabled |
| | Common | GND | GND | Connect other side to GND |
| **Buzzer** | | | | |
| | Positive | GPIO 17 | Pin 17 | PWM capable |
| | Negative | GND | GND | Ground |
| **RGB LED** | | | | |
| | Red | GPIO 18 | Pin 18 | PWM capable |
| | Green | GPIO 8 | Pin 8 | PWM capable |
| | Blue | GPIO 3 | Pin 3 | PWM capable |
| | Common | GND | GND | For common cathode |
| **RTC (DS3231)** | | | | |
| | SDA | GPIO 1 | Pin 1 | I2C Data |
| | SCL | GPIO 2 | Pin 2 | I2C Clock |
| | VCC | 3.3V | 3V3 | Power |
| | GND | GND | GND | Ground |

## Detailed Wiring Diagrams

### MAX7219 LED Matrix (4x 8x8 modules)

```
ESP32-S3                MAX7219 Module 1        Module 2-4
--------                ----------------        -----------
GPIO 11 (MOSI) ────────► DIN
GPIO 12 (SCK)  ────────► CLK
GPIO 10 (CS)   ────────► CS/LOAD
5V             ────────► VCC
GND            ────────► GND
                         DOUT ─────────────────► DIN ──► DOUT ──► ...
```

**Important Notes:**
- Use **5V** for VCC (MAX7219 requires 5V)
- ESP32-S3 outputs 3.3V logic, but MAX7219 accepts it
- Add **1000µF capacitor** between VCC and GND for stability
- Add **100nF ceramic capacitor** near each module
- Keep data wires **short** (<15cm) for reliable communication
- Power consumption: ~200-400mA at medium brightness

### Rotary Encoder

```
ESP32-S3            Rotary Encoder
--------            --------------
GPIO 4  ───────────► CLK
GPIO 5  ───────────► DT
GPIO 6  ───────────► SW (button)
3.3V    ───────────► +
GND     ───────────► GND
```

**Notes:**
- Internal pull-ups enabled in code
- Optional: Add 0.1µF capacitors between CLK/DT and GND for debouncing

### Push Buttons

```
ESP32-S3            Button
--------            ------
GPIO 7  ───────────► [Start] ──┐
GPIO 15 ───────────► [Reset] ──┼──► GND
GPIO 16 ───────────► [Mode]  ──┘
```

**Notes:**
- Buttons connect GPIO to GND when pressed
- Internal pull-ups enabled in code
- LOW = pressed, HIGH = released

### Buzzer

```
ESP32-S3            Buzzer
--------            ------
GPIO 17 ───────────► + (Positive)
GND     ───────────► - (Negative)
```

**Notes:**
- Use piezo buzzer or active buzzer
- PWM frequency controlled by code
- Optional: Add 100Ω resistor in series to reduce volume

### RGB LED

```
ESP32-S3            RGB LED
--------            -------
GPIO 18 ───220Ω────► R (Red)
GPIO 8  ───220Ω────► G (Green)
GPIO 3  ───220Ω────► B (Blue)
GND     ───────────► - (Common Cathode)
```

**Notes:**
- Use **common cathode** RGB LED
- 220Ω resistors for current limiting
- For common anode, connect common to 3.3V and invert logic in code

### RTC Module (DS3231)

```
ESP32-S3            DS3231
--------            ------
GPIO 1  ───────────► SDA
GPIO 2  ───────────► SCL
3.3V    ───────────► VCC
GND     ───────────► GND
```

**Notes:**
- DS3231 works with 3.3V
- Has built-in battery backup
- Provides accurate time when WiFi is unavailable

## Power Considerations

### Power Supply Requirements

- **ESP32-S3**: 500mA (peak)
- **MAX7219 (4 modules)**: 200-400mA (typical), up to 1A (full brightness)
- **RTC**: 1-2mA
- **Other components**: ~50mA

**Total: 750mA typical, 1.5A peak**

### Recommended Power Options

1. **USB 5V 2A adapter** (recommended)
   - Most convenient
   - Can power everything

2. **USB-C cable from computer**
   - Good for development
   - May not provide enough current for full brightness

3. **External 5V power supply**
   - Connect to 5V and GND pins
   - Best for standalone operation

### Power Distribution

```
5V Power Supply
      │
      ├───► ESP32-S3 (5V pin)
      ├───► MAX7219 VCC
      └───► GND (common ground)

ESP32-S3 3.3V regulator
      │
      ├───► Rotary Encoder (+)
      ├───► RTC VCC
      └───► GND (common ground)
```

## GPIO Restrictions on ESP32-S3

### Safe to Use (General Purpose)
- GPIO 1-18 (used in this project)
- GPIO 21
- GPIO 38-42, 45-48

### Avoid Using
- **GPIO 0**: Boot button (connected to button on board)
- **GPIO 19-20**: USB D-/D+ (if using USB Serial)
- **GPIO 26-32**: Not exposed on DevKitC-1
- **GPIO 33-37**: Flash/PSRAM (CRITICAL - do not use)
- **GPIO 43-44**: UART TX/RX (USB Serial if enabled)

## Testing Your Connections

### Step 1: Power Test
1. Connect only power (5V, GND, 3.3V)
2. Check voltages with multimeter
3. LED on board should light up

### Step 2: Display Test
1. Connect MAX7219 only
2. Upload simple display test sketch
3. All LEDs should light up

### Step 3: Button Test
1. Connect buttons
2. Monitor Serial output
3. Press each button, verify detection

### Step 4: Full System Test
1. Connect all components
2. Upload full firmware
3. Verify all features work

## Common Issues & Solutions

### Display Not Working
- **Check power**: Measure 5V at VCC pin
- **Check wiring**: DIN→11, CLK→12, CS→10
- **Add capacitors**: 1000µF on power rail
- **Try different CS pin**: Some pins may have issues

### Buttons Not Responding
- **Check connections**: Button should connect GPIO to GND
- **Verify pull-ups**: Should read HIGH when not pressed
- **Check GPIO restrictions**: Make sure GPIO is available

### Encoder Jumpy/Unreliable
- **Add capacitors**: 0.1µF between CLK/DT and GND
- **Check connections**: Ensure solid wiring
- **Swap CLK/DT**: Try reversing if direction is wrong

### USB Serial Not Working
- **Check USB cable**: Must be data cable, not charge-only
- **Install drivers**: CH340 or CP2102 drivers
- **Check build flags**: `ARDUINO_USB_CDC_ON_BOOT=1` should be set

## Pin Assignment Summary

| GPIO | Function | Component | Type |
|------|----------|-----------|------|
| 1 | I2C SDA | RTC | I2C |
| 2 | I2C SCL | RTC | I2C |
| 3 | PWM | LED Blue | Output |
| 4 | Input | Encoder CLK | Input |
| 5 | Input | Encoder DT | Input |
| 6 | Input | Encoder SW | Input |
| 7 | Input | Button Start | Input |
| 8 | PWM | LED Green | Output |
| 10 | SPI CS | MAX7219 CS | Output |
| 11 | SPI MOSI | MAX7219 DIN | SPI |
| 12 | SPI SCK | MAX7219 CLK | SPI |
| 15 | Input | Button Reset | Input |
| 16 | Input | Button Mode | Input |
| 17 | PWM | Buzzer | Output |
| 18 | PWM | LED Red | Output |

**Total GPIOs used: 14 out of ~30 available**

This leaves plenty of pins free for future expansion!
