# A filesystem framework for SPDK Blobstore

## Dependencies:

- SPDK v 20.10.x

## Getting started


1. Get a copy of `SPDK` at <https://github.com/spdk/spdk>

```sh

git clone https://github.com/spdk/spdk
cd spdk
sudo scripts/pkgdep.sh --all
./configure --with-shared
make
sudo make install

```


2. Compile

```sh
git clone https://github.com/Saltflow/spdk_blob_top_fs
cd spdk_blob_top_fs
./one_script_make.sh

```

3. Unit Test

```sh
cd unit_test
make
./start.sh

```

## Code structure


- ./spdk_fs : The basic filesystem framework in lieu of VFS

- ./simple_fs : A simple VFS

- ./blobop_fio_plugin : fio plugin

- ./test : some test cases 

- ./scripts: code style check adopted from `spdk`
