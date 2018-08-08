#!/bin/bash
#usage:
#   1. mount_disk.sh mount user_name password src_path dest_path
#
#   e.g. mount_disk.sh mount "Administrator" "1234" //192.168.0.1/ShareDir /home/ubuntu/ShareDir
#
#   2. mount_disk.sh umount mounted_path
#   e.g. mount_disk.sh umount /home/ubuntu/ShareDir

set -e

if [ "$1" == "mount" ]; then
    echo "mount ..."
    mkdir -p -m 0777 $5
    echo "ubuntu" | sudo -S mount -t cifs -o username=$2,password=$3 $4 $5
elif [ "$1" == "umount" ]; then
    echo "ubuntu" | sudo -S umount -f $2
    echo "ubuntu" | sudo -S rm -rf $2
else
    exit 1
fi

exit 0
