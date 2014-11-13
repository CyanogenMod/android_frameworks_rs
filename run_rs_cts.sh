#!/bin/bash

# Run the general RS CTS tests. We can expand this script to run more tests
# as we see fit, but this essentially should be a reasonable smoke test of
# RenderScript (to be run on build/test bots).

CTS_TRADEFED=$ANDROID_BUILD_TOP/out/host/linux-x86/bin/cts-tradefed

#$CTS_TRADEFED run commandAndExit cts --force-abi 64 -p android.renderscript
#$CTS_TRADEFED run commandAndExit cts --force-abi 32 -p android.renderscript
$CTS_TRADEFED run commandAndExit cts -p android.renderscript
exit $?
