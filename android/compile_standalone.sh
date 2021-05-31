#!/bin/bash
# ndk/build/tools/make_standalone_toolchain.py for standalone build
preinit(){
  #CHANGE TO YOUR PATH
  ANDROID_NDK_ROOT="/home/lialh4/Android/Sdk/android-ndk-r21e"

  export CPPFLAGS=--sysroot=$ANDROID_NDK_ROOT/platforms/android-21/arch-arm/
  export CXXFLAGS="--sysroot=$ANDROID_NDK_ROOT/platforms/android-21/arch-arm/ -I$ANDROID_NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.9/include -I$ANDROID_NDK_ROOT/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi-v7a/include"

  export ST=/home/lialh4/Android/Sdk/ndk/arm-linux-androideabi
  export NDK_PATH=$ST
  export boost=`pwd`/boost_1_57_0
  export C=$NDK_PATH/bin/arm-linux-androideabi-gcc
  export CXX=$NDK_PATH/bin/arm-linux-androideabi-g++
  export NDK_PROJECT_PATH=$NDK_PATH
  export ANDROID_NDK_ROOT=$NDK_PATH
  export ANDROID_NDK_HOST=linux-x86_64
  export PATH=$ST/bin/:$PATH
  export CROSS_COMPILE=arm-linux-androideabi-
  export ARCH=arm
  #export PREINITED_SOURCES=/home/lialh4/Android/Sdk/ndk/crystax-ndk-10.3.1/sources/
  #arm-linux-androideabi-g++ -v
  export AR=$NDK_PATH/bin/arm-linux-androideabi-ar
  export AS=$NDK_PATH/bin/arm-linux-androideabi-as
  export CC=$NDK_PATH/bin/arm-linux-androideabi-gcc
  export bdbv=18.1.40

}

download_depencies(){
	wget -t0 https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_1_1_1k.zip
	wget -t0 https://fossies.org/linux/misc/db-$bdbv.tar.gz
	wget -t0 https://boostorg.jfrog.io/artifactory/main/release/1.76.0/source/boost_1_76_0.tar.gz
	git clone https://github.com/GOSTSec/android-ifaddrs-from-android-source  
}
check_depencies(){
	if  ! test -e OpenSSL_1_1_1k.zip || ! test -e db-*.tar.gz || ! test -e boost*.tar.gz;then
		echo "Not exists depencies, download"
		download_depencies
	fi;
	
	#if ! test -e $boost ; then
	#	echo "Please download libboost for android http://sourceforge.net/projects/boost/files/boost/1.57.0/boost_1_57_0.7z/download and put in the $(pwd) directory"
	#fi;
	#git clone https://github.com/moritz-wundke/Boost-for-Android.git
	unpack_depencies
}
unpack_depencies(){
	if ! test -d openssl-OpenSSL_1_1_1k || ! test -d "db-$bdbv" || ! test -d boost_1_76_0;then
	tar xzvf "db-$bdbv.tar.gz" -C .
	7z x OpenSSL_1_1_1k.zip 
	tar xvpf boost_1_76_0.tar.gz -C .
	fi;
}
compile_openssl(){
 cd openssl-OpenSSL_1_1_1k
 ./Configure android-arm shared no-ssl3 no-comp no-hw no-engine -D__ARM_MAX_ARCH__=8
 echo $ANDROID_NDK_ROOT/platforms/android-21/arch-arm/usr/lib/
 make 
 cd ..
}
compile_db(){
 cd db-$bdbv/dist
 if ! test -e config.sub.old;then
 	mv config.sub config.sub.old
 	wget 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD' -O config.sub
 	mv config.guess config.guess.old
 	wget 'http://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD' -O config.guess
 	cp config.* ../lang/sql/sqlite/
 fi;
 cd ../build_unix
 export ANDROID_HOME=$ANDROID_NDK_ROOT
 #echo $ANDROID_HOME//sysroot/usr/include/
 #exit
#sources/cxx-stl/llvm-libc++/include/atomic

 ../dist/configure  --enable-cxx --disable-shared --disable-replication --host=arm-linux-androideabi CC=arm-linux-androideabi-gcc CPPFLAGS="-I$ANDROID_HOME/sysroot/usr/include/arm-linux-androideabi/ -I$ANDROID_HOME/sysroot/usr/include/ -I$ANDROID_HOME/sources/cxx-stl/llvm-libc++/include/"
# ../dist/configure --host=arm-linux-androideabi --enable-cxx --enable-shared --disable-replication

 make
 cd ..
}
compile_boost(){
  cd boost_1_76_0
  ./bootstrap.sh
  echo 'import option ; using gcc : arm : arm-linux-androideabi-g++ ;option.set keep-going : false ; '>project-config.jam
  ./b2 --layout=versioned --build-type=complete toolset=gcc-arm variant=release link=static threading=multi threadapi=pthread target-os=android define=BOOST_MATH_DISABLE_FLOAT128 include=$NDK_PATH/sources/cxx-stl/gnu-libstdc++/4.9/include include=$NDK_PATH/sources/cxx-stl/gnu-libstdc++/4.9/libs/armeabi/include include=$NDK_PATH/platforms/android-14/arch-arm/usr/include
  cd stage/lib
  for lib in *.a;
  do
	normalname=${lib/-gcc-mt-s-1_76.a/.a}
	echo "cp $lib to $normalname"
	cp $lib $normalname
  done;
  cd ../../..
}

compile_level_db(){
#maybe not need... because will be compiled but... automatic
  cd ../src/leveldb
  TARGET_OS=OS_ANDROID_CROSSCOMPILE make libleveldb.a libmemenv.a
  cd ../../android
}
preinit
check_depencies
shopt -s expand_aliases
. aliases.sh
source setenv-android.sh
gcc -v 

compile_db

compile_openssl

compile_boost
compile_level_db

#BOOST_PATH = $$MAIN_PATH/Boost-for-Android-Prebuilt/boost_1_64_0
#OPENSSL_PATH = $$MAIN_PATH/OpenSSL-for-Android-Prebuilt/openssl-1.1.1
#IFADDRS_PATH = $$MAIN_PATH/android-ifaddrs-from-android-source
#BDB_PATH = $$MAIN_PATH/db-6.0.20/build_unix
#BOOST_LIB_PATH = $$BOOST_PATH/$$ANDROID_TARGET_ARCH/lib
#OPENSSL_LIB_PATH = $$OPENSSL_PATH/$$ANDROID_TARGET_ARCH/lib
#cd ../src/
#make -f makefile.android
echo "Now cd to ../src/ and run compile_android.sh"
