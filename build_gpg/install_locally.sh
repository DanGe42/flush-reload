#!/bin/bash
# Use this script if you are using Linux or if OS X Homebrew is not installed.

set -o errexit
set -o nounset
set -o xtrace

SHA1HASH=9b78e20328d35525af7b8a9c1cf081396910e937
GPG_PREFIX=gnupg-1.4.12
GPG_ARCHIVE=${GPG_PREFIX}.tar.bz2
export CC=gcc   # If building on OS X, you will need apple-gcc42
export CFLAGS="-g -O2"

curl -O http://mirror.switch.ch/ftp/mirror/gnupg/gnupg/${GPG_ARCHIVE}
shasum -a 1 -c <(echo "${SHA1HASH}  ${GPG_ARCHIVE}")

if [ $? != 0 ]
then
  echo "WARNING: HASHES DO NOT MATCH"
  read -p -r -n 1 "Continue? [Y/n] " user_input

  if [ "$user_input" = y ] ||  [ "$user_input" = Y ]
  then
    exit 1
  fi
fi

tar xf ${GPG_ARCHIVE}
rm ${GPG_ARCHIVE}

cd ${GPG_PREFIX}
./configure --disable-dependency-tracking --disable-asm --prefix="${PWD}"
make && make check && make install
