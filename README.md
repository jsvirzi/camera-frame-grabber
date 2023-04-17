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
    chmod +x /mnt/data/jsvirzi/utils/root/bin/root
    chmod +x /mnt/data/jsvirzi/utils/root/bin/root-config
    source /mnt/data/jsvirzi/utils/root/bin/thisroot.sh
    echo "source /mnt/data/jsvirzi/utils/root/bin/thisroot.sh" >> ~/.bashrc 

## Install OpenCV on Xavier NX

    cd /mnt/data/jsvirzi/utils
    unzip opencv.zip
    unzip opencv_contrib.zip
    cd opencv-4.5.2
    mkdir build
    cd build
    cmake -DOPENCV_EXTRA_MODULES=/mnt/data/jsvirzi/utils/opencv_contrib-4.5.2/modules ..
    make
    sudo make install

## Install guvcview

    sudo apt-get install intltool
    sudo apt-get install libv4l-dev
    sudo apt-get install libudev-dev
    sudo apt-get install libavcodec-dev
    sudo apt-get install libsdl2-dev
    sudo apt-get install libgsl-dev
    sudo apt-get install portaudio19-dev

    tar xvf guvcview-src-2.0.8.tar.bz2
    cd guvcview-src-2.0.8
    ./configure
    make (-j4)
    sudo make install
    sudo ldconfig

    DBUS_FATAL_WARNINGS=0 guvcview -d /dev/video3


### GUVCVIEW -- problems with library resolution 
From experience, when running guvcview, it has been unable to resolve library locations.
If this happens, the following commands will help

    # completely optional steps. use only if problems occur with guvcview
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/mnt/data/jsvirzi/utils/guvcview-src-2.0.8/gview_v4l2core/.libs
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/mnt/data/jsvirzi/utils/guvcview-src-2.0.8/gview_render/.libs
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/mnt/data/jsvirzi/utils/guvcview-src-2.0.8/gview_audio/.libs
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/mnt/data/jsvirzi/utils/guvcview-src-2.0.8/gview_encoder/.libs
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/mnt/data/jsvirzi/utils/guvcview-src-2.0.8/guvcview/.libs

## Usage

### On generic linux machine with e-conSystem CU24G
    v4l-mmap -d /dev/video2 -res 1920 1080 -fmt 56595559 -name <session-name>

### On Xavier NX
    v4l-mmap -d /dev/video3 -res 1600 1300 -fmt 59455247 -name <session-name>
