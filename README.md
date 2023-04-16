# camera-frame-grabber

## Download the following links

    https://drive.google.com/file/d/1_jHPA54Ed0UzKlldly3K0BzjGQ7kuLZb/view?usp=sharing
    https://drive.google.com/file/d/1mnUD_cEt008iRPZwFIXA2LE1A9ZPd_VV/view?usp=sharing
    https://drive.google.com/file/d/1G2Y7eAVNNpMYOrbupcTYyPv1WPkhgjWZ/view?usp=sharing
    https://drive.google.com/file/d/1zqd9J9PXQ1_axQxz5VPGznXi2e9bPqtb/view?usp=sharing
    https://drive.google.com/drive/folders/1UdqkmWt-I_7NcyLrAxGH2Ll5svlGo-gV?usp=sharing
    https://drive.google.com/file/d/1YymOEJBrbkumLQ5XrP8r0VOTJq4FtcA3/view?usp=sharing

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

### Prebuilt Binaries

    cd /mnt/data/jsvirzi/utils
    tar xzvf ~/Downloads/root-image-xavier-nx.tar.gz
    source /mnt/data/jsvirzi/utils/root/bin/thisroot.sh 
    chmod +x /mnt/data/jsvirzi/utils/root/bin/root
    chmod +x /mnt/data/jsvirzi/utils/root/bin/root-config
    source /mnt/data/jsvirzi/utils/root/bin/thisroot.sh

## Install OpenCV on Xavier NX

    unzip opencv.zip
    unzip opencv_contrib.zip
    cd opencv-4.5.2
    mkdir build
    cd build
    cmake -DOPENCV_EXTRA_MODULES=/mnt/data/jsvirzi/utils/opencv_contrib-4.5.2/modules ..

## Usage

### On generic linux machine with e-conSystem CU24G
    v4l-mmap -d /dev/video2 -res 1920 1080 -fmt 56595559 -name <session-name>

### On Xavier NX
    v4l-mmap -d /dev/video3 -res 1600 1300 -fmt 59455247 -name <session-name>
