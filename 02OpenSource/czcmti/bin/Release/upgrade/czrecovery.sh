# !/bin/bash

set -e

if [ ! -d "/home/ubuntu/Desktop" ]; then
    mkdir -p "/home/ubuntu/Desktop"
fi
if [ ! -f "/home/ubuntu/Desktop/czcmti.desktop" ]; then
    echo "ubuntu" | sudo -S cp /usr/share/upstart/xdg/autostart/czcmti.desktop  /home/ubuntu/Desktop/czcmti.desktop
    sudo chmod 755 /home/ubuntu/Desktop/czcmti.desktop
fi
if [ ! -f "/home/ubuntu/Desktop/maintainer.desktop" ]; then
    echo "ubuntu" | sudo -S cp /usr/share/applications/maintainer.desktop  /home/ubuntu/Desktop/maintainer.desktop
    sudo chmod 755 /home/ubuntu/Desktop/maintainer.desktop
fi
exit 0