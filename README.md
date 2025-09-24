# CMPE460 MSPM0
Welcome to CMPE-460: Interface and Digital Electronic (IDE), at the Rochester Insitute of Technology!

This repository contains the files you will need for to start developing for
the Texas Instruments **LP-MSPM0G3507** microcontroller development board.

***Please read this so you can setup your project correctly!***

## Repository Structure
| Directory | Description |
| --- | --- |
| `docs` | Board development documents. Such as official TI docs for the microcontroller. |
| `source` | MSP source code. Headers and startup files needed to flash to the board. |
| `lib` | Provided driver files required for the class. |
| `proj`[^3] | Where your project code will go! |

## Project Setup
### Keil [^1][^2]
As Keil uVision is a Windows program, the following instructions are specified
for Windows only.
> Note that these paths assume the project is created under a *new* directory
> within the `proj` sub-level
#### Device Setup
The following steps will setup the Keil IDE to build and flash to the MSPM0G3507 at a basic level.

0. In the Pack Installer expand All Devices -> Texas Instruments -> MSPM0G1X0X_G3X0X Series -> MSPM0G350X -> MSPM0G3507
1. Install the `TexasInstruments::MSPM0G1X0X_G3X0X_DFP` package
2. Create a new project in a *new* directory in the `proj` sub-level
3. Select the `MSPM0G3507` target board (Texas Instrments -> MSPM0G1X0X_G3X0X Series -> MSPM0G350X -> MSPM0G3507)
4. Add the `CMSIS CORE` component. There is no `Device` component at this time.
5. Open the "Target Options" menu (the wand).
6. On the **Target** tab, select compiler version 6 for the *ARM Compiler*.
7. On the **C/C++** tab, insert this define into the *Preprocessor Symbols* section: `__MSPM0G3507__`
8. Under the same tab, add the sources directory to the *Include Paths*: `..\..\source`
9. Go to the **Linker** tab and add the *Scatter File*, located here:
   `..\..\source\ti\devices\msp\m0p\linker_files\keil\mspm0g3507.sct`.
   (Its an ASM file, not a C file)
10. Go to the **Debug** tab and select the `CMSIS-DAP Debugger` from the
   dropdown.
11. There may be a PDSC flash error under this configuration. There are two
   options if this occurs.
   * Option 1: __Ignore it.__ If this occurs, click the *Settings* button next to
     the algorithm selection dropdown. Navigate to the **Pack** tab and
     deselect the "Enable" checkbox for the *Debug Description* section.
   * Option 2: __Fix it.__ Close Keil, then copy the file:
   `..\..source\ti\devices\msp\m0p\TexasInstruments.MSPM0G1X0X_G3X0X_DFP.pdsc` to
   `C:\Users\<USER>\AppData\Local\Arm\Packs\TexasInstruments\MSPM0G1X0X_G3X0X_DFP\<VERSION>`.
   Overwrite the .pdsc file located here. Reopen the Keil project. You may
   need to reselect the target **Device** so the pack description updates. Now
   the Pack *Debug Description* can be safely enabled.
      * Note: This is a **downgrade** to pack version 1.2.1

   These options can result in slightly different project configurations. Make
   sure setup is consistent between computers.
> At this time, TI had "fixed" this pack description issue, but never actually published it to the Keil Packs repository.
12. Click `OK` to close and save these options.
13. Click on the icon of 3 blocks next to the wand to open the "Manage Project
    Items" menu.
14. Under the current target and current source group add the startup file,
    located here:
    `..\..\source\ti\devices\msp\m0p\startup_system_files\keil\startup_mspm0g350x_uvision.s`

#### Driver Setup
These steps will demonstrate how to add the driver files from the `lib`[^4] directory.
1. Open the "Manage Project Items" menu (the 3 blocks).
2. Add a new group called `lib`.
3. With the `lib` group selected, click add files and navigate to the lib
   directory at the repository top level. Make sure the file type shows all
   files. Select (or create) the desired .c files and click `Add`. Then exit and click `OK`.
4. Open the "Target Options" menu.
5. Go to the **C/C++** tab, under the *Inlcude Paths*, add the following:
   `..\..\lib` to direct to the drivers. `..\..\source;..\..\lib` can be used
   to include both of the needed paths.
6. Once done, exit by clicking `OK`.

#### Inlcudes
At this point, both the board SDK files and the driver files developed here
should be able to be included in the new project. The includes should look
something like this (yours will differ):
```c
#include <ti/devices/msp/msp.h>
#include "uart.h"
```

[^1]: Steps performed using Keil uVision V5.31.0.0
[^2]: Developed using Keil Device Pack V1.3.1
[^3]: You can rename or move the `proj` directory, but remember if you do, the
    paths listed here may change.
[^4]: This directory can be subsituted for any other directory you might want.
    The steps are largely the same.

## Recommended Modules
Setup these specific modules for core functionality on the car. You will work
through this on the labs.

* GPIOA
* GPIOB
* UART0
* UART1
* I2C1
* TIMA0
* TIMA1
* TIMG0
* TIMG6
* TIMG12
* ADC0

### Car Pinout
This class has a car! You get to drive it.

Listed here are the current pinouts for the car. If you want your code to work
on the car, it should interact with these specific pins.

| Module | Label        | Pin Out  |
| ------ | -----------  | -------- |
| UART1  | HM-10 BLE RX | PA8      |
| UART1  | HM-10 BLE TX | PA9, SW1 |
| I2C1   | OLED SCL     | PB2      |
| I2C1   | OLED SDA     | PB3      |
| TIMA0  | Channel 0    | PB8      |
| TIMA0  | Channel 1    | PB12     |
| TIMA0  | Channel 2    | PB17     |
| TIMA0  | Channel 3    | PB13     |
| TIMA1  | Channel 0    | PB4      |
| TIMG0  | Channel 0    | PA12     |
| ADC0   | Channel 0    | PA27     |
| GPIOB  | Left DC EN   | PB19     |
| GPIOA  | Right DC EN  | PA22     |
| GPIOA  | Camera SI    | PA28     |

> Note: SW1 is a physical header on the board that must be moved to connect PA9 for
> UART1 functionality to work as expected.

### Board Clock Speed
A driver file is provided in the `lib` directory to allow changing the board
default clock speed at `sysctl`. The `SYSCTL_SYSCLK_set()` function will allow 
changing the clock speed to any of the predefined enum definitions. Getters are
also provided to retrieve the current operating speed. If the set function is not
called, the default clock speed is 32 MHz. The set function is only required
to initialize the clock speed, afterwards the board will operate at the
desired frequency.
> Do not edit this driver. Doing so will likely brick your board. Only call
> the functions as needed.

---

## Labs
Lab driver header files are given in the `lib` directory. These files provide
function prototypes with some explanation on what to implement. Additional files
will also be provided here to help with the labs and/or car, like `sysctl`.

These header files are to serve as a guide. You can modify the prototypes, or add
additional functionality as you see fit. Feel free to move the files around to fit
your project structure as well.

## Good Luck!
And that should be all! Good luck!
