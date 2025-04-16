# SnZ Home Security System

The **SnZ Home Security System** is a two-factor authentication-based alarm system designed to enhance home security using embedded systems. The system combines button-based input and a potentiometer reading to verify identity. If a user fails to input the correct passcode three times in a row, an “Intruder!!” message is displayed on the uLCD, a buzzer sounds, and red LEDs flash. When the correct sequence is entered, a servo motor automatically opens and closes the door. Inside, an ultrasonic sensor detects when someone exits, updating a live occupancy counter shown on a TextLCD display.

---

## 🛠️ What It Does

- **Two-Factor Authentication** using buttons and a potentiometer
- **Alarm System** activates after three failed attempts:
  - Displays “Intruder!!” on the uLCD
  - Sounds a buzzer
  - Flashes red LEDs
- **Automatic Door Opening**:
  - Triggered after correct authentication
  - Opens via a Servo SG90 for 10 seconds
- **Occupancy Counter**:
  - Uses an ultrasonic sensor to detect exit events
  - Displays live count on a TextLCD screen

---

## 🔧 Components & Their Purpose

| Component          | Role in System |
|--------------------|----------------|
| `uLCD-4DGL`         | Displays passcode input, system status, and intruder warnings |
| `Servo SG90`        | Physically opens/closes the door |
| `HC-SR04 Sensor`    | Detects people exiting the home |
| `TextLCD`           | Shows the current people count |
| `Mbed LPC1768`      | Central controller for all system logic |
| `4x Pushbuttons`    | Input passcode (1, 2), delete, and enter |
| `2x Potentiometers` | Used for analog input (part of 2FA) |
| `Buzzer`            | Alerts on intruder detection |
| `Red & Yellow LEDs` | Show passcode status (fail/success) |
| `Barrel Jack Adapter` | Supplies 5V to power high-current components |

- **VIN (5V)** powers: uLCD, TextLCD, Servo, Sensor  
- **VOUT (3.3V)** powers: buttons, potentiometers, LEDs, and buzzer

---

## ⚠️ Challenges Faced

One of the biggest challenges we faced was power stability and signal interference. At various stages, the uLCD would display numbers without any buttons being pressed, and button inputs wouldn’t register properly. These issues stemmed from voltage fluctuations caused by high-draw components like the servo.

We initially changed our button configuration from pullup to pulldown resistors, but phantom values still appeared. Eventually, we isolated the servo, uLCD, TextLCD, and ultrasonic sensor by powering them through the barrel jack adapter (VIN), which significantly improved stability.

Another challenge was signal interference caused by the buzzer. When configured with PWM, it conflicted with the servo's signal, preventing it from moving. Switching the buzzer to use DigitalOut resolved the interference and restored proper servo function.

---

## 🔍 Comparison to Real-World Systems

| Feature                  | SnZ System | Traditional Systems |
|--------------------------|------------|---------------------|
| Passcode Input           | ✅          | ✅                  |
| Two-Factor Authentication| ✅          | ❌                  |
| Auto Door Mechanism      | ✅          | ❌                  |
| Occupancy Counter        | ✅          | ❌                  |
| Visual + Audio Feedback  | ✅          | ✅                  |

---

## 🚀 Future Improvements

If given more time and resources, we would:

- Replace the potentiometer with a **card reader** or **fingerprint scanner**
- Upgrade the cardboard prototype to a **3D-printed enclosure**
- Move from breadboard to a **custom PCB**
- Add **remote access** via Wi-Fi/Bluetooth for mobile alerts and control

---

## 📸 Schematic
![System Schematic](images/DrawnSchemaic4180.png)
