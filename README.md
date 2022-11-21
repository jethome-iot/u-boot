# uboot % jethome-boot-fip

Firmware Image Pacakge (FIP) sources used to sign Amlogic u-boot binaries in JetHome images

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
python3 acs_tool.py bl2.bin /tmp/tmp.l9HlAIaFaN/bl2_acs.bin acs.bin 0
ACS tool process done.
./blx_fix.sh /tmp/tmp.l9HlAIaFaN/bl2_acs.bin /tmp/tmp.l9HlAIaFaN/zero_tmp /tmp/tmp.l9HlAIaFaN/bl2_zero.bin bl21.bin /tmp/tmp.l9HlAIaFaN/bl21_zero.bin /tmp/tmp.l9HlAIaFaN/bl2_new.bin bl2
1728+0 records in
1728+0 records out
1728 bytes (1.7 kB, 1.7 KiB) copied, 0.00608564 s, 284 kB/s
5752+0 records in
5752+0 records out
5752 bytes (5.8 kB, 5.6 KiB) copied, 0.0141005 s, 408 kB/s
./aml_encrypt_axg --bl2sig --input /tmp/tmp.l9HlAIaFaN/bl2_new.bin --output /tmp/tmp.l9HlAIaFaN/bl2.n.bin.sig
./blx_fix.sh bl30.bin /tmp/tmp.l9HlAIaFaN/zero_tmp /tmp/tmp.l9HlAIaFaN/bl30_zero.bin bl301.bin /tmp/tmp.l9HlAIaFaN/bl301_zero.bin /tmp/tmp.l9HlAIaFaN/bl30_new.bin bl30
1464+0 records in
1464+0 records out
1464 bytes (1.5 kB, 1.4 KiB) copied, 0.00757525 s, 193 kB/s
8980+0 records in
8980+0 records out
8980 bytes (9.0 kB, 8.8 KiB) copied, 0.0230315 s, 390 kB/s
./aml_encrypt_axg --bl3sig --input /tmp/tmp.l9HlAIaFaN/bl30_new.bin --output /tmp/tmp.l9HlAIaFaN/bl30_new.bin.enc  --level 3 --type bl30
./aml_encrypt_axg --bl3sig --input bl31.img --output /tmp/tmp.l9HlAIaFaN/bl31.img.enc --level 3 --type bl31
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
