#pragma once
#include <tesseract/baseapi.h>
#include <vector>

std::vector<std::string> availableLanguages();

class Pysseract {
   public:
    Pysseract();

   private:
    tesseract::TessBaseAPI api;
};