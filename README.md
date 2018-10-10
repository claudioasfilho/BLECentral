# BLE Central Client device

How it works:

This code was based on the following Knowledge Base Article from Silicon Labs:

https://www.silabs.com/community/wireless/bluetooth/knowledge-base.entry.html/2018/01/08/ble_central_and_gatt-7cNE

The major difference here is the Central Piece operates in a Host Processor (Posix based host) connected to a Silicon Labs EFR32MG13P632F512GM48 running a NCP Bluetooth image.

This is intended to be used with the BLE Device Server example (https://github.com/claudioasfilho/BLEDeviceServer). The Android App mentioned on the KBA also works.


 It operates as a Central Device and it will search for a Demo Service  with UUID df6a8b89-32d1-486d-943a-1a1f6b0b52ed (Available on the Client side).

Once found it will connect to it and subscribe for a Notification Service UUID      0ced7930-b31f-457d-a6a2-b3db9b03e39a and it will read and write to another Characteristic UUID fb958909-f26e-43a9-927c-7e17d8fb2d8d.

Tools and SDKs:

This was tested with:

Silicon Labs Bluetooth Stack 2.10.0.0

Gecko SDK Suite v2.4.0

GNU GCC Compiler: Apple LLVM version 9.0.0 (clang-900.0.39.2)
Target: x86_64-apple-darwin16.7.0
Thread model: posix


Development Board: Silicon Labs WSTK with BRD4159A (EFR32MG13P632F512GM48)

Usage:

In order to avoid modifying the Make File, please clone this to the following folder on the Bluetooth SDK structure.

/Applications/Simplicity Studio.app/Contents/Eclipse/developer/sdks/gecko_sdk_suite/v2.4/app/bluetooth/examples_ncp_host/

It should compile as is.

In order to execute it, please use the following command:

./exe/BLECentral /dev/tty.(Serial Port) 115200

If you desire to use this with a different Host, you will need to modify the Makefile in order to accommodate the cross compiler.

Target Device Images:

Both the NCP and Stand Alone Bootloader images can be found on the File Structure.

Stand Alone Bootloader - StandAlonebootloader-bgapi-MG13-combined.s37
Bluetooth NCP Image - ncp-MG13P.s37
