andr=$(pwd)/../android
export BOOST_INCLUDE_PATH=$andr/boost_1_76_0/
export BDB_INCLUDE_PATH=$andr/db-18.1.40/build_unix/
export BDB_LIB_PATH=$BDB_INCLUDE_PATH
export OPENSSL_PATH=$andr/openssl-OpenSSL_1_1_1k/
export OPENSSL_INCLUDE_PATH=$OPENSSL_PATH/include
export IFADDRS_PATH=$andr/android-ifaddrs-from-android-source
export BOOST_LIB_PATH=$BOOST_INCLUDE_PATH/stage/lib
export OPENSSL_LIB_PATH=$OPENSSL_PATH
export ANDROID_INCLUDE_PATH=$ST/sysroot/usr/include
export ANDROID_LIB_PATH=$ST/sysroot/usr/lib/arm-linux-androideabi/
#read
if ! test -e gostcoind;then
	make -f makefile.android
fi;

#mkdir -p android_build
#cd android_build
#cp ../gostcoind . 
#cp $OPENSSL_LIB_PATH/*.so* .
#tar cvpf gostcoind.tar *
#ls $BOOST_LIB_PATH # there is static...
#ls $BDB_LIB_PATH # there is too
