#!/bin/bash
# Cross-compile environment for Android
#
# Contents licensed under the terms of the OpenSSL license
# http://www.openssl.org/source/license.html
#
# See http://wiki.openssl.org/index.php/FIPS_Library_and_Android
#   and http://wiki.openssl.org/index.php/Android
#
# Originally provided by OpenSSL, modified by R4SAS
# https://github.com/PurpleI2P/OpenSSL-for-Android-Prebuilt

#####################################################################

# Example of usage:
# * Building OpenSSL 1.1.1 for armeabi-v7a using llvm (clang)
#   * Change _ANDROID_EABI to llvm
#   * Change _ANDROID_ARCH to arch-arm
#   * Change _ANDROID_API to 14
#   * Run in shell:
#     $ . setenv-android.sh
#     $ cd openssl-1.1.1
#     $ ./Configure android-arm shared no-ssl3 no-comp no-hw no-engine -D__ARM_MAX_ARCH__=8
#     $ make
#   * Take compiled libraries from sources root directory
#
# * Building OpenSSL 1.1.1 for arm64-v8a using llvm (clang)
#   * Change _ANDROID_EABI to llvm
#   * Change _ANDROID_ARCH to arch-arm64
#   * Change _ANDROID_API to 21
#   * Run in shell:
#     $ . setenv-android.sh
#     $ cd openssl-1.1.1
#     $ ./Configure android-arm64 shared no-ssl3 no-comp no-hw no-engine
#     $ make
#   * Take compiled libraries from sources root directory
#
# Note: gcc toolchains is deprecated since NDK r18, prefered to use llvm.

####################################################################
#CHANGE TO YOUR PATH
ANDROID_NDK_ROOT="/home/lialh4/Android/Sdk/android-ndk-r21e"

# Set ANDROID_NDK_ROOT to you NDK location. For example,
# /opt/android-ndk-r8e or /opt/android-ndk-r9. This can be done in a
# login script. If ANDROID_NDK_ROOT is not specified, the script will
# try to pick it up with the value of _ANDROID_NDK_ROOT below. If
# ANDROID_NDK_ROOT is set, then the value is ignored.
_ANDROID_NDK="android-ndk-r21e"

# Set _ANDROID_EABI to the EABI you want to use. You can find the
# list in $ANDROID_NDK_ROOT/toolchains. This value is always used.
#_ANDROID_EABI="arm-linux-androideabi-4.9"
#_ANDROID_EABI="aarch64-linux-android-4.9"
#_ANDROID_EABI="x86-4.9"
#_ANDROID_EABI="x86_64-4.9"
_ANDROID_EABI="llvm"

# Set _ANDROID_ARCH to the architecture you are building for.
# This value is always used.
_ANDROID_ARCH=arch-arm
#_ANDROID_ARCH=arch-arm64
#_ANDROID_ARCH=arch-x86
#_ANDROID_ARCH=arch-x86_64

# Set _ANDROID_API to the API you want to use.
# This value is always used.
# ! If you build for arm64-v8a or x86_64, you MUST use API 21+ !
#_ANDROID_API="android-14"
_ANDROID_API="android-28"


# armeabi/armeabi-v7a toogler (for gcc)
# If you build for armeabi, change to false
IsARMv7=false

#####################################################################

# If the user did not specify the NDK location, try and pick it up.
# We expect something like ANDROID_NDK_ROOT=/opt/android-ndk-r8e
# or ANDROID_NDK_ROOT=/usr/local/android-ndk-r8e.


if [ -z "$ANDROID_NDK_ROOT" ]; then

  _ANDROID_NDK_ROOT=""
  if [ -z "$_ANDROID_NDK_ROOT" ] && [ -d "/usr/local/$_ANDROID_NDK" ]; then
    _ANDROID_NDK_ROOT="/usr/local/$_ANDROID_NDK"
  fi

  if [ -z "$_ANDROID_NDK_ROOT" ] && [ -d "/opt/$_ANDROID_NDK" ]; then
    _ANDROID_NDK_ROOT="/opt/$_ANDROID_NDK"
  fi

  if [ -z "$_ANDROID_NDK_ROOT" ] && [ -d "$HOME/$_ANDROID_NDK" ]; then
    _ANDROID_NDK_ROOT="$HOME/$_ANDROID_NDK"
  fi

  if [ -z "$_ANDROID_NDK_ROOT" ] && [ -d "$PWD/$_ANDROID_NDK" ]; then
    _ANDROID_NDK_ROOT="$PWD/$_ANDROID_NDK"
  fi

  # If a path was set, then export it
  if [ ! -z "$_ANDROID_NDK_ROOT" ] && [ -d "$_ANDROID_NDK_ROOT" ]; then
    export ANDROID_NDK_ROOT="$_ANDROID_NDK_ROOT"
  fi
