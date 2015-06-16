#!/bin/bash
IMG_NAME=beagle_sd_working.img
IMG_DIR=../../../sth_deps
IMG_FILE=$IMG_DIR/$IMG_NAME
MNT_POINT=../../../mnt/

if [ -f $IMG_FILE ];
then
   echo ""
else
   echo "copying the default image"
   cp $IMG_DIR/beagle_sd.img $IMG_FILE
fi

if [ -d $MNT_POINT ];
then
   echo ""
else
   echo "creating the mount point"
   mkdir $MNT_POINT
fi

echo "creating the SD image"
sudo mount -o loop,offset=$[106496*512] $IMG_FILE $MNT_POINT
sudo cp ../../core/build/sth_beagleboard.fw.img $MNT_POINT/boot/uImage
sudo umount $MNT_POINT
