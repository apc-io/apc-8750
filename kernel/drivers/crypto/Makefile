obj-$(CONFIG_CRYPTO_DEV_PADLOCK_AES) += padlock-aes.o
obj-$(CONFIG_CRYPTO_DEV_PADLOCK_SHA) += padlock-sha.o
obj-$(CONFIG_CRYPTO_DEV_GEODE) += geode-aes.o
obj-$(CONFIG_CRYPTO_DEV_HIFN_795X) += hifn_795x.o
obj-$(CONFIG_CRYPTO_DEV_MV_CESA) += mv_cesa.o
obj-$(CONFIG_CRYPTO_DEV_TALITOS) += talitos.o
obj-$(CONFIG_CRYPTO_DEV_IXP4XX) += ixp4xx_crypto.o
obj-$(CONFIG_CRYPTO_DEV_PPC4XX) += amcc/

#
# need add ./Makefile, and $(CHECKFLAGS) += $(CRYPTO_DEFINES)
#

ifeq ($(CONFIG_CRYPTO_DEV_WMT),y)
ifeq ($(CONFIG_CRYPTO_DEV_WMT_AES),y)
CRYPTO_DEFINES += -DCONFIG_CRYPTO_DEV_WMT_AES
endif
endif

#
# Makefile for the wmt cipher device drivers.
#
# obj-$(CONFIG_CRYPTO_DEV_WMT) += wmt-cipher.o wmt-cipher-core.o
g_crypto-objs                := wmt-cipher.o wmt-cipher-core.o
obj-$(CONFIG_CRYPTO_DEV_WMT) += g_crypto.o
