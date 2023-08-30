set -e


# Download necessary packages
sudo apt-get update -y
sudo apt-get upgrade -y
sudo apt-get install -y $(grep -vE "^\s*#" packages-list.txt  | tr "\n" " ")


# Fetch the latest stable OPTEE build for ARMv8
mkdir optee
cd optee
repo init -u https://github.com/OP-TEE/manifest.git -m qemu_v8.xml
repo sync


# Use the patch for Issue 6253 on github (https://github.com/OP-TEE/optee_os/pull/6253)
cd optee_os
git fetch github pull/6253/head && git checkout FETCH_HEAD


# Make Toolchains
cd ../build
make toolchains

# -----------------------------------------------------------------------
#                                END OF SCRIPT
# -----------------------------------------------------------------------


###############################   Build OPTEE   #########################
#                                                                       #
# Standard build:                                                       #
#           $ make run  OR  $ make -j8 run                              #
#                                                                       #
# Enable GDB Debugger:                                                  #
#           $ make GDBSERVER=y run                                      #
#                                                                       #
# Enable Kernel AddressSanitizer (KASAN):                               #
#           $ make CFG_CORE_SANITIZE_KADDRESS=y CFG_CORE_ASLR=n run     #
#                                                                       #
#                                                                       #
#########################################################################
