#!/bin/bash

export PATH=/home/yz/yzandroid/imx6-4.4.2/toolchains/arm-eabi-4.6/bin/:$PATH
export CROSS_COMPILE=arm-eabi-
export ARCH=arm


#! /bin/sh
echo "export build env gcc ..."
export PATH=$PATH:/home/cp/sda3/cqh6_linux_debian9/out/sun50iw6p1/debian/common/buildroot/external-toolchain/bin
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
