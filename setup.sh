sudo raspi-config
sudo apt update
sudo apt upgrade -y
sudo apt install python3 python3-serial python3-mido python3-rtmidi python3-termcolor -y
sudo systemctl disable hciuart
mkdir floppio
cp *.py floppio/
mkdir floppio/midi
cp example-midi/*.mid floppio/midi/
cp pico/floppy/floppy.elf floppio/
cp pico/scanner/scanner.elf floppio/
cp pico/hdd/hdd.elf floppio/
sudo nano /boot/firmware/config.txt
sudo reboot
