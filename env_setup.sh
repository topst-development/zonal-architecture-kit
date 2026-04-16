#!/bin/bash

cd ~/

# Set Locale
echo "====================Set Locale===================="
sudo locale-gen en_US.UTF-8 && sudo update-locale LANG=en_US.UTF-8

echo 'LANG=en_US.UTF-8' | sudo tee -a /etc/default/locale && \
echo 'LC_ALL=en_US.UTF-8' | sudo tee -a /etc/default/locale

# update and upgrade apt package list
echo "====================Update Apt Package List===================="
sudo apt-get update && sudo apt-get -y upgrade

# Install SSH and Samba
echo "====================Install SSH and Samba===================="
sudo apt install -y net-tools openssh-server samba

# Install Utilities
echo "====================Install Utilities===================="
sudo apt-get install -y gawk wget git diffstat unzip texinfo gcc-multilib build-essential chrpath socat cpio python3 python3-pip python3-pexpect xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa-dev libsdl1.2-dev pylint xterm zstd ncftp curl git-lfs vim zip lz4 repo
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu


# Setup VCP-G Environment
echo "====================Setup VCP-G Environment===================="
wget https://releases.linaro.org/components/toolchain/binaries/7.2-2017.11/arm-eabi/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz
tar -xvf gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz
sudo mv gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi /opt/
sudo rm -rf gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi.tar.xz gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi
/opt/gcc-linaro-7.2.1-2017.11-x86_64_arm-eabi/bin/arm-eabi-gcc --version

# Setup tcnntoolkit Environment
echo "====================Set tc-nn-toolkit Environment===================="
sudo add-apt-repository ppa:deadsnakes/ppa -y
sudo apt install -y python3.8 python3.8-venv python3.8-dev python3.8-tk

cd zonal-architecture-kit
wget https://topst-downloads.s3.ap-northeast-2.amazonaws.com/Education/zonal-architecture-kit/tc-nn-toolkit.zip
unzip tc-nn-toolkit.zip
cd tc-nn-toolkit
python3.8 -m venv ./venv
source ./venv/bin/activate
pip install --upgrade pip
pip install -r requirements.txt
pip install torch==1.12.0+cpu torchvision==0.13.0+cpu torchaudio==0.12.0 --extra-index-url https://download.pytorch.org/whl/cpu
pip install ./enlight_viewer/netron-3.5.4-py2.py3-none-any.whl
deactivate

cd ~/ 
wget https://developer.arm.com/-/media/Files/downloads/gnu-a/9.2-2019.12/binrel/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
tar -xvf gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu.tar.xz
echo "export PATH=~/gcc-arm-9.2-2019.12-x86_64-aarch64-none-linux-gnu/bin:\$PATH" >> ~/.bashrc 
source ~/.bashrc




echo "====================Finish setting environment===================="
