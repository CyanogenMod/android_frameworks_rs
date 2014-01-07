#!/bin/bash

# We are currently in frameworks/rs, so compute our top-level directory.
MY_ANDROID_DIR=$PWD/../../
cd $MY_ANDROID_DIR

# ANDROID_HOST_OUT is where the new prebuilts will be constructed/copied from.
ANDROID_HOST_OUT=$MY_ANDROID_DIR/out/host/linux-x86/

# HOST_LIB_DIR allows us to pick up the built librsrt_*.bc libraries.
HOST_LIB_DIR=$ANDROID_HOST_OUT/lib

# PREBUILTS_DIR is where we want to copy our new files to.
PREBUILTS_DIR=$MY_ANDROID_DIR/prebuilts/sdk/

# Target architectures and their system library names.
TARGETS=(arm mips x86)
SYS_NAMES=(generic generic_mips generic_x86)

print_usage() {
  echo "USAGE: $0 [-h|--help] [-n|--no-build] [-x]"
  echo "OPTIONS:"
  echo "    -h, --help     : Display this help message."
  echo "    -n, --no-build : Skip the build step and just copy files."
  echo "    -x             : Display commands before they are executed."
}

build_rs_libs() {
  echo Building for target $1
  lunch $1
  # Build the RS runtime libraries.
  cd $MY_ANDROID_DIR/frameworks/rs/driver/runtime && mma -j32 && cd - || exit 1
  # Build a sample support application to ensure that all the pieces are up to date.
  cd $MY_ANDROID_DIR/frameworks/rs/java/tests/RSTest_CompatLib/ && mma -j32 && cd - || exit 2
}

# Build everything by default
build_rs=1

while [ $# -gt 0 ]; do
  case "$1" in
    -h|--help)
      print_usage
      exit 0
      ;;
    -n|--no-build)
      build_rs=0
      ;;
    -x)
      # set lets us enable bash -x mode.
      set -x
      ;;
    *)
      echo Unknown argument: "$1"
      print_usage
      exit 99
      break
      ;;
  esac
  shift
done

if [ $build_rs -eq 1 ]; then

  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  echo !!! BUILDING RS PREBUILTS !!!
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  source build/envsetup.sh

  for t in ${TARGETS[@]}; do
    build_rs_libs aosp_${t}-userdebug
  done

  echo DONE BUILDING RS PREBUILTS

else

  echo SKIPPING BUILD OF RS PREBUILTS

fi

DATE=`date +%Y%m%d`

cd $PREBUILTS_DIR || exit 3
repo start pb_$DATE .

for i in $(seq 0 $((${#TARGETS[@]} - 1))); do
  t=${TARGETS[$i]}
  sys_lib_dir=$MY_ANDROID_DIR/out/target/product/${SYS_NAMES[$i]}/system/lib
  for a in `find renderscript/lib/$t -name \*.so`; do
    file=`basename $a`
    cp `find $sys_lib_dir -name $file` $a || exit 4
  done

  for a in `find renderscript/lib/$t -name \*.bc`; do
    file=`basename $a`
    cp `find $HOST_LIB_DIR $sys_lib_dir -name $file` $a || exit 5
  done
done

# general
# javalib.jar
cp $MY_ANDROID_DIR/out/target/common/obj/JAVA_LIBRARIES/android-support-v8-renderscript_intermediates/javalib.jar renderscript/lib

# Copy header files for compilers
cp $MY_ANDROID_DIR/external/clang/lib/Headers/*.h renderscript/clang-include
cp $MY_ANDROID_DIR/frameworks/rs/scriptc/* renderscript/include


# Linux-specific tools (bin/ and lib/)
TOOLS_BIN="
bcc_compat
llvm-rs-cc
"

TOOLS_LIB="
libbcc.so
libbcinfo.so
libclang.so
libLLVM.so
"

for a in $TOOLS_BIN; do
  cp $ANDROID_HOST_OUT/bin/$a tools/linux/
  strip tools/linux/$a
done

for a in $TOOLS_LIB; do
  cp $ANDROID_HOST_OUT/lib/$a tools/linux/
  strip tools/linux/$a
done

echo "DON'T FORGET TO UPDATE THE DARWIN COMPILER PREBUILTS!!!"
