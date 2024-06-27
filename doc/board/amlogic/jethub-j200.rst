.. SPDX-License-Identifier: GPL-2.0+

U-Boot for JetHub D2 (J200)
===================================

JetHome Jethub D2 (http://jethome.ru/jethub-d2) is a home automation controller device
manufactured by JetHome with the following specifications:

 - Amlogic S905X3 (ARM Cortex-A55) quad-core up to 2.1GHz
 - micro-HDMI video out
 - 4Gb DDR4 RAM
 - 32GB eMMC flash
 - 1 x USB 2.0 Type-C
 - 1 x 10/100/1000Mbps ethernet
 - two module slots (JXM format)
 - 2 x gpio LEDS
 - GPIO user Button
 - DC source with a voltage of 9 to 36V (variable via power module)
 - DIN Rail Mounting case
 - 1 x 1-Wire
 - 1 x RS-485
 - 3 x dry contact digital GPIO inputs
 - 2 x relay GPIO outputs

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make jethub_j200_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh jethub-j200 /path/to/u-boot/u-boot.bin my-output-dir

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440
