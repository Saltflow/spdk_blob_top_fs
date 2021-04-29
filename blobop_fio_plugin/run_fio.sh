
SPDK_DIR="/home/vagrant/spdk"

sudo HUGEMEM=512 $SPDK_DIR/scripts/setup.sh
sudo gdb --args env LD_PRELOAD="/home/vagrant/spdk/build/fio/hello_src /home/vagrant/hello_spin_reactor/simple_fs/libspdk_spdk_simple_fs.so" fio  ./spdkfs.fio
sudo $SPDK_DIR/scripts/setup.sh reset
