# Nios V Soft-Processor & Accelerometer Integration

**Author:**  Zeaiter Yehia 

---

## Overview

This project involves the design and implementation of a soft-core processor (Intel Nios V) on an FPGA.The system is designed to control peripherals, establish communication protocols, and process sensor data to create interactive applications.

The project milestones include:
1.  **System Architecture:** Configuring a Nios V/m processor using Platform Designer
2.  **LED Control:** Implementing a "Caterpillar" (Chaser) light pattern
3.  **Sensor Integration:** Communicating with an ADXL345 Accelerometer via I2C.
4.  **Application:** Creating a digital "Spirit Level" based on the Y-axis tilt.

The ultimate goal was to integrate this system into a "Magic Screen" (Telecran) project, allowing the screen to be erased by rotating or shaking the board.

---

## Project Structure

The project directory requires a strict hierarchy to manage RTL, simulation, and software files.

```text
tp_nios_v/
[cite_start]├── rtl/        # VHDL source codes [cite: 27]
[cite_start]├── sim/        # Simulation files [cite: 29]
[cite_start]├── soft/       # Software development (C code & BSP) [cite: 31]
│   ├── app/    # Application source code
│   ├── bsp/    # Board Support Package
[cite_start]│   └── sopc/   # Nios V configuration files [cite: 38]
[cite_start]└── synt/       # Quartus synthesis project [cite: 28]
````

-----

## Hardware Architecture

### Platform Designer Configuration

The system is built around the **Intel Nios V/m** processor (RISC-V architecture)[cite: 46]. [cite_start]The following components are connected via the Avalon interconnect:

  * **Processor:** Nios V/m Microcontroller.
  * **Memory:** On-Chip RAM.
  * **JTAG UART:** For terminal communication and debugging.
  * **PIO (Parallel I/O):** For driving the 10 LEDs.
  * **Avalon I2C (Master):** For communication with the ADXL345 accelerometer.

### VHDL Top-Level Integration

The top-level entity, `tp_nios_v`, instantiates the Nios system and manages the tri-state buffers required for the I2C protocol (SDA and SCL lines).

**VHDL Snippet (I2C Logic):**

```vhdl
[cite_start]-- Connecting bidirectional buffers for I2C [cite: 249-252]
s_i2c_scl_in <= io_i2c_scl;
io_i2c_scl <= '0' when s_i2c_scl_oe = '1' else 'Z';

s_i2c_sda_in <= io_i2c_sda;
io_i2c_sda <= '0' when s_i2c_sda_oe = '1' else 'Z';
```

-----

## Software Environment

The software is developed using the **Nios V Shell** and **RiscFree IDE**.

### Initialization

To prepare the development environment, the Board Support Package (BSP) and Application project are generated using the following commands:

```bash
# Generate BSP (HAL)
niosv-bsp -c -t=hal --sopc-info=sopc/nios.sopcinfo soft/bsp/settings.bsp

# Generate Application
niosv-app -a=soft/app/ -b=soft/bsp/ -s=soft/app/main.c
```

### Debugging

The application (`app.elf`) is downloaded to the board via JTAG, and output is monitored using the `juart-terminal`.

-----

## Key Features & Implementation

### 1\. LED Chaser (Chenillard)

The LEDs are controlled via the PIO module to create an animation where the line of lights "extends" and "retracts".

  * **Mechanism:** Uses bitwise masks to manipulate the variable `i` written to `PIO_0_BASE`.
  * **Extension:** Shifts the mask left (`<< 1`) and applies bitwise OR.
  * **Retraction:** Shifts the mask right (`>> 1`) and applies bitwise AND.

### 2\. Accelerometer (ADXL345)

The system reads the X, Y, and Z axes from the ADXL345 sensor using the `altera_avalon_i2c` library.

**Configuration Parameters:**

  * **Slave Address:** `0x1D`.
  * **Power Control Register:** `0x2D` (Value `0x08` to activate Measure Mode).
  * **Data Register:** `0x32` (Start reading 6 bytes here).

**Data Processing:**
The 16-bit signed acceleration values are reconstructed from the 6-byte receive buffer:

```c
x = (rxbuffer[1] << 8) | rxbuffer[0];
y = (rxbuffer[3] << 8) | rxbuffer[2];
z = (rxbuffer[5] << 8) | rxbuffer[4];
```

### 3\. Spirit Level

This feature maps the Y-axis inclination to the LED array.

  * **Logic:** The code calculates which LED index (`0` to `9`) corresponds to the current tilt value.
  * **Visual:** A single LED lights up to represent the "bubble" position.
  * **Code Snippet:**
    ```c
    int led_to_light = (y * LED_COUNT) / Y_MAX_ABS;
    uint16_t leds = (1 << led_to_light);
    IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, leds);
    ```
    

-----

## Future Improvements

The original roadmap included integrating the accelerometer with a "Magic Screen" display project. [cite_start]The intended functionality was to trigger a screen clear/erase command when the accelerometer detected a "shake" event (crossing a specific acceleration threshold on multiple axes) [cite: 346-347].


