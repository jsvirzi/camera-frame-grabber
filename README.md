# camera-frame-grabber

## Download the following links into the Downloads directory

    https://drive.google.com/file/d/1_jHPA54Ed0UzKlldly3K0BzjGQ7kuLZb/view?usp=sharing
    https://drive.google.com/file/d/1mnUD_cEt008iRPZwFIXA2LE1A9ZPd_VV/view?usp=sharing
    https://drive.google.com/file/d/1G2Y7eAVNNpMYOrbupcTYyPv1WPkhgjWZ/view?usp=sharing
    https://drive.google.com/file/d/1zqd9J9PXQ1_axQxz5VPGznXi2e9bPqtb/view?usp=sharing
    # https://drive.google.com/drive/folders/1UdqkmWt-I_7NcyLrAxGH2Ll5svlGo-gV?usp=sharing
    https://drive.google.com/file/d/1YymOEJBrbkumLQ5XrP8r0VOTJq4FtcA3/view?usp=sharing

## Command Line using wget and/or curl
    cd ~/Downloads
    
    # TODO this stopped working
    # wget -O root_v5.34.38.source.tar.gz https://drive.google.com/file/d/1_jHPA54Ed0UzKlldly3K0BzjGQ7kuLZb/view?usp=sharing
    # wget -O opencv.zip https://drive.google.com/file/d/1mnUD_cEt008iRPZwFIXA2LE1A9ZPd_VV/view?usp=sharing
    # wget -O opencv_contrib.zip https://drive.google.com/file/d/1G2Y7eAVNNpMYOrbupcTYyPv1WPkhgjWZ/view?usp=sharing
    # wget -O guvcview-src-2.0.8.tar.bz2 https://drive.google.com/file/d/1zqd9J9PXQ1_axQxz5VPGznXi2e9bPqtb/view?usp=sharing
    # wget -O root-image-xavier-nx.tar.gz https://drive.google.com/file/d/1YymOEJBrbkumLQ5XrP8r0VOTJq4FtcA3/view?usp=sharing

    # TODO this stopped working
    # wget -O root_v5.34.38.source.tar.gz https://drive.google.com/file/d/1_jHPA54Ed0UzKlldly3K0BzjGQ7kuLZb
    # wget -O opencv.zip https://drive.google.com/file/d/1mnUD_cEt008iRPZwFIXA2LE1A9ZPd_VV
    # wget -O opencv_contrib.zip https://drive.google.com/file/d/1G2Y7eAVNNpMYOrbupcTYyPv1WPkhgjWZ
    # wget -O guvcview-src-2.0.8.tar.bz2 https://drive.google.com/file/d/1zqd9J9PXQ1_axQxz5VPGznXi2e9bPqtb
    # wget -O root-image-xavier-nx.tar.gz https://drive.google.com/file/d/1YymOEJBrbkumLQ5XrP8r0VOTJq4FtcA3

### suggest to copy and paste into a bash script and execute

    #!/bin/bash
    
    cd ~/Downloads
    
    fileid="1_jHPA54Ed0UzKlldly3K0BzjGQ7kuLZb"
    filename="root_v5.34.38.source.tar.gz"
    html=`curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}"`
    curl -Lb ./cookie "https://drive.google.com/uc?export=download&`echo ${html}|grep -Po '(confirm=[a-zA-Z0-9\-_]+)'`&id=${fileid}" -o ${filename}

    fileid="1mnUD_cEt008iRPZwFIXA2LE1A9ZPd_VV"
    filename="opencv.zip"
    html=`curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}"`
    curl -Lb ./cookie "https://drive.google.com/uc?export=download&`echo ${html}|grep -Po '(confirm=[a-zA-Z0-9\-_]+)'`&id=${fileid}" -o ${filename}

    fileid="1G2Y7eAVNNpMYOrbupcTYyPv1WPkhgjWZ"
    filename="opencv_contrib.zip"
    html=`curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}"`
    curl -Lb ./cookie "https://drive.google.com/uc?export=download&`echo ${html}|grep -Po '(confirm=[a-zA-Z0-9\-_]+)'`&id=${fileid}" -o ${filename}

    fileid="1zqd9J9PXQ1_axQxz5VPGznXi2e9bPqtb"
    filename="guvcview-src-2.0.8.tar.bz2"
    html=`curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}"`
    curl -Lb ./cookie "https://drive.google.com/uc?export=download&`echo ${html}|grep -Po '(confirm=[a-zA-Z0-9\-_]+)'`&id=${fileid}" -o ${filename}

    fileid="1YymOEJBrbkumLQ5XrP8r0VOTJq4FtcA3"
    filename="root-image-xavier-nx.tar.gz"
    html=`curl -c ./cookie -s -L "https://drive.google.com/uc?export=download&id=${fileid}"`
    curl -Lb ./cookie "https://drive.google.com/uc?export=download&`echo ${html}|grep -Po '(confirm=[a-zA-Z0-9\-_]+)'`&id=${fileid}" -o ${filename}

