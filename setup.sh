#!/bin/sh

#install dependencies
sudo apt-get install -y cmake
sudo apt-get install -y libeigen3-dev
sudo apt-get install -y zlib1g zlib1g.dev
sudo apt-get install -y build-essential libgtk2.0-dev libavcodec-dev libavformat-dev libjpeg-dev libtiff5-dev libswscale-dev libjasper-dev 
sudo apt-get install mysql-server mysql-client libmysqlclient-dev
sudo apt-get install libopenblas-dev

#install opencv
sudo apt-get install -y unzip
unzip -x opencv-4.1.1.zip
OPENCV_DIR=$(pwd)/opencv-4.1.1
cd $OPENCV_DIR
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local ..
sudo make
sudo make install
sudo echo /usr/local/lib  >> /etc/ld.so.conf.d/opencv.conf 
sudo ldconfig  
sudo echo -e "PKG_CONFIG_PATH=\$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig\nexport PKG_CONFIG_PATH"  >>  /etc/bash.bashrc
source /etc/bash.bashrc
sudo updatedb

#源码编译tensorflow动态库
