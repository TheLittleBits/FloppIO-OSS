# FloppIO-OSS: The Modular Floppy Music Player 💾🎶
Modular FDD, scanner and HDD music setup for Raspberry Pi

## Description
FloppIO is a midi-compatible floppy orchestra for Raspberry Pis. It uses three pico series microcontrollers to control 8 HDDs, 4 flatbed scanners and 8 FDDs. The Raspberry Pi computer sends commands and notes through a serial port to the Picos.

### Floppy Disk Drives
The first pico controls 8 FDDs. To output signals for multiple devices at the same time, each PIO state machine controls one FDD. A state machine is like a small CPU core that runs assembly code to control one or more gpio pins. This is a handy and unique thing in the Pico's RP2040 and RP2350 chips.
A floppy disk drive requires three signals to make music: STEP, DIR and ENABLE. The STEP signal moves the reader head one step, while DIR sets the direction. ENABLE starts the drive and turns on the small green led. The pico pulses the STEP pin at the right frequency while also setting the direction. When playing a note, the ENABLE pin is activated to turn on the led. Note: the ENABLE pin is actually called **Drive Select B**.


### Hard Disk Drives
It's nice to have some chords and melody instruments, but music without percussion is quite boring. Hard drives make a unique clicking sound when there's too much power in the actuator arm coil (which bangs to its stop). The second pico controls 8 drives with a dual H-bridge chip (see chapter Installation).

### Flatbed scanner
We need linear non-return movement for high-pitched sounds. Scanners are able to do that because of the large space between the start and end of the reader head. The third pico has outputs for 4 scanners. One scanner is controlled by one DRV8825, which needs STEP, DIR and ENABLE signals. The scanner code works in a similar way as the FDD program does, but the direction is managed by the second core. There are also two inputs (one for each scanner) that are used for endstop switches. When a signal of 3.3V is provided, the movement direction changes.


## Installation
### Setup #1
> [!WARNING]
> Bluetooth will be disabled after the setup. Save any content, because your pi reboots afterwards.
> `setup.sh` is only tested on Raspberry Pi 4. Completing the setup on older or
> newer devices is at your own risk.

>[!NOTE]
> The Raspberry Pi part is unnecessary, because you can connect any midi-compatible
> device to it. Just make sure to use the correct circuitry (optoisolator, resistors, etc) to get the pico ready
> for conenction to the UART port. Midi through the USB port is unsupported right now.

<ol>
	<li>Download the latest release (on your pi) and run the `setup.sh` script in order to install FloppIO.</li>
	<li>At the following dialog, go to:</li>
	<ol type="1">
		<li>Interface Options</li>
		<li>Serial Port</li>
		<li>Enable serial login: No</li>
		<li>Enable serial port hardware: Yes</li>
		<li>Ok</li>
		<li>If asked for reboot, answer no</li>
		<li>Finish</li>
	</ol>
  <li>After some setups, a text editor appears. Look in the open file for `dtoverlay=...`. If it's there, edit the value to `disable-bt`. Otherwise, manually add `dtoverlay=disable-bt` to the end of the file. Then look for `enable_uart=...`. Set the value to 1 and if `enable_uart` is not available, add `enable_uart=1` yourself. Press Ctrl-S and Ctrl-X to save and leave the editor.</li>
  <li>Your pi will reboot</li>
</ol>

### Setup #2
> [!WARNING]
> If your Raspberry Pi Pico is already flashed, any flashed content will be removed. Proceed at own risk.

Connect the first pico to your pi while pressing the on-board BOOTSEL button. If done correctly, the pico device should appear in file explorer.

Go to the parent directory and move `pico/floppy/floppy.elf` to your pico's mount folder. The pico will unmount itself and the code is loaded. Check if the green led is on after a second or two.

Now, disconnect the first pico and connect the second as shown above. Repeat the loading process with `pico/scanner/scanner.elf`
Then, disconnect the second and plug in the third. Flash it with the `pico/hdd/hdd.elf` binary.

Now your three picos are flashed. Connect the ground wires of all picos to the Raspberry Pi 4. Then connect GPIO 14 (from the pi) to GPIO 1 (from all three picos). If done correctly, you should now have the UART TX connected to the MIDI INs from the picos.

## Wiring and stuff
### Floppy Disk Drives
First of all, we need to power your floppy drives. You can buy berg connectors (the old connectors used to power FDDs) or you can manually put 12V and 5V on the "power connector" section of the image below. If you don't have a berg or a 12V and 5V power supply, you could consider using a common Molex 4-pin supply with jumper wires (they have the same wire order as berg connectors). Whatever you do, DON'T power the drives with 5V from your pi. The two picos should be powered through USB or VSYS, and there is no problem with connecting them at your pi.

<img height="100" alt="FDD_input" src="https://github.com/user-attachments/assets/5bc2715f-4902-454e-80cc-3cf02a589119" />


Each FDD uses its STEP, DIR and ENABLE pin of the pico. Use transistors (the cheap 2N2222 for example) on a breadboard to control the FDD pins:

