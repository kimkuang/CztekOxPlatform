#!/bin/bash

#set -e

function versionToData(){
    input_version=$1
    input_version_array=(${input_version//./ })
    i=2
    input_version_data=0
    #echo '******'
    for var in ${input_version_array[@]}
    do
       #echo '---'
       #echo $var
       base=1
       for ((j=1;j<=i;j++));
       do
           base=$base*100
       done
       let input_version_data+=$var*$base
       #echo $input_version_data
       let i--
    done
    echo $input_version_data
}

function ifNeedUpgrade(){
    cur_version=$1
    dest_version=$2
    dest_version_data=$(versionToData $dest_version)
    #echo $dest_version_data 
    #echo '////'
    #echo $?
    cur_version_data=$(versionToData $cur_version)
    #echo $cur_version_data
 
    if [ $dest_version_data -gt $cur_version_data ]; then
        echo 1
    else
        echo 0
    fi 
}

function ifNeedUpgradeKernel(){
    cur_kernel_version=$(uname -v |awk '{print $1}')
    tmp=${cur_kernel_version:1:2}
    if [ $(($tmp)) -lt 32 ]; then
        echo 1
    else
        echo 0
    fi
}

#echo "Stopping camerad service..."
#pushd `pwd` >/dev/null
#cd /etc/init.d/
#sudo update-rc.d camerad remove -f
#popd >/dev/null

echo "checking czcmti running status..."
pid=`pidof czcmti || true`
if [ ! -z $pid ]; then
    echo "running app instance pid is: $pid"
    echo "killall czcmti..."
    killall czcmti || true  # turn off error checking
fi

echo "Removing old directories ..."
if [ -d "/home/ubuntu/bin/czcmti/" ]; then
        rm -rf /home/ubuntu/bin/czcmti/
fi

echo "Backup old directories ..."
if [ -d "/opt/czcmti/" ]; then
    echo "ubuntu" | sudo -S rm -rf /opt/czcmti_bak/
    echo "ubuntu" | sudo -S rm -rf /opt/czcmti/log/*
    echo "ubuntu" | sudo -S rm -rf /opt/czcmti/pic/*
    echo "ubuntu" | sudo -S rm -rf /opt/czcmti/*.raw
    echo "ubuntu" | sudo -S rm -rf /opt/czcmti/*.bmp
    
    echo "ubuntu" | sudo -S mkdir -p /opt/czcmti_bak/
    echo "ubuntu" | sudo -S cp -rf /opt/czcmti/*.ini /opt/czcmti_bak/
    echo "ubuntu" | sudo -S chown -R ubuntu:ubuntu /opt/czcmti_bak/
fi

echo "Removing project configure files ..."
if [ -d "/opt/czcmti/project/" ]; then
    echo "ubuntu" | sudo -S rm -rf /opt/czcmti/project/*
fi

echo "Prepare dirs..."
if [ ! -d "/opt/czcmti/" ]; then
        echo "ubuntu" | sudo -S mkdir -p /opt/czcmti/
fi
echo "ubuntu" | sudo -S chown -R ubuntu:ubuntu /opt/czcmti/

#rm -rf /opt/czcmti/*
mkdir -p /opt/czcmti/plugins/testitem/
mkdir -p /opt/czcmti/plugins/decoder/
mkdir -p /opt/czcmti/plugins/machine/

echo "Copy files..."
rm -rf /opt/czcmti/*.so
cp libImageSensor.so /opt/czcmti/
cp libOtpSensor.so /opt/czcmti/
cp libVcmDriver.so /opt/czcmti/
cp libUiUtils.so /opt/czcmti/
rm -f /opt/czcmti/czcmti
rm -rf /opt/czcmti/log/*
cp czcmti /opt/czcmti/
chmod u+x /opt/czcmti/czcmti
cp -f czcmti_*.qm /opt/czcmti/
rm -rf /opt/czcmti/plugins/testitem/*.so
cp plugins/testitem/*.so /opt/czcmti/plugins/testitem/
cp upgrade/testitem_3rd/*.so /opt/czcmti/plugins/testitem/
rm -rf /opt/czcmti/plugins/decoder/*.so
cp plugins/decoder/*.so /opt/czcmti/plugins/decoder/
rm -rf /opt/czcmti/plugins/machine/*.so
cp plugins/machine/*.so /opt/czcmti/plugins/machine/

echo "Upgrading resource..."
if [ -e czcmti.png ]; then
    cp czcmti.png /opt/czcmti/
fi
if [ -e upgrade/maintainer.png ]; then
    cp upgrade/maintainer.png /opt/maintainer/
fi
echo "Upgrading desktop icon..."
if [ -e czcmti.desktop ]; then
    echo "ubuntu" | sudo -S cp czcmti.desktop /usr/share/applications/
    echo "ubuntu" | sudo -S cp czcmti.desktop ~/Desktop/
    echo "ubuntu" | sudo -S chmod u+x ~/Desktop/czcmti.desktop
    echo "ubuntu" | sudo -S cp czcmti.desktop /usr/share/upstart/xdg/autostart/
fi

echo "Upgrading database..."
if [ -e system.db3 ]; then
        cp system.db3 /opt/czcmti/
fi

echo "Upgrading configurations..."
if [ -e czcmti.ini ]; then
        cp czcmti.ini /opt/czcmti/
fi
if [ -e machine.ini ]; then
        cp machine.ini /opt/czcmti/
fi
if [ -d project/ ]; then
        mkdir -p /opt/czcmti/project/
        cp -rf project/ /opt/czcmti/
fi

# additional patches
echo "Patching..."
if [ -e upgrade/libnetwork_trans.so ]; then
    cp upgrade/libnetwork_trans.so /opt/czcmti/
fi
if [ -e upgrade/smbclient ]; then
    echo "ubuntu" | sudo -S cp upgrade/smbclient /usr/bin/
fi
if [ -e upgrade/libCzUtils.so ]; then
	echo "ubuntu" | sudo -S cp upgrade/libCzUtils.so /usr/lib/
fi

if [ -e upgrade/Maintainer ]; then
    echo "Upgrading maintainer..."
    echo "ubuntu" | sudo -S mkdir -p /opt/maintainer/
    echo "ubuntu" | sudo -S chown ubuntu:ubuntu /opt/maintainer/
    cp -f upgrade/Maintainer /opt/maintainer/
    chmod u+x /opt/maintainer/Maintainer
    cp -f upgrade/Maintainer_zh_CN.qm /opt/maintainer/
    echo "ubuntu" | sudo -S cp -f upgrade/maintainer.desktop /usr/share/applications/
    echo "ubuntu" | sudo -S cp upgrade/maintainer.desktop ~/Desktop/
    echo "ubuntu" | sudo -S chmod u+x ~/Desktop/maintainer.desktop
fi

if [ -e /etc/xdg/autostart/nm-applet.desktop ]; then
    echo "Removing nm-applet.desktop..."
    echo "ubuntu" | sudo -S rm -f /etc/xdg/autostart/nm-applet.desktop
fi

if [ -e upgrade/libCmtiDal.so ]; then
    echo "Upgrading dal..."
    echo "ubuntu" | sudo -S cp -rf upgrade/libCmtiDal.so /usr/lib/
fi

#检查产品是5xx还是8xx，再拷贝对应的HAL文件
system_product_name=''
function GetSystemProduct(){
    if [ -f "/proc/device-tree/boardinfo" ]; then
        board_info=$(sed -n '1p' '/proc/device-tree/boardinfo')
        boardinfo_array=(${board_info//_/ })
        system_product_name=${boardinfo_array[0]}
    elif [ -f "/proc/device-tree/nvidia,dtsfilename" ]; then
        board_info=$(sed -n '1p' '/proc/device-tree/nvidia,dtsfilename')
        result=$(echo $board_info | grep "czcm500")
        if [[ "$result" != "" ]]; then
            system_product_name="czcm500"
        else
            result=$(echo $board_info | grep "czcm800")
            if [[ "$result" != "" ]]; then
                system_product_name="czcm800"
            fi
        fi
    fi
}
GetSystemProduct
if [ "$system_product_name" == "czcm500" ];then
    if [ -e upgrade/libCz5x8Hal_Ox.so ]; then
        echo "cp libCz5x8Hal_Ox..."
        echo "ubuntu" | sudo -S cp -rf upgrade/libCz5x8Hal_Ox.so /usr/lib/
        echo "ubuntu" | sudo -S rm -f /usr/lib/libHal_Ox.so
        echo "ubuntu" | sudo -S ln -s /usr/lib/libCz5x8Hal_Ox.so /usr/lib/libHal_Ox.so
        echo "ubuntu" | sudo -S chown ubuntu:ubuntu /usr/lib/libHal_Ox.so
    fi
elif [ "$system_product_name" == "czcm800" ];then
    if [ -e upgrade/libCz8x8Hal_Ox.so ]; then
        echo "cp libCz8x8Hal_Ox..."
        echo "ubuntu" | sudo -S cp -rf upgrade/libCz8x8Hal_Ox.so /usr/lib/
        echo "ubuntu" | sudo -S rm -f /usr/lib/libHal_Ox.so
        echo "ubuntu" | sudo -S ln -s /usr/lib/libCz8x8Hal_Ox.so /usr/lib/libHal_Ox.so
        echo "ubuntu" | sudo -S chown ubuntu:ubuntu /usr/lib/libHal_Ox.so
    fi
fi

# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
# BE AWARE of the version number of the following lines!
let rebootFlag=0

echo "Check file system version..."
version=$(cat /etc/cztek/version)
len=${#version}
mmdd=${version: -4}
mmdd=${mmdd/0/}
let len=len-4
curr_ver=${version:0:len}$mmdd
echo "file system version:" $curr_ver
if [ -e upgrade/system.conf ]; then
    let flag=$(ifNeedUpgrade $curr_ver 1.1.416)
    if [ $flag -gt 0 ]; then
        echo "Upgrading system.conf..."
        echo "ubuntu" | sudo -S cp -f upgrade/system.conf /etc/systemd/
        echo "ubuntu" | sudo -S systemctl daemon-reexec
        echo "ubuntu" | sudo -S chmod 666 /etc/cztek/version
        echo "ubuntu" | sudo -S echo "1.1.417" > /etc/cztek/version
    fi
fi

let need_upgrade_kernel=$(ifNeedUpgradeKernel)
if [ -e upgrade/Image ]; then
    if [ $need_upgrade_kernel -gt 0 ]; then
        echo "Upgrading kernel..."
        echo "ubuntu" | sudo -S cp -f /boot/Image /boot/Image_bak
        echo "ubuntu" | sudo -S cp -f upgrade/Image /boot/
        rebootFlag=1
    fi
fi

echo "Check dtb version..."
curr_ver=$(cat /proc/device-tree/sensor-ctrl/version)
echo "dtb version:"  $curr_ver
if [ -e upgrade/czcm500-A02.dtb ]; then
    let flag=$(ifNeedUpgrade $curr_ver 2.1.9)
    if [ $flag -gt 0 ]; then
        echo "Upgrading dtb..."
        echo "ubuntu" | sudo -S cp -rf upgrade/czcm500-A02.dtb /boot/
        rebootFlag=1
    fi
fi

echo "Check driver version..."
curr_ver=`modinfo dual_camera | grep -w version | awk '{print $2}'`
echo "driver version:"  $curr_ver
if [ -e upgrade/dual_camera.ko ]; then
    let flag=$(ifNeedUpgrade $curr_ver 1.5.4)
    if [ $flag -gt 0 ]; then
        echo "Upgrading drivers..."
        if [ $rebootFlag -eq 0 ]; then
            echo "Unloading the old driver..."
            echo "ubuntu" | sudo -S modprobe -r dual_camera
        fi
        echo "ubuntu" | sudo -S cp upgrade/dual_camera.ko /lib/modules/3.10.96/kernel/drivers/media/i2c/dualcamera/
        if [ $rebootFlag -eq 0 ]; then
            echo "Loading the new driver..."
            echo "ubuntu" | sudo -S modprobe dual_camera
        fi
    fi
fi

#增加恢复设置检查，
#1、检查桌面图标是否存在
if [ ! -f /usr/share/upstart/xdg/autostart/czrecovery.desktop ]; then
    echo "ubuntu" | sudo -S cp upgrade/czrecovery.desktop /usr/share/upstart/xdg/autostart/
    echo "ubuntu" | sudo -S cp upgrade/czrecovery.sh /usr/share/upstart/xdg/autostart/
    echo "ubuntu" | sudo -S chmod 755 /usr/share/upstart/xdg/autostart/czrecovery.desktop
    echo "ubuntu" | sudo -S chmod 755 /usr/share/upstart/xdg/autostart/czrecovery.sh
fi

if [ $rebootFlag -gt 0 ]; then
    echo "+++++++++++ Upgrader will RESTART computer now... +++++++++++"
    echo "ubuntu" | sudo -S reboot
fi
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

echo "Finished!!!"
exit 0
