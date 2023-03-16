#!/bin/bash
#Plaz
# ndk/build/tools/make_standalone_toolchain.py for standalone build
preinit(){
  source setenv-android.sh

  export STANDALONE=/home/lialh4/Android/Sdk/ndk/arm-linux-androideabi/

  export CPPFLAGS=--sysroot=$ANDROID_SYSROOT
  export CXXFLAGS="--sysroot=$ANDROID_SYSROOT"
  export TOOLCHAIN=$ANDROID_TOOLCHAIN
  export NDK_PATH=$ANDROID_NDK_ROOT
  export PATH=$TOOLCHAIN/bin/:$PATH:$STANDALONE/bin/
  export ARCH=arm
  export bdbv=18.1.40
  export CROSS_COMPILE_ISCLANG=true
  export CROSS_COMPILE= 
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
 ./Configure android-arm shared no-ssl3 no-comp no-hw no-engine || exit 1
 echo $ANDROID_NDK_ROOT/platforms/android-21/arch-arm/usr/lib/
 make || exit 1
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
 ../dist/configure  --enable-cxx --disable-shared --disable-replication --host=arm-linux-androideabi CC=arm-linux-androideabi-gcc CPPFLAGS="-I$ANDROID_HOME/sysroot/usr/include/arm-linux-androideabi/ -I$ANDROID_HOME/sysroot/usr/include/ -I$ANDROID_HOME/sources/cxx-stl/llvm-libc++/include/"
 make
 cd ..
}
compile_boost(){
  cd boost_1_76_0
  ./bootstrap.sh
  echo -e "import option ;\nusing gcc : arm : arm-linux-androideabi-g++ ;\noption.set keep-going : false ; ">project-config.jam
  echo "$PATH"
  ./b2 --layout=versioned --build-type=complete toolset=gcc-arm variant=release link=static threading=multi threadapi=pthread target-os=android define=BOOST_MATH_DISABLE_FLOAT128
  cd stage/lib
  for lib in *.a;
  do
	normalname=${lib/-*1_76.a/.a}
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
#gcc -v 

compile_openssl
compile_boost
compile_db
#compile_level_db


echo "Now cd to ../src/ and run compile_android.sh"

