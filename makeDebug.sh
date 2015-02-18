SCRIPTDIR="$( cd "$(dirname $0)" && pwd )"
rm -f $SCRIPTDIR/lib/Debug/*
rm -f $SCRIPTDIR/bin/Debug/homegear-addonlib
$SCRIPTDIR/premake4 gmake
cd $SCRIPTDIR
make config=debug
