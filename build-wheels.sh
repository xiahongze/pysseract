#!/bin/bash
set -e -x
yum-config-manager --add-repo https://download.opensuse.org/repositories/home:/Alexander_Pozdnyakov/CentOS_7/
rpm --import https://build.opensuse.org/projects/home:Alexander_Pozdnyakov/public_key
yum update
yum -y install tesseract wget git
yum -y install tesseract-langpack-eng


# From https://www.tekovic.com/installing-tesseract-ocr-40-on-centos-6
# This code installs tesseract 4 on CentOS 6
#export PATH="$PATH:/usr/local/bin:/usr/bin"
#yum -y groupinstall "development tools"
#yum -y install libpng-devel libtiff-devel libjpeg-devel
#yum -y install centos-release-scl
#yum -y install devtoolset-7-gcc-c++
#source /opt/rh/devtoolset-8/enable
#cd /usr/src/
#wget --quiet http://ftpmirror.gnu.org/autoconf-archive/autoconf-archive-2019.01.06.tar.xz
#tar xfJ autoconf-archive-2019.01.06.tar.xz
#cd autoconf-archive-2019.01.06/
#./configure --prefix=/usr
#make
#make install
cd /usr/src
wget --quiet --no-check-certificate http://leptonica.org/source/leptonica-1.77.0.tar.gz
tar xfz leptonica-1.77.0.tar.gz
cd leptonica-1.77.0/
./configure --prefix=/usr/local/
make
make install
cd /usr/src/
wget --quiet https://github.com/tesseract-ocr/tesseract/archive/4.1.0.tar.gz -O tesseract-4.1.0.tar.gz
tar xfz tesseract-4.1.0.tar.gz
cd tesseract-4.1.0
mkdir include
mkdir include/tesseract
find ./src -name "*.h" -type f | xargs -I {} cp {} ./include/tesseract
cp ./src/api/tess_version.h.in ./include/tesseract/tess_version.h
#curl -L https://github.com/tesseract-ocr/tessdata_fast/blob/master/eng.traineddata >> ./eng.traineddata
#curl -L https://github.com/tesseract-ocr/tessdata_fast/blob/master/osd.traineddata >> ./osd.traineddata
#cp   eng.traineddata osd.traineddata ./tessdata/
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
#./autogen.sh
#./configure --prefix=/usr/local/ --with-extra-libraries=/usr/local/lib/
#make install
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
#mv /usr/src/leptonica-1.77.0/src /usr/src/leptonica-1.77.0/leptonica
#ln -s /usr/src/leptonica-1.77.0/leptonica /usr/local/include
#cp /usr/src/tesseract-4.1.0/include/tesseract /usr/local/include
#export CPLUS_INCLUDE_PATH=/usr/src/tesseract/include:${CPATH}
# From here on, the script is building and testing our package
export PYHOME=/home
cd ${PYHOME}

/opt/python/cp37-cp37m/bin/pip install twine cmake 
ln -s /opt/python/cp37-cp37m/bin/cmake /usr/bin/cmake

# Compile wheels
for PYBIN in /opt/python/cp37*/bin; do
    "${PYBIN}/pip" install pybind11
    "${PYBIN}/pip" wheel /io/ -w wheelhouse/
    "${PYBIN}/python" /io/setup.py sdist -d /io/wheelhouse/
done

# Bundle external shared libraries into the wheels and fix naming
for whl in wheelhouse/*.whl; do
    auditwheel repair "$whl" -w /io/wheelhouse/
done

# Test
for PYBIN in /opt/python/cp37*/bin/; do
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
