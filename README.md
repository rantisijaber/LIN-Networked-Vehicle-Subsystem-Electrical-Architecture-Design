# LIN-Networked Vehicle Subsystem — Architecture

## Overview

A two-node automotive electrical subsystem using LIN (Local Interconnect Network) as the communication bus. One node (**Master**) reads a physical input and broadcasts it on a schedule. The other node (**Slave**) listens for that broadcast and actuates a motor in response. The design covers four layers: **power distribution**, **network/communication**, **signal actuation**, and **requirements verification**.

---

## Architecture Diagram

<!-- Insert diagram image here, e.g. ![Architecture Diagram](./diagram.png) -->

---

## 1. Power Distribution Layer

| Element | Role |
|---|---|
| 12V Battery | Primary vehicle power source |
| 5A Fuse | Protects the wiring harness downstream of the battery |
| +12V Rail | Shared power bus feeding both nodes |
| Common Ground (GND) | Shared return path for both nodes — required for LIN's single-ended signaling to work correctly |

Both nodes tap off the same +12V rail and share the same ground plane.

### Load Analysis

| Node | Component | Measured/Rated Current | Notes |
|---|---|---|---|
| Master | STM32 board | ~80 mA | Active mode, measured via multimeter in series with supply |
| Master | Switch/pot circuit | <5 mA | Negligible, pull-up/pull-down resistor current only |
| Slave | STM32 board | ~80 mA | Active mode |
| Slave | L293D + motor (stall/running) | ~600 mA typical, up to ~1A stall | Motor datasheet + measured running current |
| **Total system draw** | | **~750 mA – 1.2A worst case** | Used to size fuse and wire |

**Fuse sizing:** A 5A fuse gives roughly 4x margin above worst-case total draw (~1.2A), which protects the wiring harness while avoiding nuisance trips from motor inrush current at startup.

**Wire gauge sizing:** For the ~1.2A worst-case load at a short harness length (bench-scale, <1m runs), 22 AWG is comfortably rated (typically good for several amps at this length per standard automotive wire ampacity charts). If this were scaled to a longer real vehicle harness run, gauge would need to be re-checked against voltage drop over distance, not just ampacity.

---

## 2. Node Architecture

### Master Node

| Block | Function |
|---|---|
| Switch / Potentiometer | Physical input the user actuates |
| STM32 MCU (LIN Master) | Reads input, builds LIN frame, schedules transmission |
| TJA1020 Transceiver | Converts 3.3V UART logic ↔ 12V LIN bus levels |
| 1kΩ Pull-up Resistor (to VBAT) | Sets the LIN bus's idle/recessive state — master side only |

### Slave Node

| Block | Function |
|---|---|
| TJA1020 Transceiver | Converts 12V LIN bus levels ↔ 3.3V UART logic |
| STM32 MCU (LIN Slave) | Detects frame, validates checksum, extracts data |
| L293D Motor Driver | Amplifies STM32's low-power PWM signal to drive the motor |
| DC Motor | Physical actuator — the system's output |

### Shared Bus

| Element | Role |
|---|---|
| LIN Bus (single wire) | Carries all communication between master and slave, 19200 baud |

---

## 3. Signal Flow

**Master path:** The switch/potentiometer is read via GPIO/ADC by the master STM32. On a periodic schedule, the STM32 builds a LIN frame (Break field, Sync byte, Protected ID, Data byte, Checksum) and transmits it over USART. The TJA1020 transceiver converts this 3.3V UART signal into 12V LIN bus levels and drives it onto the bus.

**Slave path:** The slave's TJA1020 detects the incoming frame and converts the LIN bus voltage levels back down to 3.3V UART logic. The slave STM32 receives the frame, validates the checksum, and extracts the data byte. That value is mapped to a PWM duty cycle, which drives the L293D motor driver. The L293D switches the 12V rail through to the motor, producing physical actuation.

**Power path:** The 12V battery feeds through a 5A fuse onto a shared +12V rail, which powers both nodes. Both nodes share a common ground return, which the LIN bus's single-ended signaling depends on.

---

## 4. LIN Network Schedule

| Field | Value |
|---|---|
| Baud rate | 19200 |
| Break field | ≥13 bit times dominant |
| Sync byte | 0x55 |
| Checksum type | Enhanced (covers PID + data bytes) |

| Frame ID (PID base) | Direction | Payload | Update Rate | Purpose |
|---|---|---|---|---|
| 0x10 | Master → Slave | 1 byte: switch/pot value (0–255) | 20 ms | Commands slave actuator position/speed |
| 0x11 | Slave → Master | 1 byte: status/fault flag | On request or 100 ms | Reports slave health back to master |

**Timing budget:** With a 20ms master transmit period, the system targets end-to-end latency (switch movement to motor response) of under 50ms, giving roughly 2.5x margin over a single transmit cycle to account for frame transmission time, checksum validation, and PWM update.

---

## 5. Design Rationale

- **LIN over CAN**: LIN's simpler single-wire, master/slave model fits a small point-to-point actuator subsystem without the cost or complexity of CAN's peer-arbitration hardware.
- **Transceiver required on both nodes**: STM32 logic operates at 3.3V while the LIN bus operates at 12V — the TJA1020 bridges that voltage domain gap in both directions.
- **Pull-up resistor only on the master**: A single pull-up defines one predictable idle/recessive bus state. Duplicating it on the slave would create unpredictable parallel resistance and an ill-defined bus voltage.
- **Motor driver required**: An STM32 GPIO can source roughly 20mA, far below what a DC motor needs to operate. The L293D acts as the amplification stage between a low-power control signal and the higher-current power delivery the motor requires.
- **Enhanced checksum over classic**: Including the PID in the checksum protects against a corrupted identifier, not just corrupted data — relevant since a corrupted PID could otherwise cause a node to misinterpret which frame it received.

---

## 6. Requirements Traceability

| Requirement | Design Decision | Verification Method |
|---|---|---|
| Master shall transmit switch state every 20ms | TIM2 configured for 20ms periodic interrupt driving frame transmission | Logic analyzer capture measuring time between consecutive break fields |
| Slave shall actuate motor within 50ms of a valid frame | Interrupt-driven slave RX (no polling delay) + 20ms schedule gives margin | Logic analyzer timestamp from break field to PWM/GPIO change |
| System shall detect and reject a corrupted frame | Enhanced checksum validated before acting on any data byte | Bench test: intentionally corrupt a data byte, confirm slave ignores frame and motor does not respond |
| System shall not exceed fuse-rated current under worst-case load | 5A fuse sized with ~4x margin above ~1.2A worst-case measured draw | Multimeter measurement of total current draw under motor stall condition |
| Bus shall maintain a defined idle state | 1kΩ pull-up to VBAT installed on master node only | Oscilloscope measurement of bus voltage during idle (no transmission) period |
