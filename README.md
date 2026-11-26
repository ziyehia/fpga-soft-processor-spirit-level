# Nios V FPGA lab · ADXL345 accelerometer

This ENSEA FPGA lab integrates a **Nios V/m** soft processor and an **ADXL345** accelerometer on the **DE10-Nano** (Cyclone V, `5CSEBA6U23I7`). The firmware prints signed X, Y, and Z acceleration samples over JTAG UART and uses the Y-axis sample to drive a ten-LED spirit level.

**Lab team** · Redwon Mohammad and Yehia Zeaiter

## Hardware

[The Platform Designer system](sopc/nios.qsys) connects the processor to on-chip RAM, JTAG UART, a 10-bit LED PIO, and an Avalon I²C master. [The VHDL top level](rtl/tp_nios_v.vhd) exposes the LED bus and drives SDA and SCL as open-drain signals. [The Quartus project](synt/tp_nios_v.qpf) uses the [board pin assignments](synt/tp_nios_v.qsf).

## Firmware and lab milestones

[The C firmware](soft/app/main.c) follows the acquisition code shown on page 13 of the lab report. It addresses the ADXL345 at `0x1D`, writes `0x08` to register `0x2D` to start measurement, and reads six bytes starting at `0x32`. It decodes three signed 16-bit little-endian samples, prints them, and waits 100 ms before the next read. Startup failures stop the application. A failed sample clears the LEDs before the next read attempt.

The spirit level restores the behavior shown on page 14. Nonnegative Y readings turn the LEDs off. Negative readings select one LED, moving across the array as the magnitude increases. The original formula gives index 10 at −260. The firmware caps the index at 9, so −260 and more negative readings select the last LED.

The LED chaser shown on pages 9–10 is available as a separate build. It lights the LEDs one at a time up to all ten, then turns them off in reverse order until only the first LED remains lit.

## Unfinished work

- **Télécran erase integration.** The lab report (p. 15) proposes adding a dedicated PIO output from the Nios V system to Télécran’s clear input. Implement gesture detection in the firmware, route the clear signal in VHDL, and test that the gesture clears the drawing once while ordinary tilting leaves it intact. The acceleration threshold and timing still need to be determined on hardware.
- **Board validation.** Generate a programming file with a valid Nios V/m IP license and test both firmware builds on the DE10-Nano. Check that tilting moves the lit LED across the array, that the endpoint selects the last LED, and that the chaser lights and releases the LEDs in order.

## Build and run

Use Quartus Prime Lite 24.1 with Cyclone V device support, the Nios V command-line tools, and the RISC-V compiler. Quartus needs a valid Nios V/m IP license to generate a programming file. From the repository root, generate the Platform Designer outputs, HAL board support package, and firmware.

```sh
qsys-generate sopc/nios.qsys --synthesis=VHDL
mkdir -p soft/bsp
niosv-bsp -c -t=hal -s=sopc/nios.sopcinfo soft/bsp/settings.bsp
niosv-app -a=soft/app -b=soft/bsp -s=soft/app/main.c
cmake -S soft/app -B soft/app/build
cmake --build soft/app/build
quartus_sh --flow compile synt/tp_nios_v
```

Once Quartus generates `synt/output_files/tp_nios_v.sof`, program it using Quartus Programmer. Download the firmware over JTAG and open the terminal.

```sh
niosv-download -g -r soft/app/build/app.elf
juart-terminal
```

To build the chaser after generating the BSP and application project, use a separate build directory and download its ELF over JTAG.

```sh
cmake -S soft/app -B soft/app/build/chaser -DCMAKE_C_FLAGS=-DLED_CHASER
cmake --build soft/app/build/chaser
niosv-download -g -r soft/app/build/chaser/app.elf
```

[Altera Nios V software build flow](https://docs.altera.com/r/docs/773196/current/an-978-nios-v-processor-migration-guidelines/software-component?contentId=Kep2FcErhF~Os7xLpMvD~w)

## Verification

A clean build regenerated the Platform Designer system and produced a RISC-V `app.elf` for each mode. Host checks covered signed decoding, the LED index bounds, the chaser sequence, and I²C error handling. The existing FPGA design passed Quartus synthesis and fitting in the earlier export check. Quartus reported that it could not generate a programming file with the available IP license, so board behavior remains unverified.