fi

# Error checking
# ANDROID_NDK_ROOT should always be set by the user (even when not running this script)
# http://groups.google.com/group/android-ndk/browse_thread/thread/a998e139aca71d77
if [ -z "$ANDROID_NDK_ROOT" ] || [ ! -d "$ANDROID_NDK_ROOT" ]; then
  echo "Error: ANDROID_NDK_ROOT is not a valid path. Please edit this script."
  # echo "$ANDROID_NDK_ROOT"
  # exit 1
fi

# Error checking
if [ ! -d "$ANDROID_NDK_ROOT/toolchains" ]; then
  echo "Error: ANDROID_NDK_ROOT/toolchains is not a valid path. Please edit this script."
  # echo "$ANDROID_NDK_ROOT/toolchains"
  # exit 1
fi

# Error checking
if [ ! -d "$ANDROID_NDK_ROOT/toolchains/$_ANDROID_EABI" ]; then
  echo "Error: ANDROID_EABI is not a valid path. Please edit this script."
  # echo "$ANDROID_NDK_ROOT/toolchains/$_ANDROID_EABI"
  # exit 1
fi

#####################################################################

# Based on ANDROID_NDK_ROOT, try and pick up the required toolchain. We expect something like:
# /opt/android-ndk-r83/toolchains/arm-linux-androideabi-4.7/prebuilt/linux-x86_64/bin
# Once we locate the toolchain, we add it to the PATH. Note: this is the 'hard way' of
# doing things according to the NDK documentation for Ice Cream Sandwich.
# https://android.googlesource.com/platform/ndk/+/ics-mr0/docs/STANDALONE-TOOLCHAIN.html

ANDROID_TOOLCHAIN=""
for host in "linux-x86_64" "linux-x86" "darwin-x86_64" "darwin-x86"
do
  if [ -d "$ANDROID_NDK_ROOT/toolchains/$_ANDROID_EABI/prebuilt/$host/bin" ]; then
    ANDROID_TOOLCHAIN="$ANDROID_NDK_ROOT/toolchains/$_ANDROID_EABI/prebuilt/$host/bin"
    break
  fi
done

# Error checking
if [ -z "$ANDROID_TOOLCHAIN" ] || [ ! -d "$ANDROID_TOOLCHAIN" ]; then
  echo "Error: ANDROID_TOOLCHAIN is not valid. Please edit this script."
  # echo "$ANDROID_TOOLCHAIN"
  # exit 1
fi

if [ "$_ANDROID_EABI" == "llvm" ]; then
  ANDROID_TOOLS="clang llvm-ar llvm-link"
else
  case $_ANDROID_ARCH in
    arch-arm)
      ANDROID_TOOLS="arm-linux-androideabi-gcc arm-linux-androideabi-ranlib arm-linux-androideabi-ld"
      ;;
    arch-arm64)
      ANDROID_TOOLS="aarch64-linux-android-gcc aarch64-linux-android-ranlib aarch64-linux-android-ld"
      ;;
    arch-x86)
      ANDROID_TOOLS="i686-linux-android-gcc i686-linux-android-ranlib i686-linux-android-ld"
      ;;
    arch-x86_64)
      ANDROID_TOOLS="x86_64-linux-android-gcc x86_64-linux-android-ranlib x86_64-linux-android-ld"
      ;;
    *)
      echo "ERROR ERROR ERROR"
      ;;
  esac
fi

for tool in $ANDROID_TOOLS
do
  # Error checking
  if [ ! -e "$ANDROID_TOOLCHAIN/$tool" ]; then
    echo "Error: Failed to find $tool. Please edit this script."
    # echo "$ANDROID_TOOLCHAIN/$tool"
    # exit 1
  fi
done

