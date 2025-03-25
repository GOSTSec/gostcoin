if $CROSS_COMPILE_ISCLANG;then
	alias gcc="$TOOLCHAIN/$CROSS_COMPILE""clang"
	#echo gcc=$C
	alias g++="$TOOLCHAIN/$CROSS_COMPILE""clang++"
else
	alias gcc="$TOOLCHAIN/$CROSS_COMPILE""gcc"
	#echo gcc=$C
	alias g++="$TOOLCHAIN/$CROSS_COMPILE""g++"
fi;
