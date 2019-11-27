from _pysseract import *


def iterAt(self, level: PageIteratorLevel):
    r'''
    Returns a generator that allows you to iterate through all the results at a specified PageIteratorLevel

    :param PageIteratorLevel level: A PageIteratorLevel value indicating the level you want to iterate through results at
    :return: Generator[Tuple[BoundingBox, str], None, None]
    '''

    resIter = self.GetIterator()
    while not resIter.Empty(level):
        box = resIter.BoundingBox(level)
        text = resIter.GetUTF8Text(level)
        yield box, text
        resIter.Next(level)


setattr(Pysseract, 'IterAt', iterAt)
