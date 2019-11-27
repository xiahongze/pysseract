from pathlib import Path
from unittest import TestCase

import pysseract


class PysseractTest(TestCase):

    thisPath = Path(__file__)

    def testNull(self):
        self.assertTrue(True)

    def testUTF8Text(self):
        t = pysseract.Pysseract()
        t.SetImageFromPath(self.thisPath.with_name(
            "001-helloworld.png").as_posix())
        self.assertEqual("Hello, World!\n", t.utf8Text)

    def testResultIter(self):
        t = pysseract.Pysseract()
        t.pageSegMode = pysseract.PageSegMode.SINGLE_BLOCK
        t.SetImageFromPath(self.thisPath.with_name(
            "002-quick-fox.jpg").as_posix())
        t.SetSourceResolution(70)
        resIter = t.GetIterator()
        LEVEL = pysseract.PageIteratorLevel.TEXTLINE
        lines = []
        boxes = []
        for box, text in t.IterAt(LEVEL):
            lines.append(text)
            boxes.append(box)
        self.assertListEqual(
            lines, ['The quick brown\n', 'fox jumps over\n', 'the lazy dog.\n'])

    def testNews(self):
        t = pysseract.Pysseract()
        t.pageSegMode = pysseract.PageSegMode.SINGLE_BLOCK
        t.SetImageFromPath(self.thisPath.with_name(
            '003-skynews.png').as_posix())
        t.SetSourceResolution(70)
        resIter = t.GetIterator()
        LEVEL = pysseract.PageIteratorLevel.TEXTLINE
        lines = []
        boxes = []
        for box, text in t.IterAt(LEVEL):
            lines.append(text)
            boxes.append(box)
        self.assertEqual(len(boxes), 8)
        self.assertTrue('HEADLINE' in lines[-1])
