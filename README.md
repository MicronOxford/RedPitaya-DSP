ActionTable runner for the Red Pitaya.

Building requires arm-linux-gnueabi-gcc, Python 2.7 and pip

Clone this repo and run 'make' to make the dsp, server, and download the os.
copy everything from build/ to the SD card.
The server will start up on ip 192.168.1.100 port 7000 on boot
logging output is available on the serial port (inner microusb port) or verbose
output in a log file on the root when the red pitaya is running. SSH into it to view
as the linux root is entirely volatile and is erased on reboot.

SSH: root@192.168.1.100, password root.

If connecting with cockpit fails with a timeout error, this is probably because
the versions of Pyro on the cockpit machine and the Red Pitaya do not match.
Update the Pyro version retreived in the Makefile to the one running on cockpit
and rebuild the SD card.
