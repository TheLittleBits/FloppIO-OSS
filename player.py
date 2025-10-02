#  ______ _                  _____ ____  
# |  ____| |                |_   _/ __ \ 
# | |__  | | ___  _ __  _ __  | || |  | |
# |  __| | |/ _ \| '_ \| '_ \ | || |  | |
# | |    | | (_) | |_) | |_) || || |__| |
# |_|    |_|\___/| .__/| .__/_____\____/ 
#                | |   | |               
#                |_|   |_|               
#
#           Made by Cosmin Pirciu
#
#
# FDD, HDD and scanner music using MIDI Files

print('Loading modules... ', end='', flush = True)
import mido
import sys
import serial
from time import sleep
from termcolor import cprint
cprint('DONE', color = 'green', flush = True)

cprint('''
  ______ _                  _____ ____  
 |  ____| |                |_   _/ __ \\ 
 | |__  | | ___  _ __  _ __  | || |  | |
 |  __| | |/ _ \\| '_ \\| '_ \\ | || |  | |
 | |    | | (_) | |_) | |_) || || |__| |
 |_|    |_|\\___/| .__/| .__/_____\\____/ 
                | |   | |               
                |_|   |_|               
      ''', color = 'blue', flush = True)

port = None

# Opening the midi file with mido
print('Loading midi file... ', end = '', flush = True)

try:
    MidiFile = mido.MidiFile(sys.argv[1])
except IndexError:
    cprint('\n[FATAL] ', color = 'red', end = '', flush = True)
    print('Please specify the midi file.', flush = True)
    exit()
except OSError:
    cprint('\n[FATAL] ', color = 'red', end = '', flush = True)
    print('File not found.', flush = True)
    exit()

cprint('DONE\n', color = 'green', flush = True)

def cleanup(port):
    # Send All Notes Off message to all channels
    if port != None:
        for i in range(16):
            port.write(bytes([0b10110000 + i]))
            port.write(bytes([120]))
            port.write(bytes([0]))
            sleep(0.01)

def send_msg(port, msg):
    # Send a message to all picos
    port.write(bytes(msg.bytes()))

def main():
    global port
    # Open the serial port to the three picos
    port = serial.Serial('/dev/serial0', 31250, bytesize=8, parity='N', stopbits=1)

    for msg in MidiFile.play():
        send_msg(port, msg) # Sends the message to all picos

    print('\nDone playing file. Goodbye', flush = True)
    cleanup(port)
    exit()


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt: # In case of keyboard interruption
        cprint('Interrupted by user.', color = 'red', flush = True)
        print('Closing up.', flush = True)
        cleanup(port) # Sends "All Notes Off" message
        exit()
    except Exception as error: # In case of other errors
        cprint('\n[ERROR] ', color = 'red', end = '', flush = True)
        print(str(error), flush = True)
        print('Closing up.', flush = True)
        cleanup(port) # Sends "All Notes Off" message
        exit()
