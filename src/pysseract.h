#pragma once
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>
#include <vector>

std::vector<std::string> availableLanguages();
const char* defaultDataPath();

class Pysseract {
   public:
    Pysseract(const char* datapath, const char* language);
    Pysseract();
    void Clear();

    void SetPageSegMode(const int mode);
    const int GetPageSegMode();
    const char* GetDataPath();
    void SetSourceResolution(const int ppi);
    void SetImageFromPath(const char* imgpath);
    void SetImageFromBytes(const std::string& bytes);

   private:
    tesseract::TessBaseAPI api;
};