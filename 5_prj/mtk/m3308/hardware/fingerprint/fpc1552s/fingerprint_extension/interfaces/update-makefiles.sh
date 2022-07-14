#!/bin/bash
# Run this script in the Android tree when changes has been made
# to any of the .hal files. This will update the makefiles and also
# generate dummy c++ implementations.

MY_PATH="`dirname \"$0\"`"

STUBS_LOC=/tmp/fingerprits-hidl-stubs

echo "*********************************************************************************************"
echo "*                                                                                           *"
echo "* This script will generate makefiles for the com.fingerprints.extension@1.0 HIDL interface *"
echo "* It will also generate stubs in $STUBS_LOC                                *"
echo "* that are useful if when a new interface has been added to any of the the .hidl files      *"
echo "*                                                                                           *"
echo "* This script should be executed if the path to the fingerprint delivery is changed.  I.e.  *"
echo "* not equal to \"vendor/fingerprints/\" or if i new interface has been added.               *"
echo "*                                                                                           *"
echo "* The script should be executed from the root of the Android tree after executing the lunch *"
echo "* command.                                                                                  *"
echo "*                                                                                           *"
echo "*********************************************************************************************"

if [ "$ANDROID_BUILD_TOP" == "" ]; then
    printf "\n\nERROR: Execute \". build/envsetup.sh && lunch <lunch-combo>\" before executing"
    printf "this script\n\n"
    exit -1
fi

source system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
    "com.fingerprints:$MY_PATH/" \
    "android.hardware:hardware/interfaces" \
    "android.hidl:system/libhidl/transport"


mkdir -p $STUBS_LOC
PACKAGE=com.fingerprints.extension@1.0
hidl-gen -o $STUBS_LOC -Lc++-impl -randroid.hardware:hardware/interfaces -randroid.hidl:system/libhidl/transport -r com.fingerprints:$MY_PATH/ $PACKAGE
hidl-gen -o $STUBS_LOC -Landroidbp-impl -randroid.hardware:hardware/interfaces -randroid.hidl:system/libhidl/transport -r com.fingerprints:$MY_PATH/ $PACKAGE
hidl-gen -o $STUBS_LOC -Ljava -randroid.hardware:hardware/interfaces -randroid.hidl:system/libhidl/transport -r com.fingerprints:$MY_PATH/ $PACKAGE
hidl-gen -L hash -randroid.hardware:hardware/interfaces -randroid.hidl:system/libhidl/transport -r com.fingerprints:$MY_PATH/ $PACKAGE