<img height="328" alt="FDD" src="https://github.com/user-attachments/assets/e7c461ed-ab65-4669-97d8-2c344cc80a92" />

The breadboard inputs are connected to the pico using this pinout:

<img height="342" alt="pico_fdd" src="https://github.com/user-attachments/assets/3be8ba2d-e35f-4d94-84ad-1e229f1902fb" />

Connect the breadboard outputs to the floppy drive inputs using this diagram:
<img height="100" alt="FDD_input" src="https://github.com/user-attachments/assets/5bc2715f-4902-454e-80cc-3cf02a589119" />

You can use more drives if you add more transistors, wires and resistors. Each of them needs 3 transistors, so the images above only controls one drive.

### Flatbed Scanners
> [!NOTE]
> This section is optional. You may choose to only use the FDDs and ignore the scanner part without changing anything.

> [!NOTE]
> This tutorial shows how to use the scanner with DRV8825. You may choose to use A4988 instead, but this it not tested.

For using a scanner at this project, you need to disassemble it. There is little chance you can get your scanner running afterwards, so disassembling is at your own risk. You could go looking for an old scanner at secondhand shops, because new scanners are expensive and very silent. Plus, the stepper motor wires need to be visible. After finding the right scanner, locate the stepper motor wires (DRV8825 only supports motors with 4 wires) and connect them to the DRV8825 using this diagram:

<img height="342" alt="pico_scanner" src="https://github.com/user-attachments/assets/24e54e27-a002-4957-8f07-2b674f83a596" />

<br>
<br>

> [!NOTE]
> Now before proceeding to the pico setup, you need to trim the potentiometer according to the motors rated current. Just set up all everything as in the image above, WITHOUT the STEP and DIR pin connected. You must also leave the stepper disconnected. Power 3.3V to the SLP pin (in addition to the other 3V3 powered pins) and 12V to the bottom rail to activate the driver. Connect voltage meter to the GROUND and the little potentiometer to measure the VREF. Make sure VREF is under 0.1V, and if it doesn't work you can also increase the VREF (by turning the potentiometer). Afterwards, disconnect the SLP from the 3.3V supply, connect the stepper wires and continue.

Find a power source of 12 volts to power the breadboard's bottom power rail. The upper power rail only needs logic voltage of 3.3V (from the pico). Connect the orange wires to the pico's DRV8825-corresponding pins using the same pinout as above.

If you are using two scanners, use another DRV8825 with the above diagram. Only change the pico's output pins to 'SCANNER __2__ ...' instead of 'SCANNER __1__ ...'.


Next, use the image below to connect the two scanner endstops to the pico. Each button has to be glued or mounted on the opposite sides of the scanner. When the reader head hits a button, it changes the direction. Without endstops, your scanner won't do that and hit the edge. Be careful and test multiple times if this part is set up correctly.

<img height="400" alt="Endstops" src="https://github.com/user-attachments/assets/e2b3b72a-f2f2-43e6-bb73-5ce18ba01307" />

Your scanners should now be up and running.

### Hard Disk Drives
> [!NOTE]
> This section is optional. You may choose to only use the FDDs and ignore the HDD part without changing anything.

Connecting hard drives to the FloppIO network is more complicated than adding scanners or floppy drives. You need to solder the HDD coil ends to a wire yourself, which destroys the drive and all the data on it. Proceed at your own risk.

Locate the drive's coil ends and test them with a 3.3V power source. If the coil reader arm moves, you are fine. If it moves but not with a _snap_ sound, apply more voltage. Use the image below to connect the wires you just found to the breadboard:

<img height="260" alt="HDD" src="https://github.com/user-attachments/assets/3c9fe2fa-0ef0-4918-873a-aa4c491d9230" />

The L293D is a dual H-bridge IC, which does a perfect job for this setup. Power the top rail with pico's logic voltage (3.3V) and the bottom rail with the voltage for your drives. One L293D chip controls 2 HDDs, so you need four of them for 8 drives. The orange wires go to the HDD-corresponding pins of the third pico:

<img height="342" alt="pico_hdd" src="https://github.com/user-attachments/assets/67a9d012-f4df-460d-939f-6db9b6c2cf9e" />

To play a song, turn on all power supplies and check your connections. If the pico's onboard leds are on, the picos are functioning.
Now you can run `python3 player.py YOURMIDIFILE` on your pi. Don't forget the `midi/` directory with working midi examples!

## Features
Current features include:
 - Midi-compatibility: All programs are midi-compatible. That means it uses the same communication protocol as your midi keyboard or synthesizer. This allows pitchwheel effects and easier future development.
 - Power-saving mode: The HDD coils aren't always powered. Only at click they move, which is very power efficient.
 - Reset at startup: The floppy disk drives reset to their middle position at startup, which makes music sound even better.
 - Modular and easily expandable: Because of its modular nature, expanding is very easy. You can also choose to let one pico away.