# Only modify/export PATH if ANDROID_TOOLCHAIN good
if [ ! -z "$ANDROID_TOOLCHAIN" ]; then
  export ANDROID_TOOLCHAIN="$ANDROID_TOOLCHAIN"
  export PATH="$ANDROID_TOOLCHAIN":"$PATH"
fi

#####################################################################

# For the Android SYSROOT. Can be used on the command line with --sysroot
# https://android.googlesource.com/platform/ndk/+/ics-mr0/docs/STANDALONE-TOOLCHAIN.html
export ANDROID_SYSROOT="$ANDROID_NDK_ROOT/platforms/$_ANDROID_API/$_ANDROID_ARCH"
export SYSROOT="$ANDROID_SYSROOT"
export NDK_SYSROOT="$ANDROID_SYSROOT"

# Error checking
if [ -z "$ANDROID_SYSROOT" ] || [ ! -d "$ANDROID_SYSROOT" ]; then
  echo "Error: ANDROID_SYSROOT is not valid. Please edit this script."
  # echo "$ANDROID_SYSROOT"
  # exit 1
fi

#####################################################################

# Most of these should be OK (MACHINE, SYSTEM, ARCH). RELEASE is ignored.
if [ "$_ANDROID_ARCH" == "arch-arm" ]; then
  if [ "$IsARMv7" = true ]; then
    export MACHINE=armv7
  else
    export MACHINE=arm
  fi
  export RELEASE=2.6.37
  export SYSTEM=android
  export ARCH=arm
  export CROSS_COMPILE="arm-linux-androideabi-"
fi

if [ "$_ANDROID_ARCH" == "arch-arm64" ]; then
  export MACHINE=arm64
  export RELEASE=2.6.37
  export SYSTEM=android
  export ARCH=aarch64
  export CROSS_COMPILE="aarch64-linux-android-"
fi

if [ "$_ANDROID_ARCH" == "arch-x86" ]; then
  export MACHINE=i686
  export RELEASE=2.6.37
  export SYSTEM=android
  export ARCH=x86
  export CROSS_COMPILE="i686-linux-android-"
fi

if [ "$_ANDROID_ARCH" == "arch-x86_64" ]; then
  export MACHINE=x86_64
  export RELEASE=2.6.37
  export SYSTEM=android
  export ARCH=x86_64
  export CROSS_COMPILE="x86_64-linux-android-"
fi

if [ "$_ANDROID_EABI" == "llvm" ]; then
  unset CROSS_COMPILE
fi

# For the Android toolchain
# https://android.googlesource.com/platform/ndk/+/ics-mr0/docs/STANDALONE-TOOLCHAIN.html
export ANDROID_NDK="$ANDROID_NDK_ROOT"
export ANDROID_SYSROOT="$ANDROID_NDK_ROOT/platforms/$_ANDROID_API/$_ANDROID_ARCH"
export SYSROOT="$ANDROID_SYSROOT"
export CROSS_SYSROOT="$ANDROID_SYSROOT"
export NDK_SYSROOT="$ANDROID_SYSROOT"
export ANDROID_NDK_SYSROOT="$ANDROID_SYSROOT"
export ANDROID_API="$_ANDROID_API"

# CROSS_COMPILE and ANDROID_DEV are DFW (Don't Fiddle With). Its used by OpenSSL build system.
# export CROSS_COMPILE="arm-linux-androideabi-"
export ANDROID_DEV="$ANDROID_NDK_ROOT/platforms/$_ANDROID_API/$_ANDROID_ARCH/usr"
#export HOSTCC=gcc

VERBOSE=1
if [ ! -z "$VERBOSE" ] && [ "$VERBOSE" != "0" ]; then
  echo "ANDROID_NDK_ROOT: $ANDROID_NDK_ROOT"
  echo "ANDROID_ARCH: $_ANDROID_ARCH"
  echo "ANDROID_EABI: $_ANDROID_EABI"
  echo "ANDROID_API: $ANDROID_API"
  echo "ANDROID_SYSROOT: $ANDROID_SYSROOT"
  echo "ANDROID_TOOLCHAIN: $ANDROID_TOOLCHAIN"
  echo "CROSS_COMPILE: $CROSS_COMPILE"
  echo "ANDROID_DEV: $ANDROID_DEV"
fi
