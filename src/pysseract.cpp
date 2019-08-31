#include <pysseract.h>
#include <tesseract/genericvector.h>

std::vector<std::string> availableLanguages() {
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

const char* defaultDataPath() {
    tesseract::TessBaseAPI api;
    api.Init(nullptr, nullptr);
    return api.GetDatapath();
}

Pysseract::Pysseract(const char* datapath, const char* languages) { api.Init(datapath, languages); }