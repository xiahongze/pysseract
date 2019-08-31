#pragma once
#include <tesseract/baseapi.h>
#include <vector>

std::vector<std::string> availableLanguages();
const char* defaultDataPath();

class Pysseract {
   public:
    Pysseract(const char* datapath, const char* language);

   private:
    tesseract::TessBaseAPI api;
};