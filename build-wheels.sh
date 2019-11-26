#!/bin/bash
set -e -x

export PYHOME=/home
cd io

/opt/python/cp37-cp37m/bin/pip install twine cmake 
ln -s /opt/python/cp37-cp37m/bin/cmake /usr/bin/cmake

# Compile wheels
for PYBIN in /opt/python/cp3*/bin; do
    "${PYBIN}/pip" install pybind11
    "${PYBIN}/pip" install --upgrade setuptools
    "${PYBIN}/pip" wheel /io/ -w ${PYHOME}/wheelhouse/
    "${PYBIN}/python" /io/setup.py sdist -d /io/wheelhouse/
done

# Bundle external shared libraries into the wheels and fix naming
for whl in ${PYHOME}/wheelhouse/*.whl; do
    auditwheel repair "$whl" -w /io/wheelhouse/
done

# Test
for PYBIN in /opt/python/cp3*/bin/; do
    "${PYBIN}/pip" install --no-index -f /io/wheelhouse pysseract
    "${PYBIN}/python" setup.py test
done

#  Upload to test pypi and rename if TRAVIS_TAG not defined
for WHEEL in /io/wheelhouse/pysseract*; do
    /opt/python/cp37-cp37m/bin/twine upload \
        --repository-url https://test.pypi.org/legacy/ \
        --skip-existing \
        -u "${TWINE_USERNAME}" -p "${TWINE_PASSWORD}" \
        "${WHEEL}"
    if [ -z "${TRAVIS_TAG}" ]; then
        newfile=$(echo "${WHEEL}" | sed 's/[0-9]*\.[0-9]*\.[0-9]*//g')
        mv "${WHEEL}" $newfile
    fi
done
