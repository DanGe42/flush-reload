#!/bin/bash
SHA1HASH=9b78e20328d35525af7b8a9c1cf081396910e937
SHA256HASH=4f03ca6902aaee79d0eda00eea0fefde7db9eb005d8ffd54fac9806bc24050ec
GPG_PREFIX=gnupg-1.4.12
GPG_ARCHIVE=${GPG_PREFIX}.tar.bz2
HASH=sha256sum
export CC=gcc   # If building on OS X, you will need apple-gcc42
export CFLAGS="-g -O2 -std=c89"

# Download GPG 1.4.12
curl -O http://mirror.switch.ch/ftp/mirror/gnupg/gnupg/${GPG_ARCHIVE}
shasum -a 256 -c <(echo "${SHA256HASH}  ${GPG_ARCHIVE}")

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

# Install GPG locally
cd ${GPG_PREFIX}
./configure --disable-dependency-tracking --disable-asm --prefix=${PWD}
make && make check

# Generate the secret key
cd ..
./gnupg-1.4.12/g10/gpg --gen-key --batch keyparams
./gnupg-1.4.12/g10/gpg --import flushreload.sec
