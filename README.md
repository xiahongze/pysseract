Pysseract
=========

[![Build Status](https://travis-ci.org/xiahongze/pysseract.svg?branch=master)](https://travis-ci.org/xiahongze/pysseract)
[![](https://img.shields.io/badge/python-3.5+-blue.svg)](https://www.python.org/download/releases/3.5.0/)
[![](https://readthedocs.org/projects/pysseract/badge/?version=latest)](https://pysseract.readthedocs.io/en/latest/?badge=latest)

A Python binding to [Tesseract API](https://github.com/tesseract-ocr/tesseract). Tesseract is an open-source tool made available by Google for Optical Character Recognition (OCR) - that is, getting a computer to read the text in an image. Tesseract allows you to perform this task at a number of levels of granularity (one character at a time, one word at a time, and so on), by segmenting the page in a number of different ways (by assuming the whole page is one lump of text, or one line, or sparsely located throughout the source image), and with a number of different language models including ones you have built (pre-built models are available at https://github.com/tesseract-ocr/tessdata among other places).

Pip 19.3.1 or greater is required if you're installing the wheel for this package, otherwise just install the source. On Linux, if you install the wheel Tesseract comes included. You will however need to provide the Tesseract models. An example of how you might do this with English on a linux system is as follows:

```bash
curl -O https://raw.githubusercontent.com/tesseract-ocr/tessdata_fast/4.0.0/eng.traineddata
mkdir -p /usr/local/share/tessdata/ && sudo mv eng.traineddata /usr/local/share/tessdata/ 
```

The reason the file is being put in to `/usr/local/share/tessdata/` is because that is the default value for `TESSDATA_PREFIX`, an environment variable that Tesseract uses to locate model files. You're free to override the value of `TESSDATA_PREFIX`, of course. 

[Documentation](https://pysseract.readthedocs.io/en/latest/pysseract.html) is hosted on *readthedocs*.

# Basic usage

In order to just get all the text from an image and concatenate it into a string, run the following:

```python
import pysseract
t = pysseract.Pysseract()
t.SetImageFromPath('tests/001-helloworld.png')
print(t.utf8Text)
```

If instead you want to iterate through the text boxes found in an image at the TEXTLINE level (coarser-grained than WORD, but also lower-level than BLOCK), then you might run the following:

```python
with pysseract.Pysseract() as t:
    boxes = []
    text = []
    conf = []
    LEVEL = pysseract.PageIteratorLevel.TEXTLINE
    for box, text, confidence in t.IterAt(LEVEL):
        lines.append(text)
        boxes.append(box)
        confidence.append(conf)
```

A third possibility is that you may want to control how exactly the image is segmented. This is done before instantiating a `ResultIterator`, as follows:

```python
with pysseract.Pysseract() as t:
    t.pageSegMode = pysseract.PageSegMode.SINGLE_BLOCK
    t.SetImageFromPath("002-quick-fox.jpg")
    t.SetSourceResolution(70)
    boxes = []
    text = []
    conf = []
    LEVEL = pysseract.PageIteratorLevel.TEXTLINE
    for box, text, confidence in t.IterAt(LEVEL):
        lines.append(text)
        boxes.append(box)
        confidence.append(conf)
```

Finally, if you want to work with the low-level iterator built into Tesseract, the below code will work for you. This is primarily intended for people who want fine-grain control when searching through the results. For instance, if you want to look at the first paragraph, jump to the next word, then the next block after that, then the next symbol after that, you would use this approach:

```python
t = pysseract.Pysseract()
t.SetImageFromPath("002-quick-fox.jpg")
resIter = t.GetIterator()
boxes = []
lines = []
confidence = []

# First, look at the paragraph level
level = pysseract.PageIteratorLevel.PARA
boxes.append(resIter.BoundingBox(level))
lines.append(resIter.GetUTF8Text(level))
confidence.append(resIter.Confidence(level))

# Now the next word after the paragraph we just looked at
level = pysseract.PageIteratorLevel.WORD
resIter.Next(level)
boxes.append(resIter.BoundingBox(level))
lines.append(resIter.GetUTF8Text(level))
confidence.append(resIter.Confidence(level))

# Now the next block
level = pysseract.PageIteratorLevel.BLOCK
resIter.Next(level)
boxes.append(resIter.BoundingBox(level))
lines.append(resIter.GetUTF8Text(level))
confidence.append(resIter.Confidence(level))

# Lastly, look at the next symbol after the block we just looked at
level = pysseract.PageIteratorLevel.SYMBOL
resIter.Next(level)
boxes.append(resIter.BoundingBox(level))
lines.append(resIter.GetUTF8Text(level))
confidence.append(resIter.Confidence(level))
```

# Building the package

Requirements

- gcc/clang with at least c++11 support
- libtesseract, libtesseract-dev (equivalent on non-Debian/Ubuntu systems)
- pybind11>=2.2

```bash
python3 setup.py build install test
```

# Building the documentation

```bash
pip install sphinx sphinx_rtd_theme m2r
python3 setup.py build_sphinx
```

You should find the generated html in `build/sphinx`.

# Contribute

Look at [Tesseract BaseAPI](https://github.com/tesseract-ocr/tesseract/blob/master/src/api/baseapi.cpp)
and import those functions of interest to `pymodule.cpp`.

Please write a brief description in your wrapper function like those already in `pymodule.cpp`.

# Reference

- [basic pybind11](https://pybind11.readthedocs.io/en/master/basics.html)
- [class based pybind11](https://pybind11.readthedocs.io/en/master/classes.html)
- [compiling with pybind11](https://pybind11.readthedocs.io/en/master/compiling.html)

# LICENSE

MIT
