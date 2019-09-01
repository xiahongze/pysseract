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
void Pysseract::Clear() { api.Clear(); }

tesseract::PageSegMode Pysseract::GetPageSegMode() { return api.GetPageSegMode(); }
void Pysseract::SetPageSegMode(tesseract::PageSegMode mode) { api.SetPageSegMode(mode); }
const char* Pysseract::GetDataPath() { return api.GetDatapath(); }
void Pysseract::SetSourceResolution(const int ppi) { api.SetSourceResolution(ppi); }
void Pysseract::SetImageFromPath(const char* imgpath) {
    Pix* image = pixRead(imgpath);
    api.SetImage(image);
}
void Pysseract::SetImageFromBytes(const std::string& bytes) {
    Pix* image = pixReadMem((unsigned char*)bytes.data(), bytes.size());
    api.SetImage(image);
}
void Pysseract::SetRectangle(int left, int top, int width, int height) { api.SetRectangle(left, top, width, height); }
bool Pysseract::SetVariable(const char* name, const char* value) { return api.SetVariable(name, value); }

const char* Pysseract::GetUTF8Text() { return api.GetUTF8Text(); }
const char* Pysseract::GetUNLVText() { return api.GetUNLVText(); }
const char* Pysseract::GetHOCRText(const int pagenum) { return api.GetHOCRText(pagenum); }
const char* Pysseract::GetTSVText(const int pagenum) { return api.GetTSVText(pagenum); }
const char* Pysseract::GetAltoText(const int pagenum) { return api.GetAltoText(pagenum); }
const char* Pysseract::GetLSTMBoxText(const int pagenum) { return api.GetLSTMBoxText(pagenum); }
const char* Pysseract::GetBoxText(const int pagenum) { return api.GetBoxText(pagenum); }
const char* Pysseract::GetWordStrBoxText(const int pagenum) { return api.GetWordStrBoxText(pagenum); }
const char* Pysseract::GetOsdText(const int pagenum) { return api.GetOsdText(pagenum); }
std::unique_ptr<tesseract::ResultIterator> Pysseract::GetIterator() {
    if (api.Recognize(nullptr) < 0) {
        return nullptr;
    }
    return std::unique_ptr<tesseract::ResultIterator>(api.GetIterator());
};