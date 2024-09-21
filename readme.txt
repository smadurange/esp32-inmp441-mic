ESP32-S3 CONFIGURATION

  1. Rename the esp32/main/Kconfig.projbuild.example to 
     esp32/main/Kconfig.projbuild.example, and fill in the config values.
  2. Run the following command to change the build target to ESP32-S3.
     $ idf.py set-target esp32s3
  3. Run the following command to open the menuconfig program and update
     config values like the flash size and serial flash speed.
     $ idf.py menuconfig

Now you can build and flash the program like any other ESP32 program.

UDP SERVER

To start receving audio capture data from the MCU, use the recv.py start a UDP
server on a machine connected to the same access point as the MCU:

  $ python3 recv.py

The UDP server collects data up to ~30 seconds and saves as a wav file in the 
same directory.

