#!/bin/bash
set -e -x

yum -y install wget
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
wget https://repo.continuum.io/archive/Anaconda3-4.3.1-Linux-x86_64.sh
bash Anaconda3-4.3.1-Linux-x86_64.sh
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
