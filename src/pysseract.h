#pragma once
#include <tesseract/baseapi.h>
#include <vector>

int add(int i, int j);

class Pysseract {
   public:
    Pysseract();
    static const char* Version();
    static const std::vector<std::string> AvailableLanguages();

   private:
    tesseract::TessBaseAPI api;
};