# Musical Alarm Clock

An open-source musical alarm clock built with the Seeed Studio XIAO ESP32-C3, an ST7789 TFT display, and a piezo buzzer. The project includes firmware, a custom PCB (KiCad), and a 3D-printable enclosure.

Designed by **George Sun**.

## Features

- Large HH:MM clock display on a 320×240 TFT screen
- Piezo buzzer alarm with "WAKE UP!" visual alert
- 4-button interface for setting the alarm hour/minute
- Configurable alarm time with a simple state-machine UI
- Compact 90mm × 48.5mm PCB form factor

## Bill of Materials

| Component | Description |
|-----------|-------------|
| Seeed Studio XIAO ESP32-C3 | Main microcontroller |
| ST7789 TFT Display (320×240) | Driven over SPI |
| Piezo Buzzer (12mm × 9.5mm) | Alarm output |
| 4× Push-button switches | User input |
| 1×8 pin header (2.54mm) | Display connector |

## Project Structure

```
musical-alarm-clock/
├── CAD/
│   ├── CAD.step                  # Enclosure 3D model
│   └── musicalalarmclock.step    # PCB 3D model
├── PCB/
│   ├── musicalalarmclock.kicad_sch   # Schematic
│   ├── musicalalarmclock.kicad_pcb   # PCB layout
│   └── musicalalarmclock.kicad_pro   # KiCad project config
└── firmware/
    └── musicalalarmclock/
        └── musicalalarmclock.ino      # Arduino firmware
```

## Building the Firmware

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. In **Boards Manager**, install the **Seeed Studio XIAO ESP32-C3** board package.
3. In **Library Manager**, install the following libraries:
   - `Adafruit ST7789`
   - `Adafruit GFX Library`
4. Open `firmware/musicalalarmclock/musicalalarmclock.ino`.
5. Select **Seeed XIAO ESP32-C3** as the board and upload via USB.

Serial monitor is available at **115200 baud** for debug output.

## Pin Mapping

| Function | GPIO |
|----------|------|
| TFT SCLK | 9 |
| TFT MOSI | 10 |
| TFT RST | 8 |
| TFT DC | 6 |
| TFT CS | 7 |
| TFT Backlight | 21 |
| Button 1 (Decrement) | 5 |
| Button 2 (Increment) | 4 |
| Button 3 (Mode) | 3 |
| Button 4 (Cancel/Confirm) | 2 |
| Buzzer | 20 |

## Fabricating the PCB

Open the KiCad project in `PCB/` to view the schematic and board layout. Use the generated drill files in `PCB/musicalalarmclock/` to send to a PCB fab house. The board is a 2-layer design with a 90mm × 48.5mm outline.

## 3D Enclosure

STEP models for the enclosure and PCB assembly are in `CAD/`. Use these with your preferred CAD software or send to a 3D printing service.

## License

This project is open source. See the repository for license details.
