#!/bin/sh

if [ -z $SNUCLROOT ]; then
  echo "[ERROR] Set \"SNUCLROOT\" environment."
fi

#runtime

## USER SELECTION ##

#Default Devices types
X86=off
LEGACY=off

#Default Cluster Mode
CLUSTER=off

#User Selection
while [ $# -gt 0 ]; do
  case "$1" in
		X86) X86=on ;;
		LEGACY) LEGACY=on ;;
		CLUSTER) CLUSTER=on ;;
	esac
	shift
done
## USER SELECTION ##

echo "********** [BUILD] SnuCL Runtime **********"
cd $SNUCLROOT/src/runtime
make X86=${X86} LEGACY=${LEGACY} CLUSTER=${CLUSTER} 


#compiler
echo "********** [BUILD] SnuCL Compiler **********"
CBT=compiler-build-temp
cd $SNUCLROOT/build/
mkdir -p $CBT
cd $CBT
$SNUCLROOT/src/compiler/configure
make BUILD_EXAMPLES=1 ENABLE_OPTIMIZED=1
cp Release/bin/clang $SNUCLROOT/bin
cp Release/bin/snuclc-merger $SNUCLROOT/bin
cp Release/lib/libSnuCL* $SNUCLROOT/lib

#builtins
echo "********** [BUILD] SnuCL Built-in Functions **********"
cd $SNUCLROOT/src/builtins/x86
make -j24
make install

echo "********** [BUILD] Complete  **********"
