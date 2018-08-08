#!/bin/bash
if [ $# != 2 ]
then
	echo "Usage: `basename $0` debug|release -jN"
	exit 0
fi

MODE="Release"
if [ $1 == "debug" ]
then
	MODE="Debug"
fi

pushd ./libImageSensor
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then
	exit 1
fi

popd
pushd ./libOtpSensor
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then 
	exit 1
fi

popd
pushd ./libVcmDriver
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then
	exit 1
fi

popd
pushd ./ProtocolHandler
rm -rf ./lib/$MODE/*.so
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then
	exit 1
fi

popd
pushd ./SensorImageDecoder/DecodeToRgb24
rm -rf ../lib/$MODE/*.so
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then
	exit 1
fi

popd
pushd ./czcmti
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then
	exit 1
fi

popd
pushd ./CommonTestItem
rm -rf lib/$MODE/*.so
if [ -e Makefile ]; then
    make distclean 1> /dev/null
fi
qmake && make $1 $2 1> /dev/null
if [ $? != 0 ]
then
	exit 1
fi

popd
mkdir -p ./bin/$MODE/
rm -rf ./bin/$MODE/*.so
cp -rvf ./lib/$MODE/libOtpSensor.so ./bin/$MODE/
cp -rvf ./lib/$MODE/libImageSensor.so ./bin/$MODE/
cp -rvf ./lib/$MODE/libVcmDriver.so ./bin/$MODE/
cp -rvf ./lib/$MODE/libUiUtils.so ./bin/$MODE/
cp -rvf ./czcmti/czcmti.desktop ./bin/$MODE/
cp -rvf ./czcmti/czcmti.png ./bin/$MODE/
cp -rvf ./czcmti/czcmti_*.qm ./bin/$MODE/
cp -rvf ./czcmti/res/deploy.sh ./bin/$MODE/
chmod u+x ./bin/$MODE/deploy.sh
cp -rvf ./czcmti/system.db3 ./bin/$MODE/

#cp -rvf ./lib/$MODE/libHal_Ox.so ./bin/$MODE/upgrade/lib/
cp -rvf ./lib/$MODE/libCzUtils.so /usr/lib/
#cp -rvf ./lib/$MODE/dual_camera.ko ./bin/$MODE/upgrade/driver/

mkdir -p ./bin/$MODE/plugins/machine/
rm -rf ./bin/$MODE/plugins/machine/*.so
cp -rvf ./ProtocolHandler/lib/$MODE/*.so ./bin/$MODE/plugins/machine/

mkdir -p ./bin/$MODE/plugins/decoder/
rm -rf ./bin/$MODE/plugins/decoder/*.so
cp -rvf ./SensorImageDecoder/lib/$MODE/*.so ./bin/$MODE/plugins/decoder

mkdir -p ./bin/$MODE/plugins/testitem/
rm -rf ./bin/$MODE/plugins/testitem/*.so
cp -rvf ./CommonTestItem/lib/$MODE/*.so ./bin/$MODE/plugins/testitem

CURTIME=`date +%y%m%d%H%M%S`
OBJDIR="czcmti"
MAJOR_VERSION=`cat czcmti/common/Global.h | grep "MAJOR_VERSION" | awk '{ print $3 }'`
MINOR_VERSION=`cat czcmti/common/Global.h | grep "MINOR_VERSION" | awk '{ print $3 }'`
REVISION_VERSION=`cat czcmti/common/Global.h | grep "REVISION_VERSION" | awk '{ print $3 }'`
pushd ./bin
cp -rf $MODE $OBJDIR

if [ -d "./makeself-2.3.1/" ]; then
    echo "making self-running package ..."
    pushd ./makeself-2.3.1/
    chmod u+x *.sh
    ./makeself.sh ../$OBJDIR ../../../Release/czcmti-$1-$MAJOR_VERSION.$MINOR_VERSION.$REVISION_VERSION.$CURTIME.run "czcmti installer." ./deploy.sh
    popd
    printf "\033[01;32mOK...\n"
else
    echo "makeself-2.3.1 dir is not exist."
    printf "\033[01;31mNG...\n"
fi

rm -rf $OBJDIR
popd > /dev/null

exit 0
