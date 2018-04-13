#!/bin/bash

source test/gc_common

FILE="file.bin"
DIR="test/ci/001"
ports=( 8888 8889 )

# Start server connected to US node
gc_start $DIR/server.cfg $DIR/backend_us.cfg
# Start client connected to UK node
gc_start $DIR/client.cfg $DIR/backend_uk.cfg

# Create dst directories
echo 'Creating port directories'
for p in ${ports[@]}; do
    mkdir -p $DIR/$p
done

# Create file to dowload
dd if=/dev/zero of=$DIR/$FILE bs=1024 count=512

# Wait for gc to start up
echo 'Waiting for GC to start up'
sleep 20

# Create file copies
echo 'Copying files..'
PWD=`pwd`
for p in ${ports[@]}; do
    scp -o "StrictHostKeyChecking no" -P $p localhost:$PWD/$DIR/$FILE $DIR/$p/
done

# Final checksums
DST=$DIR/$FILE
for p in ${ports[@]}; do
    c1=$(gc_files_cmp $DIR/$p/$FILE $DST)
    echo "Checksums of $DIR/$p/$FILE and $DST, error: $c1"
    if [ $c1 -ne 0 ]; then
        exit 1
    fi
done

gc_stop

sleep 2

cat $DIR/server.cfg.vglog
cat $DIR/client.cfg.vglog

# Valgrind logs test
#if [ $(gc_check_vg $DIR/server.cfg.vglog) -ne 0 ]; then
#    echo "VG log server check failed"
#    exit 1
#fi

#if [ $(gc_check_vg $DIR/client.cfg.vglog) -ne 0 ]; then
#    echo "VG log client check failed"
#    exit 1
#fi
