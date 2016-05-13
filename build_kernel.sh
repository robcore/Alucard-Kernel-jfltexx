#!/bin/bash
rm -rf $(pwd)/out;
rm $(pwd)/arch/arm/boot/dhd.ko;
rm $(pwd)/arch/arm/boot/scsi_wait_scan.ko;
rm $(pwd)/arch/arm/boot/zImage;
rm $(pwd)/arch/arm/boot/boot.img-zImage;
export KERNEL_CONFIG=alucard_defconfig;
export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/arm-cortex_a15-linux-gnueabihf_5.3/bin/arm-cortex_a15-linux-gnueabihf-
export ENABLE_GRAPHITE=true
make ARCH=arm CROSS_COMPILE=/opt/toolchains/arm-cortex_a15-linux-gnueabihf_5.3/bin/arm-cortex_a15-linux-gnueabihf- mrproper;
make distclean;
make clean && make mrproper;
export ARCH=arm
export CROSS_COMPILE=/opt/toolchains/arm-cortex_a15-linux-gnueabihf_5.3/bin/arm-cortex_a15-linux-gnueabihf-
export ENABLE_GRAPHITE=true
mkdir $(pwd)/out;
cp $(pwd)/arch/arm/configs/alucard_defconfig $(pwd)/out/.config;
make ARCH=arm -j7 O=$(pwd)/out oldconfig;
make ARCH=arm -S -s -j7 O=$(pwd)/out;
cp $(pwd)/out/arch/arm/boot/zImage $(pwd)/arch/arm/boot/zImage;
cp $(pwd)/out/drivers/net/wireless/bcmdhd/dhd.ko $(pwd)/arch/arm/boot/dhd.ko;
cp $(pwd)/out/drivers/scsi/scsi_wait_scan.ko $(pwd)/arch/arm/boot/scsi_wait_scan.ko;
mv $(pwd)/arch/arm/boot/zImage $(pwd)/arch/arm/boot/boot.img-zImage;
cd /media/root/robcore/AIK;
rm -rf S4-Machinex-8.x-new;
cp -R S4-Machinex-8.x S4-Machinex-8.x-new;
cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/dhd.ko $(pwd)/S4-Machinex-8.x-new/system/lib/modules/dhd.ko;
cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/scsi_wait_scan.ko $(pwd)/S4-Machinex-8.x-new/system/lib/modules/scsi_wait_scan.ko;
rm $(pwd)/split_img/boot.img-zImage;
cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/boot.img-zImage $(pwd)/split_img/boot.img-zImage;
rm image-new.img;
sh repackimg.sh;
cp image-new.img $(pwd)/S4-Machinex-8.x-new/boot.img
