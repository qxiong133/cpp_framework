#!/bin/sh

program_root=$1
export LD_LIBRARY_PATH=$program_root/lib

cd $program_root;cd bin;pwd;./$2
