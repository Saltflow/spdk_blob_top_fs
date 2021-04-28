PROGRAM=("file_test/file_ut" "posix_test/posix_ut")
DB="db"
if [ "$1" == "$DB" ];
then
sudo gdb --args env LD_PRELOAD=/home/vagrant/hello_spin_reactor/simple_fs/libspdk_spdk_simple_fs.so ./${PROGRAM[1]};
else
for((i = 0; i < 2; i++)); do
sudo LD_PRELOAD=/home/vagrant/hello_spin_reactor/simple_fs/libspdk_spdk_simple_fs.so ./${PROGRAM[i]};
done;
fi
