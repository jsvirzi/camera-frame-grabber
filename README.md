# camera-frame-grabber

## Download the following links

    https://drive.google.com/file/d/1_jHPA54Ed0UzKlldly3K0BzjGQ7kuLZb/view?usp=sharing
    https://drive.google.com/file/d/1mnUD_cEt008iRPZwFIXA2LE1A9ZPd_VV/view?usp=sharing
    https://drive.google.com/file/d/1G2Y7eAVNNpMYOrbupcTYyPv1WPkhgjWZ/view?usp=sharing
    https://drive.google.com/file/d/1zqd9J9PXQ1_axQxz5VPGznXi2e9bPqtb/view?usp=sharing
    https://drive.google.com/drive/folders/1UdqkmWt-I_7NcyLrAxGH2Ll5svlGo-gV?usp=sharing

## Install ROOT 5.34.38

### Binaries
Prerequisite modules must be installed:

    sudo apt-get install apt-utils
    sudo apt-get install dpkg-dev cmake g++ gcc binutils 
    sudo apt-get install libx11-dev libxpm-dev libxft-dev libxext-dev 
    sudo apt-get install python libssl-dev 
    sudo apt-get install libgtk2.0-dev libcanberra-gtk-module

### Source Code

    cd /mnt/data/jsvirzi/utils
    tar xzvf root_v5.34.38.source.tar.gz
    cd root
    ./configure (or next line)
    ./configure linuxarm64 --prefix=/usr/local --build=aarch64-unknown-linux-gnu
    make
    source /mnt/data/jsvirzi/utils/root/bin/thisroot.sh

## Install OpenCV on Xavier NX

    unzip opencv.zip
    unzip opencv_contrib.zip
    cmake -DOPENCV_EXTRA_MODULES=/mnt/data/jsvirzi/utils/opencv_contrib-4.5.2/modules ..