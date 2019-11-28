from _pysseract import *


def iterAt(self, level: PageIteratorLevel):
    r'''
    Returns a generator that allows you to iterate through all the results at a specified PageIteratorLevel

    :param PageIteratorLevel level: A PageIteratorLevel value indicating the level you want to iterate through results at
    :return: Generator[Tuple[BoundingBox, str, float], None, None]
    '''

    with self.GetIterator() as resIter:
        while not resIter.Empty(level):
            box = resIter.BoundingBox(level)
            text = resIter.GetUTF8Text(level)
            conf = resIter.Confidence(level)
            yield box, text, conf
            resIter.Next(level)


setattr(Pysseract, 'IterAt', iterAt)
