# ARM compilation. TODO
Create standalone: https://developer.android.com/ndk/guides/standalone_toolchain
Then change variables for you in setenv-android.sh and preparedepencies.sh ($ST and $NDK_PATH will be for standalone) but ANDROID_NDK_ROOT for real NDK-bundle.
then run preparedepencies.sh
then go to ../src/ and run compile_android.sh
Or change variables in gostcoin-qt-android.pro and compile for Qt...
```
lialh4@RBH4:~/GTMP/src$ lddandr gostcoind 
  0x00000001 (NEEDED)          Shared library: [libssl.so.1.1]
  0x00000001 (NEEDED)          Shared library: [libcrypto.so.1.1]
  0x00000001 (NEEDED)          Shared library: [libz.so]
  0x00000001 (NEEDED)          Shared library: [libdl.so]
  0x00000001 (NEEDED)          Shared library: [libc++_shared.so]
  0x00000001 (NEEDED)          Shared library: [libm.so]
  0x00000001 (NEEDED)          Shared library: [libc.so]
```
Now is static. but some trouble ```
error: "./gostcoind": executable's TLS segment is underaligned: alignment is 8, needs to be at least 32 for ARM Bionic
libc: error: "./gostcoind": executable's TLS segment is underaligned: alignment is 8, needs to be at least 32 for ARM Bionic```
You can to try change -lc to LMODE2 in makefile.android but then you will to get segmentation fault. TODO: fix it...
