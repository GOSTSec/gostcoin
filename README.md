GOSTCoin Core
=============

GOSTCoin (GST) is a digital currency based on [blockchain](https://en.wikipedia.org/wiki/Blockchain) technology.
It allows instant payments worldwide with focus on privacy and security of its users.

Why GOSTCoin?
-------------

GOSTCoin uses Soviet and Russian government standard cryptography: 
GOST R 34.10-2012 for signature and GOST R 34.11-2012 for hash. 
[More info about crypto](https://github.com/GOSTSec/gostcoin/wiki/Cryptography).

GOSTCoin is using [Invisible Internet](https://github.com/PurpleI2P/i2pd) (I2P) as a secure network layer.

GOSTCoin needs I2P router
-------------------------

Install and run [i2pd](https://github.com/PurpleI2P/i2pd).

Enable SAM API in i2pd. Edit in `i2pd.conf`:

    [sam]
    enabled = true

and restart i2pd. Local TCP port 7656 should be available.

Building GOSTCoin
-----------------

**Install development libraries:**

    apt-get install build-essential libtool libboost-all-dev git libdb++-dev libssl-dev zlib1g-dev

**Clone repository:**

    git clone https://github.com/GOSTSec/gostcoin.git ~/gostcoin

**Build gostcoind:**

    cd ~/gostcoin/src
    make -f makefile.unix

**Optional: Build QT GUI**

    # install requirements 
    apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler 
    # build GUI
    cd ~/gostcoin
    qmake && make
    
    # build GUI with QR codes and paper wallet
    apt-get install libqrencode-dev
    cd ~/gostcoin
    qmake "USE_QRCODE=1" && make

Mining tools
------------

Dedicated mining tools are available: [cpuminer for CPU](https://github.com/GOSTSec/cpuminer-x11-gost), [ccminer for NVIDIA GPU](https://github.com/GOSTSec/ccminer) and [sgminer for AMD GPU](https://github.com/GOSTSec/sgminer)  

License
-------

GOSTCoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see http://opensource.org/licenses/MIT.

