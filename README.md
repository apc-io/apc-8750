# APC Uboot and Kernel


## u-boot

     export PATH=$PATH:$arm-toolchain/bin
     cd u-boot
     make wmt_config
     make all
     
## Kernel

     export PATH=$PATH:$arm-toolchain/bin
     make CROSS_COMPILE=arm-none-linux-gnueabi- clean
     make Android_defconfig
     make ubin CROSS_COMPILE=arm-none-linux-gnueabi-





