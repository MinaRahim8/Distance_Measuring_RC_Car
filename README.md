# Measurement Vehicle

A remote-controlled car that measures distance. Drive it with a joystick, and it reports how far it has traveled. It can also measure the length, width, and area of a space. Readings come back over a two-way 2.4 GHz radio link and appear on an LCD on the remote.

Built by **Minahil Rahimullah** and **Marco Tinoco** at the University of Illinois Chicago for CS 362.

[Measurement vehicle and remote]
<img width="1819" height="708" alt="image" src="https://github.com/user-attachments/assets/d456595a-783d-427a-8f63-3ba52bccacbb" />

---

## Features

- **Wireless joystick control:** tank-style (differential) steering over nRF24L01 radios
- **Distance mode:** live distance traveled, in meters, from wheel encoders
- **Area mode:** measure a length, then a width, and the remote computes the area in m²
- **Two-way telemetry:** the car sends its distance back to the remote after each command
- **Link-loss handling:** the remote gives up waiting after 200 ms, so a dropped reply never freezes it

## How it works

```
 REMOTE (Arduino)                                CAR (Arduino)
 ┌──────────────────┐   joystick x, y  ──────▶   ┌──────────────────────┐
 │ Joystick         │                            │ Motor mixing         │
 │ Mode buttons     │   nRF24L01, 2.4 GHz        │ Dual H-bridge driver │
 │ 16x2 LCD         │                            │ 2 DC motors          │
 │                  │   ◀──────  distance (m)    │ 2 wheel encoders     │
 └──────────────────┘                            └──────────────────────┘
```

1. The **remote** reads the joystick's x and y axes and transmits them to the car.
2. The remote then switches to listening and waits up to 200 ms for a reply. The nRF24L01 is half-duplex, so each side alternates between sending and receiving.
3. The **car** maps the y axis to forward/reverse speed and the x axis to a turn offset. It combines them into separate left and right motor speeds, constrained to ±255, and drives the motors through the H-bridge with PWM.
4. Each wheel has an **encoder** wired to a hardware interrupt pin. Every pulse increments a counter, with a 10 ms debounce to reject noise.
5. The car averages both wheels' counts, converts them to rotations (40 pulses per revolution), multiplies by the wheel circumference, and sends the distance back.
6. The **remote** shows the distance on the LCD, or uses it for area measurement, depending on the selected mode.

Distance is computed from the encoder counts:

```
distance = (2π × r) × ((left_pulses + right_pulses) / 2) / 40
```

where `r = 0.03175 m`, the wheel radius.

## Using it

| Control | Action |
| --- | --- |
| Joystick | Drive: up/down for forward/reverse, left/right to turn |
| **Distance** button | Show distance traveled |
| **Area** button, 1st press | Mark the starting point |
| **Area** button, 2nd press | Record and display the **length** |
| **Area** button, 3rd press | Record and display the **width** |
| **Area** button, 4th press | Display the **area** (length × width) |

To measure a room: press **Area** at one corner, drive along one wall, and press it again. Then drive along the adjacent wall, press it a third time, and press once more to see the area.

## Hardware

| Part | Qty | Used in |
| ---  | --- | ---     |
| Arduino Uno (or compatible) | 2 | Remote, car |
| nRF24L01+ 2.4 GHz transceiver (the PA+LNA antenna version extends range) | 2 | Remote, car |
| Analog joystick module | 1 | Remote |
| 16x2 character LCD (HD44780) | 1 | Remote |
| Push buttons | 2 | Remote |
| L298N dual H-bridge motor driver | 1 | Car |
| Yellow TT DC gear motors with wheels (r = 3.175 cm) | 2 | Car |
| Slotted optical speed sensors with encoder discs (40 pulses/rev) | 2 | Car |
| Chassis, batteries, jumper wires | — | Both |

> **Note:** The nRF24L01 runs on **3.3 V**. Powering it from 5 V can damage it. A 10 µF capacitor across its VCC and GND helps keep the link stable.

![Car wiring]
<img width="921" height="632" alt="image" src="https://github.com/user-attachments/assets/ebb0f9c6-5a07-4c57-a26c-b73a1c33b5f8" />


![Remote wiring]
<img width="921" height="523" alt="image" src="https://github.com/user-attachments/assets/cfda5cfb-98da-47c0-93ad-342a8de1cf97" />


