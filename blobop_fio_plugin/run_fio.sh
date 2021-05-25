
SPDK_DIR="/home/vagrant/spdk"

sudo HUGEMEM=512 $SPDK_DIR/scripts/setup.sh
sudo LD_PRELOAD="$SPDK_DIR/build/fio/hello_src ../simple_fs/libspdk_spdk_simple_fs.so" fio  ./spdkfs.fio
sudo $SPDK_DIR/scripts/setup.sh reset
sudo rm -rf ./spdkfs