## Preparation for NX

    mkdir -p /mnt/data/jsvirzi/projects
    mkdir -p /mnt/data/jsvirzi/utils

## Install ROOT 5.34.38

### Binaries
Prerequisite modules must be installed:

    sudo apt-get install -y apt-utils
    sudo apt-get install -y dpkg-dev cmake g++ gcc binutils 
    sudo apt-get install -y libx11-dev libxpm-dev libxft-dev libxext-dev 
    sudo apt-get install -y python libssl-dev 
    sudo apt-get install -y libgtk2.0-dev libcanberra-gtk-module

### Source Code

    cd /mnt/data/jsvirzi/utils
    tar xzvf ~/Downloads/root_v5.34.38.source.tar.gz
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
    unzip ~/Downloads/opencv.zip
    unzip ~/Downloads/opencv_contrib.zip
    cd opencv-4.5.2
    mkdir build
    cd build
    cmake -DOPENCV_EXTRA_MODULES=/mnt/data/jsvirzi/utils/opencv_contrib-4.5.2/modules ..
    make
    sudo make install

## Install guvcview

    sudo apt-get install -y intltool
    sudo apt-get install -y libv4l-dev
    sudo apt-get install -y libudev-dev
    sudo apt-get install -y libavcodec-dev
    sudo apt-get install -y libsdl2-dev
    sudo apt-get install -y libgsl-dev
    sudo apt-get install -y portaudio19-dev

    cd /mnt/data/jsvirzi/utils
    tar xvf ~/Downloads/guvcview-src-2.0.8.tar.bz2
    cd guvcview-src-2.0.8
    ./configure
    make
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

##  Compile/Build V4L Camera Frame Grabber + Utilities

    mkdir -p /mnt/data/jsvirzi/projects
    cd /mnt/data/jsvirzi/projects
    git clone git@github.com:jsvirzi/camera-frame-grabber.git
    cd camera-frame-grabber
    mkdir -p build
    cd build
    cmake ..
    make

## Update source in existing repository

    cd -p /mnt/data/jsvirzi/projects/camera-frame-grabber/build
    git pull
    make

## Usage

### On generic linux machine with e-conSystem CU24G
    cd /mnt/data/jsvirzi/projects/camera-frame-grabber/build
    ./v4l-mmap -d /dev/video2 -res 1920 1080 -fmt 56595559 -name <session-name>

### On Xavier NX

#### LPR Day Camera
    cd /mnt/data/jsvirzi/projects/camera-frame-grabber/build
    ./v4l-mmap -d /dev/video3 -res 1600 1300 -fmt 59455247 -name <session-name>

#### LPR Night Camera
    cd /mnt/data/jsvirzi/projects/camera-frame-grabber/build
    ./v4l-mmap -d /dev/videoX -res 1600 1300 -fmt 59455247 -name <session-name>

#### Context Camera
    cd /mnt/data/jsvirzi/projects/camera-frame-grabber/build
    ./v4l-mmap -d /dev/video2 -res 1920 1080 -fmt 56595559 -name <session-name>
