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

Pysseract::Pysseract(const char* datapath, const char* language) { api.Init(datapath, language); }
Pysseract::Pysseract() { api.Init(nullptr, nullptr); }
const int Pysseract::GetPageSegMode() { return api.GetPageSegMode(); }
void Pysseract::SetPageSegMode(const int mode) { api.SetPageSegMode(tesseract::PageSegMode(mode)); }
const char* Pysseract::GetDataPath() { return api.GetDatapath(); }
void Pysseract::SetSourceResolution(const int ppi) { api.SetSourceResolution(ppi); }
void Pysseract::SetImageFromPath(const char* imgpath) {
    Pix* image = pixRead(imgpath);
    api.SetImage(image);
}
void Pysseract::SetImageFromBytes(const std::string& bytes) {
    char* chars = (char*)bytes.data();
    Pix* image = pixReadMem(reinterpret_cast<unsigned char*>(chars), bytes.size());
    api.SetImage(image);
}