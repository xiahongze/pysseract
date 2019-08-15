#include <pysseract.h>
#include <tesseract/genericvector.h>
#include <iostream>

int add(int i, int j) { return i + j; }

const char* Pysseract::Version() { return tesseract::TessBaseAPI::Version(); }

const std::vector<std::string> Pysseract::AvailableLanguages() {
    tesseract::TessBaseAPI api;
    api.Init(nullptr, nullptr);
    GenericVector<STRING> glangs;
    api.GetAvailableLanguagesAsVector(&glangs);
    std::vector<std::string> langs(glangs.size());
    for (int i = 0; i < glangs.size(); i++) {
        langs[i] = glangs[i].c_str();
    }
    return langs;
}