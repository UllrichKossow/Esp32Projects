#!/bin/bash -x
if [ $# -eq 0 ]; then
    echo missing argument
    exit
fi
for f in $(find -name sdkconfig.defaults); do
    pushd $(dirname $f)
    idf.py $*
    popd
done

