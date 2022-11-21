O ?= .
TMP ?= .
BL33 ?=

COMPRESS_LZ4 ?= 0

ifeq ($(COMPRESS_LZ4),0)
	BL33_ARGS :=
else
	BL33_ARGS :=--compress lz4
endif

AML_ENCRYPT ?= aml_encrypt_axg

.PHONY: clean distclean
.NOTPARALLEL: ${TMP}/bl33.bin.enc ${O}/u-boot.bin

all: ${O}/u-boot.bin

clean:
	rm -f ${TMP}/bl33.bin.enc

distclean: clean
	rm -f ${O}/u-boot.bin ${O}/u-boot.bin.sd.bin ${O}/u-boot.bin.usb.bl2 ${O}/u-boot.bin.usb.tpl

ifneq ($(GXLIMG_PATH),)

${TMP}/bl33.bin.enc: ${BL33}
	${GXLIMG_PATH} -t bl3x -s ${BL33} ${TMP}/bl33.bin.enc

${O}/u-boot.bin: bl2.n.bin.sig bl30_new.bin.enc bl31.img.enc ${TMP}/bl33.bin.enc
	${GXLIMG_PATH} -t fip --rev v3 \
		       --bl2 bl2.n.bin.sig --bl30 bl30_new.bin.enc \
		       --bl31 bl31.img.enc --bl33 ${TMP}/bl33.bin.enc \
		       ${O}/u-boot.bin;\
	dd if=${O}/u-boot.bin of=${O}/u-boot.bin.usb.tpl skip=49152 bs=1
	dd if=${O}/u-boot.bin of=${O}/u-boot.bin.usb.bl2 bs=49152 count=1

else

${TMP}/bl33.bin.enc: ${BL33}
	./${AML_ENCRYPT} --bl3sig --input ${BL33} --output ${TMP}/bl33.bin.enc --level 3 --type bl33 ${BL33_ARGS}

${O}/u-boot.bin: bl2.n.bin.sig bl30_new.bin.enc bl31.img.enc ${TMP}/bl33.bin.enc
	./${AML_ENCRYPT} --bootmk --output ${O}/u-boot.bin --level v3 \
		       --bl2 bl2.n.bin.sig --bl30 bl30_new.bin.enc \
		       --bl31 bl31.img.enc --bl33 ${TMP}/bl33.bin.enc \
		       --level 3
endif
