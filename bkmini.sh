#!/bin/bash
cp $(pwd)/out/arch/arm/boot/zImage $(pwd)/arch/arm/boot/zImage;
cp $(pwd)/out/drivers/net/wireless/bcmdhd/dhd.ko $(pwd)/arch/arm/boot/dhd.ko;
cp $(pwd)/out/drivers/scsi/scsi_wait_scan.ko $(pwd)/arch/arm/boot/scsi_wait_scan.ko;
cp $(pwd)/out/drivers/gud/mcdrvmodule.ko $(pwd)/arch/arm/boot/mcdrvmodule.ko
cp $(pwd)/out/drivers/gud/mckernelapi.ko $(pwd)/arch/arm/boot/mckernelapi.ko
cp $(pwd)/out/drivers/crypto/msm/qce40.ko $(pwd)/arch/arm/boot/qce40.ko
cp $(pwd)/out/drivers/crypto/msm/qcedev.ko $(pwd)/arch/arm/boot/qcedev.ko
cp $(pwd)/out/drivers/crypto/msm/qcrypto.ko $(pwd)/arch/arm/boot/qcrypto.ko
cp $(pwd)/out/drivers/usb/serial/usb_wwan.ko $(pwd)/arch/arm/boot/usb_wwan.ko
cp $(pwd)/out/drivers/usb/serial/option.ko $(pwd)/arch/arm/boot/option.ko
cp $(pwd)/out/drivers/spi/spidev.ko $(pwd)/arch/arm/boot/spidev.ko
cp $(pwd)/out/drivers/media/video/gspca/gspca_main.ko $(pwd)/arch/arm/boot/gspca_main.ko
cp $(pwd)/out/crypto/ansi_cprng.ko $(pwd)/arch/arm/boot/ansi_cprng.ko
cp $(pwd)/out/arch/arm/mach-msm/reset_modem.ko $(pwd)/arch/arm/boot/reset_modem.ko
cp $(pwd)/out/fs/cifs/cifs.ko $(pwd)/arch/arm/boot/cifs.ko
mv $(pwd)/arch/arm/boot/zImage $(pwd)/arch/arm/boot/boot.img-zImage;
cd /media/root/robcore/AIK;
rm -rf S4-Machinex-8.x-new;
cp -R S4-Machinex-8.x S4-Machinex-8.x-new;
#cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/dhd.ko $(pwd)/S4-Machinex-8.x-new/system/lib/modules/dhd.ko;
#cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/scsi_wait_scan.ko $(pwd)/S4-Machinex-8.x-new/system/lib/modules/scsi_wait_scan.ko;
cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/*.ko $(pwd)/S4-Machinex-8.x-new/system/lib/modules/;
rm $(pwd)/split_img/boot.img-zImage;
cp /media/root/robcore/android/Alucard-Kernel-jfltexx-my-tw-5.0/arch/arm/boot/boot.img-zImage $(pwd)/split_img/boot.img-zImage;
rm image-new.img;
sh repackimg.sh;
cp image-new.img $(pwd)/S4-Machinex-8.x-new/boot.img
