from _pysseract import *


def iter_level(self, level: PageIteratorLevel):
    resIter = self.GetIterator()
    while not resIter.Empty(level):
        box = resIter.BoundingBox(level)
        text = resIter.GetUTF8Text(level)
        yield box, text
        resIter.Next(level)


setattr(Pysseract, 'iterAt', iter_level)
