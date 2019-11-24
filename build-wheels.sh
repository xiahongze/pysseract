#!/bin/bash
set -e -x
yum -y install wget

MACHINE_TYPE=`uname -m`
SERVER=http://copr-be.cloud.fedoraproject.org/results/mosquito/myrepo-el6/
VERSION=glibc-2.17-55.fc20
REPO64=epel-6-x86_64
REPO32=epel-6-i386
SERVER32=$SERVER/$REPO32/$VERSION
SERVER64=$SERVER/$REPO64/$VERSION
#x64 rpms
GLIBC64_1=glibc-2.17-55.el6.x86_64.rpm
GLIBC64_2=glibc-common-2.17-55.el6.x86_64.rpm
GLIBC64_3=glibc-devel-2.17-55.el6.x86_64.rpm
GLIBC64_4=glibc-headers-2.17-55.el6.x86_64.rpm
GLIBC64_5=glibc-static-2.17-55.el6.x86_64.rpm

#x32 rpms
GLIBC32_1=glibc-2.17-55.el6.i686.rpm
GLIBC32_2=glibc-common-2.17-55.el6.i686.rpm
GLIBC32_3=glibc-devel-2.17-55.el6.i686.rpm
GLIBC32_4=glibc-headers-2.17-55.el6.i686.rpm
GLIBC32_5=glibc-static-2.17-55.el6.i686.rpm
# update glibc and download conda
if [ ${MACHINE_TYPE} == 'x86_64' ]; then
    wget --quiet $SERVER64/$GLIBC64_1
    wget --quiet $SERVER64/$GLIBC64_2
    wget --quiet $SERVER64/$GLIBC64_3
    wget --quiet $SERVER64/$GLIBC64_4
    wget --quiet $SERVER64/$GLIBC64_5
    # yum -y install $GLIBC64_1 $GLIBC64_2 $GLIBC64_3 $GLIBC64_4 $GLIBC64_5
    wget --quiet --no-check-certificate https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O ./miniconda.sh
else
    wget --quiet $SERVER32/$GLIBC32_1
    wget --quiet $SERVER32/$GLIBC32_2
    wget --quiet $SERVER32/$GLIBC32_3
    wget --quiet $SERVER32/$GLIBC32_4
    wget --quiet $SERVER32/$GLIBC32_5
    # yum -y install $GLIBC32_1 $GLIBC32_2 $GLIBC32_3 $GLIBC32_4 $GLIBC32_5
    wget --quiet --no-check-certificate https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86.sh -O ./miniconda.sh
fi


# From https://www.tekovic.com/installing-tesseract-ocr-40-on-centos-6
# This code installs tesseract 4 on CentOS 6
export PATH="$PATH:/usr/local/bin"
#yum -y groupinstall "development tools"
#yum -y install libpng-devel libtiff-devel libjpeg-devel
#yum -y install centos-release-scl
#yum -y install devtoolset-7-gcc-c++
#source /opt/rh/devtoolset-7/enable
#cd /usr/src/
#wget http://ftpmirror.gnu.org/autoconf-archive/autoconf-archive-2019.01.06.tar.xz
#tar xvvfJ autoconf-archive-2019.01.06.tar.xz
#cd autoconf-archive-2019.01.06/
#./configure --prefix=/usr
#make
#make install
#cd /usr/src/
#wget http://leptonica.org/source/leptonica-1.77.0.tar.gz
#tar xvvfz leptonica-1.77.0.tar.gz
#cd leptonica-1.77.0/
#./configure --prefix=/usr/local/
#make
#make install
#cd /usr/src/
#wget https://github.com/tesseract-ocr/tesseract/archive/4.1.0.tar.gz -O tesseract-4.1.0.tar.gz
#tar xvvfz tesseract-4.1.0.tar.gz
#cd tesseract-4.1.0
#wget -nc https://github.com/tesseract-ocr/tessdata_fast/blob/master/eng.traineddata
#wget -nc https://github.com/tesseract-ocr/tessdata_fast/blob/master/osd.traineddata
#cp   eng.traineddata osd.traineddata ./tessdata/
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#./autogen.sh
#./configure --prefix=/usr/local/ --with-extra-libraries=/usr/local/lib/
#make install
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

bash miniconda.sh -b
conda install -c conda-forge tesseract

# From here on, the script is building and testing our package
export PYHOME=/home
cd ${PYHOME}

/opt/python/cp37-cp37m/bin/pip install twine cmake
ln -s /opt/python/cp37-cp37m/bin/cmake /usr/bin/cmake

# Compile wheels
for PYBIN in /opt/python/cp3*/bin; do
    "${PYBIN}/pip" wheel /io/ -w wheelhouse/
    "${PYBIN}/python" /io/setup.py sdist -d /io/wheelhouse/
done

# Bundle external shared libraries into the wheels and fix naming
for whl in wheelhouse/*.whl; do
    auditwheel repair "$whl" -w /io/wheelhouse/
done

# Test
for PYBIN in /opt/python/cp3*/bin/; do
    "${PYBIN}/pip" install --no-index -f /io/wheelhouse pysseract
    "${PYBIN}/python" setup.py test
done

#  Upload
for WHEEL in /io/wheelhouse/pysseract*; do
    /opt/python/cp37-cp37m/bin/twine upload \
        --skip-existing \
        -u "${TWINE_USERNAME}" -p "${TWINE_PASSWORD}" \
        "${WHEEL}"
done
