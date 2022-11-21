O ?= .
TMP ?= .
BL33 ?=
COMPRESS_LZ4 ?= 0

ifeq ($(COMPRESS_LZ4),0)
	BL33_ARGS :=
else
	BL33_ARGS :=--compress lz4
endif

.PHONY: clean distclean
.NOTPARALLEL: ${TMP}/bl33.bin.enc ${O}/u-boot.bin

all: ${O}/u-boot.bin

clean:
	rm -f ${TMP}/bl33.bin.enc

distclean: clean
	rm -f ${O}/u-boot.bin ${O}/u-boot.bin.sd.bin ${O}/u-boot.bin.usb.bl2 ${O}/u-boot.bin.usb.tpl


ifneq ($(GXLIMG_PATH),)

${TMP}/bl33.bin.enc: ${BL33}
	$(GXLIMG_PATH) -t bl3x -c ${BL33} ${TMP}/bl33.bin.enc ${BL33_ARGS}

${O}/u-boot.bin: bl2.n.bin.sig bl30_new.bin.enc bl31.img.enc bl33.bin.enc
	$(GXLIMG_PATH) -t fip \
	               --bl2 bl2.n.bin.sig --bl30 bl30_new.bin.enc \
		       --bl31 bl31.img.enc --bl33 ${TMP}/bl33.bin.enc \
		       ${O}/u-boot.bin
	dd if=${O}/u-boot.bin of=${O}/u-boot.bin.usb.tpl skip=49152 bs=1
	dd if=${O}/u-boot.bin of=${O}/u-boot.bin.usb.bl2 bs=49152 count=1
else

${TMP}/bl33.bin.enc: ${BL33}
	./aml_encrypt_gxl --bl3enc --input ${BL33} --output ${TMP}/bl33.bin.enc ${BL33_ARGS}

${O}/u-boot.bin: bl2.n.bin.sig /bl30_new.bin.enc bl31.img.enc ${TMP}/bl33.bin.enc
	./aml_encrypt_gxl --bootmk --output ${O}/u-boot.bin \
	               --bl2 bl2.n.bin.sig --bl30 bl30_new.bin.enc \
		       --bl31 bl31.img.enc --bl33 ${TMP}/bl33.bin.enc
endif
