# uboot % jethome-boot-fip

Firmware Image Pacakge (FIP) sources used to sign u-boot binaries in JetHome images

## U-Boot/BL33 Firmware Packaging

```
Usage: ./build-fip.sh: <board model> <bl33/u-boot.bin path> [output directory] [temporary directory]
```

Pass the board name and the bl33/U-boot payload to generate a bootable binary.

Example:

```
$ mkdir output
$ ./build-fip.sh jethub-j100 bl33.bin output
make: Entering directory 'jethub-j100'
./aml_encrypt_axg --bl3sig --input /path/to/u-boot/bl33.bin --output /tmp/tmp.l9HlAIaFaN/bl33.bin.enc --level 3 --type bl33 
./aml_encrypt_axg --bootmk --output /path/to/output/u-boot.bin --level v3 \
               --bl2 /tmp/tmp.l9HlAIaFaN/bl2.n.bin.sig --bl30 /tmp/tmp.l9HlAIaFaN/bl30_new.bin.enc \
               --bl31 /tmp/tmp.l9HlAIaFaN/bl31.img.enc --bl33 /tmp/tmp.l9HlAIaFaN/bl33.bin.enc \
               --level 3
make: Leaving directory 'jethub-j100'

$ ls output
u-boot.bin  u-boot.bin.sd.bin  u-boot.bin.usb.bl2  u-boot.bin.usb.tpl
```


System Requirements:
 - x86-64 Linux system
 - Python 3 (for GXBB, GXL & GXM boards only)
 - sh
 - make
 - readlink
 - mktemp
 - cat
 - dd

Open-source tools exist to replace the binary-only Amlogic tools:
 - https://github.com/afaerber/meson-tools (GXBB, GXL & GXM only)
 - https://github.com/repk/gxlimg (GXBB, GXL, GXM & AXG only)
 - https://github.com/angerman/meson64-tools (developed for G12B, should work on G12A & SM1)

Based on work by Neil Armstrong & Christian Hewitt
 - https://github.com/LibreELEC/amlogic-boot-fip
