#!/bin/bash

set -x

if [[ $1 -lt 1 ]]
then
	echo "Please provide a revision number."
	exit 0
fi
if test -z $2
then
  echo "Please specify branch."
  exit 0
fi

wget https://github.com/Homegear/HomegearAddonLib/archive/$2.zip
unzip $2.zip
rm $2.zip
version=$(head -n 1 HomegearAddonLib-$2/Version.h | cut -d " " -f3 | tr -d '"')
sourcePath=libhomegear-addon-$version
mv HomegearAddonLib-$2 $sourcePath
rm -Rf $sourcePath/.* 1>/dev/null 2>&2
rm -Rf $sourcePath/obj
rm -Rf $sourcePath/bin
rm -f $sourcePath/premake4*
tar -zcpf libhomegear-addon_$version.orig.tar.gz $sourcePath
cd $sourcePath
dch -v $version-$1 -M
debuild -us -uc
cd ..
rm -Rf $sourcePath
rm libhomegear-addon_$version-?_*.build
rm libhomegear-addon_$version-?_*.changes
rm libhomegear-addon_$version-?.debian.tar.gz
rm libhomegear-addon_$version-?.dsc
rm libhomegear-addon_$version.orig.tar.gz