## Design decisions

What we tried, what we kept, and why:

| Decision | What we tried first | What we kept | Why |
| --- | --- | --- | --- |
| Remote link | Infrared | nRF24L01+ radio | IR needs line of sight, so the car lagged or lost signal whenever something blocked it. Radio passes through people and walls and reaches much farther. |
| Drive train | Four wheels | Two wheels on one L298N | Four wheels left no room on the chassis for the other components. Two wheels kept the circuit simpler. |
| Power | One shared supply | Budgeted per subsystem | Power was the hardest part of the build. Combining circuits meant repeatedly adjusting how much power the motors and logic needed, including whether the Arduinos should run on a separate source. |

## Wiring

### Remote (`mRahi2Proj1.ino`)

| Component | Pin |
| --- | --- |
| LCD RS, EN | D2, D3 |
| LCD D4, D5, D6, D7 | D4, D5, D6, D7 |
| nRF24L01 CE, CSN | D8, D9 |
| nRF24L01 MOSI, MISO, SCK | D11, D12, D13 |
| Joystick X, Y | A0, A1 |
| Area button | A2 (D16) |
| Distance button | A3 (D17) |

### Car (`mRahi2Proj2.ino`)

| Component | Pin |
| --- | --- |
| Right encoder | D2 (interrupt) |
| Left encoder | D3 (interrupt) |
| Motor driver IN1, IN2 (left) | A0 (D14), A1 (D15) |
| Motor driver IN3, IN4 (right) | D4, D7 |
| Motor driver ENA, ENB (PWM) | D5, D6 |
| nRF24L01 CE, CSN | D9, D10 |
| nRF24L01 MOSI, MISO, SCK | D11, D12, D13 |

## Radio configuration

Both boards use the same settings, with mirrored pipe addresses so each one's writing pipe is the other's reading pipe.

| Setting | Value |
| --- | --- |
| Channel | 124, chosen to stay clear of common 2.4 GHz traffic |
| Data rate | 2 Mbps |
| Power level | `RF24_PA_MIN` |
| Remote → car pipe | `0xE8E8F0F0E1` |
| Car → remote pipe | `0xE8E8F0F0E2` |

## Getting started

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Install the **RF24** library by TMRh20 from the Library Manager. `SPI` and `LiquidCrystal` come with the IDE.
3. Upload `mRahi2Proj1.ino` to the remote's Arduino.
4. Upload `mRahi2Proj2.ino` to the car's Arduino.
5. Power both boards. The remote shows "Welcome / Starting up...", then begins sending joystick data.

> The Arduino IDE expects each `.ino` file to sit in a folder with the same name, for example `mRahi2Proj1/mRahi2Proj1.ino`.

## Repository structure

```
.
├── mRahi2Proj1/
│   └── mRahi2Proj1.ino   # Remote: joystick, LCD, mode buttons
├── mRahi2Proj2/
│   └── mRahi2Proj2.ino   # Car: motors, encoders, distance calculation
├── images/               # Photos and wiring diagrams
└── README.md
```

## Known limitations and future work

- **Direction-blind encoders:** single-channel encoders count pulses but can't tell direction, so reversing adds distance. Quadrature encoders would fix this.
- **Interrupt-safe counters:** the pulse counters should be declared `volatile` and read with interrupts briefly paused, so a reading can never be corrupted mid-update.
- **Speed ceiling:** the 10 ms debounce allows at most 100 counts per second, which limits the top speed that can be measured accurately.
- **Wheel slip:** odometry assumes the wheels never slip, so smooth floors work best.
- **Simpler radio protocol:** the nRF24L01's acknowledgment payloads could carry the distance back automatically, removing the need to switch between listening and transmitting.
- **Accuracy testing:** compare readings against a tape measure across distances and surfaces.
- **Ultrasonic ranging:** the chassis carries an ultrasonic sensor on a servo, intended to measure the distance to nearby walls. Combining that with wheel odometry would let the car measure a room without driving its full length. It isn't used in the current code.

## Authors

- **Minahil Rahimullah**, hardware lead: researched and sourced parts; assembled, wired, and troubleshot the circuits; refined the pseudocode into the final firmware · [GitHub](https://github.com/MinaRahim8)
- **Marco Tinoco**, project coordinator: wrote the project description and milestones, drafted the pseudocode, and helped build the final product
