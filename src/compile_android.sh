yn(){
#	echo "$all = all"
	if [[ "$all" == "y" ]];then
		return 0
	fi;

	echo "$1?[y/n/a]: "
        read sure
        while [[ "$sure" != "y"* ]] && [[ "$sure" != "n"* ]] && [[ "$sure" != "a"* ]];
        do
		echo "$1?[y/n/a]: "
                read sure
        done;
        if [[ $sure == "y"* ]];then
                return 0
        elif [[ $sure == "n"* ]]
	then
                return 1
	else
		export all="y"
		return 0
        fi;
}

andr=$(pwd)/../android
source $andr/setenv-android.sh
export BOOST_INCLUDE_PATH=$andr/boost_1_76_0/
export BDB_INCLUDE_PATH=$andr/db-18.1.40/build_unix/
export BDB_LIB_PATH=$BDB_INCLUDE_PATH
export OPENSSL_PATH=$andr/openssl-OpenSSL_1_1_1k/
export OPENSSL_INCLUDE_PATH=$OPENSSL_PATH/include
export IFADDRS_PATH=$andr/android-ifaddrs-from-android-source
export BOOST_LIB_PATH=$BOOST_INCLUDE_PATH/stage/lib
export OPENSSL_LIB_PATH=$OPENSSL_PATH
#export ANDROID_INCLUDE_PATH=$ST/sysroot/usr/include
#export ANDROID_LIB_PATH=$ST/sysroot/usr/lib/arm-linux-androideabi/
echo "Cd to ../android for run script preparedepencies.sh but before change variables for you in preparedepcnies.sh and setenv-android.sh"
echo "Then (in the src directory) open makefile.android and change CXX/Sysroot variables;"
echo "When is be done - ENTER..."
read
if ! test -e gostcoind;then
	make -f makefile.android
else
	echo "your gostcoind like to compiled."
	if yn "rebuild gostcoind?"; then
		 make -f makefile.android clean
		 make -f makefile.android	
	fi;
fi;
