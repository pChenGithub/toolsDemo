
#编译ffmpeg
#交叉编译
./configure --prefix=/home/cp/build_something/FFmpeg-release-4.1/_install_arm --arch=arm --target-os=linux --enable-cross-compile  --cross-prefix=/home/cp/gcc_tools/gcc-linaro-5.4.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
make 
make install



