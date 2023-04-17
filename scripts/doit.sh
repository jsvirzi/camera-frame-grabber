cd /mnt/data/jsvirzi/projects/camera-frame-grabber/build
v4l-mmap -d /dev/video3 -res 1600 1300 -fmt 59455247 -name ${1}

