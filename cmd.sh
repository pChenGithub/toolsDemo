
#编译ffmpeg
#交叉编译
./configure --enable-libx264 --enable-gpl --prefix=/home/cp/build_something/FFmpeg-release-4.1/_install_arm --enable-shared --arch=arm --target-os=linux --enable-cross-compile  --cross-prefix=/home/cp/gcc_tools/gcc-linaro-5.4.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
make 
make install


#编译libx264
#交叉编译
./configure --prefix=/home/cp/build_something/x264-master/_install_arm --disable-asm --enable-shared --host=arm-linux --cross-prefix=/home/cp/gcc_tools/gcc-linaro-5.4.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-


#编译qt5.9.6
#脚本文件

#! /bin/sh
PWD=$(pwd)

echo "build qt source codec ..."
echo "export gcc path ..."
source /home/cp/gcc_tools/mkenv-gcc-5.4.1.sh
echo "done"

echo "path: ${PWD}"

echo "config ..."
./configure -prefix ${PWD}/_install_arm \
    -xplatform arm-linux-gnueabihf-gcc \
    -nomake examples \
    -nomake tests    \
    -nomake tools \
    -opensource \
    -confirm-license \
    -no-opengl
#    -opengl es2 \
#    -gstreamer \

#./configure -prefix ${PWD}/_install
echo "done"
echo "done"





