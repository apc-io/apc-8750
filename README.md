# APC Uboot and Kernel


## u-boot

     export PATH=$PATH:$TOOLS/arm-toolchain/bin
     cd $CURRENT_PATH/u-boot
     make wmt_config
     make all
     
## Kernel

     cd $CURRENT_PATH/kernel
     export PATH=$PATH:$TOOLS/arm-toolchain/bin
     make 8710_defconfig
     make ubin





