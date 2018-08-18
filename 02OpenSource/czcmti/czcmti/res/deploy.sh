#!/bin/bash

function Init(){
    echo "checking czcmti running status..."
    pid=`pidof czcmti || true`
    if [ ! -z $pid ]; then
        echo "running app instance pid is: $pid"
        echo "killall czcmti..."
        killall czcmti || true  # turn off error checking
    fi

    if [ -d "/home/ubuntu/bin/czcmti/" ]; then
        echo "Removing old directories ..."
        rm -rf /home/ubuntu/bin/czcmti/
    fi

    if [ -d "/opt/czcmti/" ]; then
        echo "Backup old directories ..."
        echo "ubuntu" | sudo -S rm -rf /opt/czcmti_bak/
        echo "ubuntu" | sudo -S rm -rf /opt/czcmti/log/*
        echo "ubuntu" | sudo -S rm -rf /opt/czcmti/pic/*
        echo "ubuntu" | sudo -S rm -rf /opt/czcmti/*.raw
        echo "ubuntu" | sudo -S rm -rf /opt/czcmti/*.bmp
        echo "ubuntu" | sudo -S mkdir -p /opt/czcmti_bak/
        echo "ubuntu" | sudo -S cp -rf /opt/czcmti/*.ini /opt/czcmti_bak/
        echo "ubuntu" | sudo -S chown -R ubuntu:ubuntu /opt/czcmti_bak/
    fi

    if [ -d "/opt/czcmti/project/" ]; then
        echo "Removing project configure files ..."
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
}

function UpgradePlugins(){
    echo "Copy files..."
    rm -rf /opt/czcmti/*.so
    cp libImageSensor.so /opt/czcmti/
    cp libOtpSensor.so /opt/czcmti/
    cp libVcmDriver.so /opt/czcmti/
    cp libUiUtils.so /opt/czcmti/
    cp libCmtiDal.so /opt/czcmti/
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
}

function UpgradeCzcmtiConfig(){    
    if [ -e czcmti.ini ]; then
        echo "Upgrading czcmti.ini..."
        cp czcmti.ini /opt/czcmti/
    fi
    if [ -e machine.ini ]; then
        echo "Upgrading machine.ini..."
        cp machine.ini /opt/czcmti/
    fi
    #检查是否升级数据库
    if [ -e system.db3 ]; then
        echo "Upgrading database..."
        cp system.db3 /opt/czcmti/
    fi
    if [ -d project/ ]; then
        echo "Upgrading project configure files..."
        mkdir -p /opt/czcmti/project/
        cp -rf project/ /opt/czcmti/
    fi    
    if [ -e czcmti.png ]; then
        echo "Upgrading czcmti.png..."
        cp czcmti.png /opt/czcmti/
    fi
    if [ -e czcmti.desktop ]; then
        echo "Upgrading desktop icon..."
        echo "ubuntu" | sudo -S cp czcmti.desktop /usr/share/applications/
        echo "ubuntu" | sudo -S cp czcmti.desktop ~/Desktop/
        echo "ubuntu" | sudo -S chmod u+x ~/Desktop/czcmti.desktop
        echo "ubuntu" | sudo -S cp czcmti.desktop /usr/share/upstart/xdg/autostart/
    fi
}

#升级一些额外的补丁
function UpgradeAdditionPatches(){
    #检查是否升级网传模块
    if [ -e upgrade/libnetwork_trans.so ]; then
        echo "Upgrading network_trans..."
        cp upgrade/libnetwork_trans.so /opt/czcmti/
    fi
    #检查是否升级samba
    if [ -e upgrade/smbclient ]; then
        echo "Upgrading smbclient..."
        echo "ubuntu" | sudo -S cp upgrade/smbclient /usr/bin/
    fi
    #检查是否升级libCzUtils
    if [ -e upgrade/libCzUtils.so ]; then
        echo "Upgrading CzUtils..."
        echo "ubuntu" | sudo -S cp upgrade/libCzUtils.so /usr/lib/
    fi
    #检查是否升级libCmtiAlgorithm
    if [ -e upgrade/libCmtiAlgorithm.so ]; then
        echo "Upgrading CmtiAlgorithm..."
        echo "ubuntu" | sudo -S cp upgrade/libCmtiAlgorithm.so /usr/lib/
    fi
    if [ -e /usr/lib/libCmtiDal.so ]; then
        echo "Removing old CmtiDal..."
        echo "ubuntu" | sudo -S rm -rf /usr/lib/libCmtiDal.so
    fi
    #检查是否升级Maintainer
    if [ -e upgrade/Maintainer ]; then
        echo "Upgrading maintainer..."
        echo "ubuntu" | sudo -S mkdir -p /opt/maintainer/
        echo "ubuntu" | sudo -S chown ubuntu:ubuntu /opt/maintainer/
        cp -f upgrade/Maintainer /opt/maintainer/
        chmod u+x /opt/maintainer/Maintainer
        cp -f upgrade/Maintainer_zh_CN.qm /opt/maintainer/
        cp -f upgrade/maintainer.png /opt/maintainer/
        echo "ubuntu" | sudo -S cp -f upgrade/maintainer.desktop /usr/share/applications/
        echo "ubuntu" | sudo -S cp upgrade/maintainer.desktop ~/Desktop/
        echo "ubuntu" | sudo -S chmod 755 ~/Desktop/maintainer.desktop
    fi
}

#修正一些系统的设置
function ModifySystemSetting(){
    #检查是否删除nm-applet
    if [ -e /etc/xdg/autostart/nm-applet.desktop ]; then
        echo "Removing nm-applet.desktop..."
        echo "ubuntu" | sudo -S rm -f /etc/xdg/autostart/nm-applet.desktop
    fi

    #检查桌面图标是否存在
    if [ ! -f /usr/share/upstart/xdg/autostart/czrecovery.desktop ]; then
        echo "ubuntu" | sudo -S cp upgrade/czrecovery.desktop /usr/share/upstart/xdg/autostart/
        echo "ubuntu" | sudo -S cp upgrade/czrecovery.sh /usr/share/upstart/xdg/autostart/
        echo "ubuntu" | sudo -S chmod 755 /usr/share/upstart/xdg/autostart/czrecovery.desktop
        echo "ubuntu" | sudo -S chmod 755 /usr/share/upstart/xdg/autostart/czrecovery.sh
    fi
}

#+++++++++++++++++++++++++++++检查是否需要升级系统+++++++++++++++++++++++++++++++++
#版本号转化函数：版本号格式：1.1.1231，转化为1011231
function VersionStringToInt(){
    input_version=$1
    input_version_array=(${input_version//./ })
    #${input_version_array[2]/0/}
    #mmdd=$($input_version_array[2] | sed 's/^0.//g')
    mmdd=${input_version_array[2]/0/}
    input_version_data=$((${input_version_array[0]} * 1000000 + ${input_version_array[1]} * 10000 + ${mmdd}))
    echo $input_version_data
}

#获取当前系统参数函数集
#当前系统参数全局变量
system_kernel_version=0
system_product_name=''
system_platform=''
system_kernel_tree=''
system_driver_version=0
system_dtb_version=0
system_systemconf_version=0
system_hal_version=0
#目标参数全局变量，需要设置成固定值
target_kernel_version=0
target_driver_version=0
target_dtb_version=0
target_systemconf_version=0
target_hal_version=0
#内核和驱动的升级标志
isSystemUpgrade=0

function GetSystemProductPlatformKernelTree(){
    if [ -f "/proc/device-tree/boardinfo" ]; then
        board_info=$(sed -n '1p' '/proc/device-tree/boardinfo')
        boardinfo_array=(${board_info//_/ })
        system_product_name=${boardinfo_array[0]}
        system_platform=${boardinfo_array[1]}
        system_kernel_tree=${boardinfo_array[2]}
    elif [ -f "/proc/device-tree/nvidia,dtsfilename" ]; then
        board_info=$(sed -n '1p' '/proc/device-tree/nvidia,dtsfilename')
        result=$(echo $board_info | grep "czcm500")
        if [[ "$result" != "" ]]; then
            system_product_name="czcm500"
            system_platform="ox2"
            system_kernel_tree="2421"
        else
            result=$(echo $board_info | grep "czcm800")
            if [[ "$result" != "" ]]; then
                system_product_name="czcm800"
                system_platform="tg3"
                system_kernel_tree="281"
            fi
        fi
    fi
}

function GetSystemKernelVersion(){
    cur_kernel_version=$(uname -v |awk '{print $1}')
    system_kernel_version=${cur_kernel_version:1}
}

function GetSystemDriverVersion(){
    ret=$(modinfo dual_camera | grep -w version | awk '{print $2}')
    if [ "$ret" != "" ]; then
        system_driver_version=$(VersionStringToInt $ret)
    else
        ret=$(modinfo camera_control | grep -w version | awk '{print $2}')
        if [ "$ret" == "" ]; then
            system_driver_version=0
        else
            system_driver_version=$(VersionStringToInt $ret)
        fi

        if [ $system_driver_version -eq 0 ]; then
            driver_cmd="camera_control_"$system_platform"_"$system_kernel_tree
            ret=$(modinfo $driver_cmd | grep -w version | awk '{print $2}')
            if [ "$ret" == "" ]; then
                system_driver_version=0
            else
                system_driver_version=$(VersionStringToInt $ret)
            fi
        fi
    fi
}

function GetSystemDtbVersion(){
    if [ -f "/proc/device-tree/sensor-ctrl/version" ];then
        version=$(sed -n '1p' '/proc/device-tree/sensor-ctrl/version')
        system_dtb_version=$(VersionStringToInt $version)
    fi
}

#查询文件系统版本
function GetSystemSystemConfVersion(){
    if [ -f "/etc/cztek/version" ];then
        version=$(sed -n '1p' '/etc/cztek/version')
        system_systemconf_version=$(VersionStringToInt $version)
    fi
}

function GetSystemHalVersion(){
    result=`lmfgtool info 2>/dev/stdout`
    flag=0
    for line in $result
    do
        #先找到hal
        if [[ "$(echo $line | grep 'hal')" != "" ]]; then
            flag=1
        fi
        #再找Version
        if [ "$(echo $line | grep 'Version')" != "" -a $flag -eq 1 ]; then
            array=(${line//:/ })
            #Version:1.2.622
            system_hal_version=$(VersionStringToInt ${array[1]})
            break
        fi
    done
}

#获取当前系统各参数的总入口
function GetSystemParameters(){
    GetSystemProductPlatformKernelTree
    GetSystemKernelVersion
    GetSystemDriverVersion
    GetSystemDtbVersion
    GetSystemSystemConfVersion
    GetSystemHalVersion
    echo 'Current system version:'
    echo '      kernel    :' $system_kernel_version
    echo '      driver    :' $system_driver_version
    echo '      dtb       :' $system_dtb_version
    echo '      systemconf:' $system_systemconf_version
    echo '      hal       :' $system_hal_version
}

function GetTargetParameters(){
    if [ "$system_product_name" == "czcm500" ];then
        #内核版本：使用固定数值
        #版本号的转化：1.5.2 -> 1050002
        if [ "$system_kernel_tree" == "2421" ];then
            target_kernel_version=32
            target_driver_version=1050004
            target_dtb_version=2020002
            target_systemconf_version=1010611
            target_hal_version=1020416
        elif [ "$system_kernel_tree" == "282" ];then
            target_kernel_version=512
            target_driver_version=1050002
            target_dtb_version=2010009
            target_systemconf_version=1010416
            target_hal_version=1020622
        fi
    elif [ "$system_product_name" == "czcm800" ];then
        target_kernel_version=205
        target_driver_version=2050009
        target_dtb_version=1000013
        #8xx无system.conf文件，写0即可表示不升级
        target_systemconf_version=0
        target_hal_version=1020620
    fi
    echo 'Target version:'
    echo '      kernel    :' $target_kernel_version
    echo '      driver    :' $target_driver_version
    echo '      dtb       :' $target_dtb_version
    echo '      systemconf:' $target_systemconf_version
    echo '      hal       :' $target_hal_version
}

#检查是否要升级系统补丁
function CheckIfUpgradeSystemPatch(){
    GetSystemParameters
    GetTargetParameters
    echo "check result:"
    if [ $target_kernel_version -gt $system_kernel_version ];then
        isSystemUpgrade=1
        echo "      kernel: need upgrade"
    else
        echo "      kernel: not need upgrade"
    fi
    if [ $target_driver_version -gt $system_driver_version ];then
        isSystemUpgrade=1
        echo "      driver: need upgrade"
    else
        echo "      driver: not need upgrade"
    fi
    if [ $target_dtb_version -gt $system_dtb_version ];then
        isSystemUpgrade=1
        echo "      dtb: need upgrade"
    else
        echo "      dtb: not need upgrade"
    fi
    if [ $target_systemconf_version -gt $system_systemconf_version ];then
        isSystemUpgrade=1
        echo "      systemconf: need upgrade"
    else
        echo "      systemconf: not need upgrade"
    fi
    
    if [ $target_hal_version -gt $system_hal_version ];then
        isSystemUpgrade=1
        echo "      hal: need upgrade"
    else
        echo "      hal: not need upgrade"
    fi
}
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


CheckIfUpgradeSystemPatch
if [ $isSystemUpgrade -gt 0 ]; then
    echo "+++++++++++Please upgrade system patch first... +++++++++++"
    exit 0
fi
echo "+++++++++++Start Upgrading... +++++++++++"
Init
UpgradePlugins
UpgradeCzcmtiConfig
UpgradeAdditionPatches
ModifySystemSetting

echo "Finished!!!"
exit 0
