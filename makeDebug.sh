SCRIPTDIR="$( cd "$(dirname $0)" && pwd )"
rm -f $SCRIPTDIR/lib/Debug/*
rm -f $SCRIPTDIR/bin/Debug/*
$SCRIPTDIR/premake4 gmake
cd $SCRIPTDIR
make config=debug
cp $SCRIPTDIR/bin/Debug/libhomegear-addon.so /usr/lib/libhomegear-addon.so.0
mkdir -p /usr/include/homegear-addon
mkdir -p /usr/include/homegear-addon/Encoding
mkdir -p /usr/include/homegear-addon/HelperFunctions
cp $SCRIPTDIR/*.h /usr/include/homegear-addon
cp $SCRIPTDIR/Encoding/*.h /usr/include/homegear-addon/Encoding
cp $SCRIPTDIR/HelperFunctions/*.h /usr/include/homegear-addon/HelperFunctions
