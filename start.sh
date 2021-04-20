PROGRAM="file_ut/file_ut"
DB="db"
if [ "$1" == "$DB" ];
then
sudo gdb --args env LD_PRELOAD=/home/vagrant/hello_spin_reactor/simple_fs/libspdk_spdk_simple_fs.so ./$PROGRAM;
else
sudo LD_PRELOAD=/home/vagrant/hello_spin_reactor/simple_fs/libspdk_spdk_simple_fs.so ./$PROGRAM;
fi
