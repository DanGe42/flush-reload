#!/bin/bash
SHA1HASH=9b78e20328d35525af7b8a9c1cf081396910e937
GPG_PREFIX=gnupg-1.4.12
GPG_ARCHIVE=${GPG_PREFIX}.tar.bz2
export CC="/usr/local/Cellar/apple-gcc42/4.2.1-5666.3/bin/gcc-4.2"   # If building on OS X, you will need apple-gcc42
export CFLAGS="-g -O2"

curl -O http://mirror.switch.ch/ftp/mirror/gnupg/gnupg/${GPG_ARCHIVE}
shasum -a 1 -c <(echo "${SHA1HASH}  ${GPG_ARCHIVE}")

if [ $? != 0 ]
then
  echo "WARNING: HASHES DO NOT MATCH"
  read -p -n 1 "Continue? [Y/n] " user_input

  if [ user_input = y -o user_input = Y ]
  then
    exit 1
  fi
fi

tar xf ${GPG_ARCHIVE}
rm ${GPG_ARCHIVE}

cd ${GPG_PREFIX}
./configure --disable-dependency-tracking --disable-asm --prefix=${PWD}
make && make check && make install
