checksums()
{
    if [ "$#" -ne 2 ]; then
        echo 'Please, specify two files to compare'
        exit 1
    fi

    f=`md5sum $1`
    g=`md5sum $2`

    if [ ${f:0:32} == ${g:0:32} ]; then
        echo "Test passed"
        exit 0
    else
        echo "Checksum failed"
        exit 1
    fi
}

check_for_file()
{
    for i in `seq 1 30`;
    do
        if [ ! -f $1 ]; then
            sleep 1
            continue
        fi
        checksums $1 $2
    done

    echo "Test timedout"
    exit 1
}

FILE="file.bin"
DIR="test/ci/001"

./grizzlycloud --nolog --config $DIR/server.cfg --loglevel trace &
./grizzlycloud --nolog --config $DIR/client.cfg --loglevel trace &

# create dst directories
echo 'Creating port directories'
mkdir -p $DIR/8888

# create file to dowload
dd if=/dev/zero of=$DIR/$FILE bs=1024 count=512

# wait for gc to start up
echo 'Waiting for GC to start up'
sleep 20

# create file copies
echo 'Copying files..'
PWD=`pwd`
scp -o "StrictHostKeyChecking no" -P 8888 localhost:$PWD/$DIR/$FILE $DIR/8888/

check_for_file $DIR/8888/$FILE $DIR/$FILE
