# HubVolt & AtmoSync — Hardware & Firmware Resources

![ESP32](https://img.shields.io/badge/ESP32-Firmware-blue?logo=espressif)
![KiCAD](https://img.shields.io/badge/KiCAD-Schematics-orange?logo=kicad)
![Gerber](https://img.shields.io/badge/Gerber-PCB-green)
![3D Design](https://img.shields.io/badge/3D--Enclosure-Design-yellow)
![C++](https://img.shields.io/badge/Arduino-Source-blue?logo=arduino)

---

## System Architecture

### 📡 AtmoSync Architecture
The HubVolt system centers around an ESP32 microcontroller connected to a transistor-driven buck converter that enables or disables the USB hub's power supply. The firmware listens for TCP commands to activate or deactivate the USB Hub power. A web dashboard enables seamless control and configuration. Safety features such as emergency shutdown and Wi-Fi fallback modes ensure reliability.

![AtmoSync Architecture](AtmoSync/AtmoSync.svg)

### 🔌 HubVolt Architecture
AtmoSync uses an ESP32 microcontroller interfaced with an SHT31 temperature and humidity sensor via I2C. It continuously displays data on an 8x32 LED matrix using the MAX7219 module and communicates the data to a central server via TCP or POST. The firmware includes dynamic AP+STA Wi-Fi configuration and sensor readouts periodically for live environmental tracking.

![HubVolt Architecture](HubVolt/HubVolt.svg)

Both **HubVolt** and **AtmoSync** are ESP32-based IoT systems. HubVolt focuses on **USB hub power control**, while AtmoSync is a **temperature and humidity logger** with display capabilities. Each system uses a web-based configuration interface and communicates with a server for registration and command handling.

---

## Folder Structure

```
All files/
├── AtmoSync/
│   ├── Schematic_AtmoSync_v1/      # KiCAD or PDF schematic files for AtmoSync
│   ├── AtmoSync.svg                # Schematic as SVG image
│   ├── AtmoSync_v1.ino             # Arduino/ESP32 firmware source code
│   └── Gerber_AtmoSync_v1.zip      # Gerber files for PCB fabrication
|   └── AtmoSync Setup Guide        # Setup Guide for AtmoSync
├── HubVolt/
│   ├── Enclosure_design/           # 3D enclosure design files (STL/STEP/etc.)
│   ├── Schematic_HubVolt_v1/       # KiCAD or PDF schematic files for HubVolt
│   ├── Gerber_HubVolt_v1.0.zip     # Gerber files v1.0 for PCB, the version 1.0 uses adjustable Buck converter
│   ├── Gerber_HubVolt_v1.1.zip     # Gerber files v1.1 for PCB, the version 1.1 uses fixed voltage Buck converter
│   ├── HubVolt.svg                 # Schematic as SVG image
│   └── HubVolt_v1.ino              # Arduino/ESP32 firmware source code
|   └── HubVolt Setup Guide         # Setup Guide for HubVolt
└── readme.md                       # This documentation file
```

---

## Components List

### 🔧 AtmoSync Components

| Component               | Quantity | Description                          |
|------------------------|----------|--------------------------------------|
| ESP32 Dev Board        | 1        | Core microcontroller                 |
| SHT31 Sensor           | 1        | Temperature & humidity sensor        |
| MAX7219 Display Module | 1        | 8x32 LED Matrix (4-in-1)             |
| Buck converter         | 1        | step down DC power to 5v             |


### 🔌 HubVolt Components

| Component             | Quantity | Description                            |
|----------------------|----------|----------------------------------------|
| ESP32 Dev Board      | 1        | Core microcontroller                   |
| Transistor (e.g., 2N2222) | 1     | Cntrolling switching power supply of USB HUB   |
| Buck converter       | 2        | step down DC power to 5v               |
| USB Hub module       | 1        | provide USB hub functionality           |
---

## First-Time Setup Guide

To change saved WiFi or server configuration after initial setup:
- First, turn OFF the power supply to the device.
- Press and HOLD the **boot button**.
- Turn ON the power supply **while keeping the boot button pressed**.
- Hold for **at least 1 second**, then release.
- The device will now boot into configuration mode.

1. **Power the ESP32** with USB or 5V supply.
2. Press the **boot button** before power-on to clear saved WiFi & server config.
3. Connect to the Access Point: `ESP32-Setup` → Password: `configureme`
4. Open browser → navigate to `192.168.4.1`
5. Fill the configuration form:
   - Device Name (e.g., atmosync01 or hubvolt01)
   - WiFi SSID / Password
   - Server IP and Port
   - Auth method (Open / Personal / Enterprise)
6. Save → Device restarts → Connects to WiFi → Registers with server.

---

## Troubleshooting

### 🔁 Reconfiguration Instructions
If you want to change the WiFi and Server configuration parameters:
1. Turn off the device's power supply.
2. Press and hold the boot button.
3. While holding it, power on the device.
4. Keep it pressed for at least 1 second.
5. Release the button; the device will enter setup mode.

Then reconnect to `ESP32-Setup` WiFi and open `192.168.4.1` to reconfigure.

| Issue                              | Possible Cause                      | Solution                               |
|-----------------------------------|-------------------------------------|----------------------------------------|
| Device not visible on network     | Wrong WiFi credentials              | Reboot with boot button, reconfigure   |
| Configuration page not loading    | Not in AP mode                      | Check serial monitor for IP info       |
| Data not displaying on matrix     | Sensor error or no data             | Check wiring; ensure SHT31 connected   |
| Power not switching (HubVolt)     | Relay not triggered                 | Check GPIO, circuit, or transistor bias|
| Device restarts unexpectedly      | Server unreachable or crash         | Verify IP, power stability, firmware   |

---

## License & Credits

Developed during the **Summer Research Internship Program (SRIP)** at **IIT Gandhinagar**.

---
