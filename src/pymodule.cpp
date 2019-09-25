#include <leptonica/allheaders.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>
#include <sstream>

namespace py = pybind11;
using tesseract::OcrEngineMode;
using tesseract::PageIteratorLevel;
using tesseract::PageSegMode;
using tesseract::ResultIterator;
using tesseract::TessBaseAPI;

PYBIND11_MODULE(pysseract, m) {
    m.doc() = R"pbdoc(
        pysseract
        -----------------------
        .. currentmodule:: pysseract
        .. autosummary::
        
           :toctree: _generate
           Pysseract
           Box
           ResultIterator
           PageIteratorLevel
           PageSegMode
           OcrEngineMode
           apiVersion
           availableLanguages
           defaultDataPath

    )pbdoc";

    m.def("apiVersion", &tesseract::TessBaseAPI::Version, "Tesseract API version as seen in the library");
    m.def("availableLanguages",
          []() {
              tesseract::TessBaseAPI api;
              api.Init(nullptr, nullptr);
              GenericVector<STRING> glangs;
              api.GetAvailableLanguagesAsVector(&glangs);
              std::vector<std::string> langs(glangs.size());
              for (int i = 0; i < glangs.size(); i++) {
                  langs[i] = glangs[i].c_str();
              }
              return langs;
          },
          "return a list of available languages from TESSDATA_PREFIX");
    m.def("defaultDataPath",
          []() {
              tesseract::TessBaseAPI api;
              api.Init(nullptr, nullptr);
              return api.GetDatapath();
          },
          "return TESSDATA_PREFIX");

    py::class_<TessBaseAPI>(m, "Pysseract", R"pbdoc(
        This is the main class for interacting with the Tesseract API
        
        .. code-block:: python

            import pysseract
            t = pysseract.Pysseract()
            t.SetImageFromPath('/Users/hxia/Downloads/001-helloworld.png')
            print(t.utf8Text)
            t.End()
        
        For advanced use cases, please consult ResultIterator.
    )pbdoc")
        .def(py::init([](const char *datapath, const char *language) {
                 TessBaseAPI *api = new (TessBaseAPI);
                 api->Init(datapath, language);
                 return std::unique_ptr<TessBaseAPI>(api);
             }),
             py::arg("datapath"), py::arg("language"))
        .def(py::init([](const char *datapath, const char *language, OcrEngineMode mode) {
                 TessBaseAPI *api = new (TessBaseAPI);
                 api->Init(datapath, language, mode);
                 return std::unique_ptr<TessBaseAPI>(api);
             }),
             py::arg("datapath"), py::arg("language"), py::arg("engineMode"))
        .def(py::init([](const char *datapath, const char *language, OcrEngineMode mode,
                         std::vector<std::string> configs, std::unordered_map<std::string, std::string> settings,
                         bool set_only_non_debug_params) {
                 TessBaseAPI *api = new (TessBaseAPI);

                 char *configs_[configs.size()];
                 for (size_t i = 0; i < configs.size(); i++) {
                     configs[i] = configs[i].c_str();
                 }

                 GenericVector<STRING> vars_vec;
                 GenericVector<STRING> vars_values;
                 for (auto &&entry : settings) {
                     vars_vec.push_back(STRING(entry.first.c_str()));
                     vars_values.push_back(STRING(entry.second.c_str()));
                 }

                 api->Init(datapath, language, mode, configs_, configs.size(), &vars_vec, &vars_values,
                           set_only_non_debug_params);
                 return std::unique_ptr<TessBaseAPI>(api);
             }),
             py::arg("datapath"), py::arg("language"), py::arg("engineMode"), py::arg("configsList"),
             py::arg("settingDict"), py::arg("setOnlyNonDebugParams"), R"pbdoc(
                 detailed initialization for the base API, two conversions have been done as followed
                 char **configs, int configs_size <==> configsList
                 const GenericVector<STRING> *vars_vec, const GenericVector<STRING> *vars_values <==> settingDict
                 )pbdoc")
        .def(py::init([]() {
            TessBaseAPI *api = new (TessBaseAPI);
            api->Init(nullptr, nullptr);
            return std::unique_ptr<TessBaseAPI>(api);
        }))
        .def("Clear", &TessBaseAPI::Clear, R"pbdoc(
        Free up recognition results and any stored image data, without actually
        freeing any recognition data that would be time-consuming to reload.
        Afterwards, you must call SetImage or TesseractRect before doing
        any Recognize or Get* operation.
    )pbdoc")
        .def("End", &TessBaseAPI::End,
             "Close down tesseract and free up all memory, after which the instance should not be reused.")
        .def("__enter__", [](TessBaseAPI &api) { return &api; }, "for with statement")
        .def("__exit__",
             [](TessBaseAPI &api, py::object exc_type, py::object exc_value, py::object traceback) { api.End(); },
             "for with statement, after exitting with, the instance is not reusable.")
        .def_property_readonly("dataPath", &TessBaseAPI::GetDatapath)
        .def_property("pageSegMode", &TessBaseAPI::GetPageSegMode, &TessBaseAPI::SetPageSegMode)
        .def_property_readonly("utf8Text", &TessBaseAPI::GetUTF8Text)
        .def_property_readonly("unlvText", &TessBaseAPI::GetUNLVText)
        .def("GetHOCRText", (char *(TessBaseAPI::*)(int)) & TessBaseAPI::GetHOCRText, py::arg("pagenum"))
        .def("GetTSVText", &TessBaseAPI::GetTSVText, py::arg("pagenum"))
#if TESSERACT_VERSION >= (4 << 16 | 1 << 8)
        .def("GetAltoText", (char *(TessBaseAPI::*)(int)) & TessBaseAPI::GetAltoText, py::arg("pagenum"))
        .def("GetLSTMBoxText", &TessBaseAPI::GetLSTMBoxText, py::arg("pagenum"))
        .def("GetWordStrBoxText", &TessBaseAPI::GetWordStrBoxText, py::arg("pagenum"))
#endif
        .def("GetOsdText", &TessBaseAPI::GetOsdText, py::arg("pagenum"))
        .def("GetIterator",
             [](TessBaseAPI &api) {
                 api.Recognize(nullptr);
                 return api.GetIterator();
             },
             "Returns the iterator over boxes found in a given source image")
        .def("GetVariableAsString",
             [](TessBaseAPI &api, const char *name) {
                 STRING str;
                 bool res = api.GetVariableAsString(name, &str);
                 if (res) {
                     return str.c_str();
                 }
                 return "";
             },
             "Get value of named variable as a string, if it exists.")
        .def("SetSourceResolution", &TessBaseAPI::SetSourceResolution, py::arg("ppi"),
             "Set the pixel-per-inch value for the source image")
        .def("SetImageFromPath",
             [](TessBaseAPI &api, const char *imgpath) {
                 Pix *image = pixRead(imgpath);
                 api.SetImage(image);
             },
             py::arg("imgpath"), "Read an image from a given fully-qualified file path")
        .def("SetImageFromBytes",
             [](TessBaseAPI &api, const std::string &bytes) {
                 Pix *image = pixReadMem((unsigned char *)bytes.data(), bytes.size());
                 api.SetImage(image);
             },
             py::arg("bytes"), "Read an image from a string of bytes")
        .def("SetVariable", &TessBaseAPI::SetVariable, py::arg("name"), py::arg("value"),
             "Note: Must be called after Init(). Only works for non-init variables.")
        .def("SetRectangle", &TessBaseAPI::SetRectangle, py::arg("left"), py::arg("top"), py::arg("width"),
             py::arg("height"), "Restrict recognition to a sub-rectangle of the image. Call after SetImage.");

    py::enum_<PageIteratorLevel>(m, "PageIteratorLevel", "Enumeration of page iteration level settings")
        .value("BLOCK", PageIteratorLevel::RIL_BLOCK, "ResultIterator at the block level")
        .value("PARA", PageIteratorLevel::RIL_PARA, "ResultIterator at the paragraph level")
        .value("TEXTLINE", PageIteratorLevel::RIL_TEXTLINE, "ResultIterator at the text line level")
        .value("WORD", PageIteratorLevel::RIL_WORD, "ResultIterator at the word level")
        .value("SYMBOL", PageIteratorLevel::RIL_SYMBOL, "ResultIterator at the symbol level");

    py::enum_<PageSegMode>(m, "PageSegMode", "Enumeration of page segmentation settings")
        .value("OSD_ONLY", PageSegMode::PSM_OSD_ONLY, "Segment the page in \"OSD only\" mode")
        .value("AUTO_OSD", PageSegMode::PSM_AUTO_OSD, "Segment the page in \"Auto OSD\" mode")
        .value("AUTO_ONLY", PageSegMode::PSM_AUTO_ONLY, "Segment the page in \"Automatic only\" mode")
        .value("AUTO", PageSegMode::PSM_AUTO, "Segment the page in \"Automatic\" mode")
        .value("SINGLE_COLUMN", PageSegMode::PSM_SINGLE_COLUMN, "Segment the page in \"Single column\" mode")
        .value("SINGLE_BLOCK_VERT_TEXT", PageSegMode::PSM_SINGLE_BLOCK_VERT_TEXT,
               "Segment the page in \"Single block of vertical text\" mode")
        .value("SINGLE_BLOCK", PageSegMode::PSM_SINGLE_BLOCK, "Segment the page in \"Single block\" mode")
        .value("SINGLE_LINE", PageSegMode::PSM_SINGLE_LINE, "Segment the page in \"Single line\" mode")
        .value("SINGLE_WORD", PageSegMode::PSM_SINGLE_WORD, "Segment the page in \"Single word\" mode")
        .value("CIRCLE_WORD", PageSegMode::PSM_CIRCLE_WORD, "Segment the page in \"Circle word\" mode")
        .value("SINGLE_CHAR", PageSegMode::PSM_SINGLE_CHAR, "Segment the page in \"Single character\" mode")
        .value("SPARSE_TEXT", PageSegMode::PSM_SPARSE_TEXT, "Segment the page in \"Sparse text\" mode")
        .value("SPARSE_TEXT_OSD", PageSegMode::PSM_SPARSE_TEXT_OSD, "Segment the page in \"Sparse text OSD\" mode")
        .value("RAW_LINE", PageSegMode::PSM_RAW_LINE, "Segment the page in \"Raw line\" mode")
        .value("COUNT", PageSegMode::PSM_COUNT, "Segment the page in \"Count\" mode");

    py::enum_<OcrEngineMode>(m, "OcrEngineMode", "Enum of Engine Mode")
        .value("TESSERACT_ONLY", OcrEngineMode::OEM_TESSERACT_ONLY, "Run Tesseract only - fastest; deprecated")
        .value("LSTM_ONLY", OcrEngineMode::OEM_LSTM_ONLY, "Run just the LSTM line recognizer")
        .value("TESSERACT_LSTM_COMBINED", OcrEngineMode::OEM_TESSERACT_LSTM_COMBINED,
               "Run the LSTM recognizer, but allow fallback to Tesseract when things get difficult. deprecated")
        .value("DEFAULT", OcrEngineMode::OEM_DEFAULT, R"pbdoc(
            Specify this mode when calling init_*(),
            to indicate that any of the above modes
            should be automatically inferred from the
            variables in the language-specific config,
            command-line configs, or if not specified
            in any of the above should be set to the
            default OEM_TESSERACT_ONLY.
            )pbdoc")
        .value("COUNT", OcrEngineMode::OEM_COUNT, "Number of OEMs");

    py::class_<Box>(m, "Box", R"pbdoc(The bounding box structure)pbdoc")
        .def_readonly("left", &Box::x, "Leftmost co-ordinate of the box")
        .def_readonly("top", &Box::y, "Topmost co-ordinate of the box")
        .def_readonly("width", &Box::w, "Box width")
        .def_readonly("height", &Box::h, "Box height")
        .def("__repr__",
             [](const Box &box) {
                 std::ostringstream os;
                 os << "class Box<left:" << box.x << ",top:" << box.y << ",width:" << box.w << ",height:" << box.h
                    << ">";
                 return os.str();
             })
        .def_property_readonly("valid",
                               [](const Box &box) {
                                   if (box.x < 0 || box.y <= 0 || box.w <= 0 || box.h <= 0) return false;
                                   return true;
                               },
                               "Returns a boolean indicating whether the dimensions of the box are valid");

    py::class_<ResultIterator>(m, "ResultIterator", R"pbdoc(
        Internal Iterator that yields result at chosen level

        .. code-block:: python

            t: pysseract.Pysseract
            resIter = t.GetIterator()
            LEVEL = pysseract.PageIteratorLevel.TEXTLINE
            while True:
                box = resIter.BoundingBox(LEVEL)
                text = resIter.GetUTF8Text(LEVEL)
                if not resIter.Next(LEVEL):
                    break

        For more examples, please consult https://github.com/tesseract-ocr/tesseract/wiki/APIExample
        )pbdoc")
        .def("Begin", &ResultIterator::Begin)
        .def("Next", &ResultIterator::Next, py::arg("pageIterLv"))
        .def("Empty", &ResultIterator::Empty, py::arg("pageIterLv"))
#if TESSERACT_VERSION >= (4 << 16 | 1 << 8)
        .def("GetBestLSTMSymbolChoices", &ResultIterator::GetBestLSTMSymbolChoices)
#endif
        .def("BoundingBox",
             [](const ResultIterator &ri, const PageIteratorLevel &lv) {
                 Box box;
                 // it is actually left, bottom, right, top
                 ri.BoundingBox(lv, &box.x, &box.h, &box.w, &box.y);
                 box.w -= box.x;
                 box.h = box.y - box.h;
                 return box;
             },
             py::arg("pageIterLv"))
        .def("IsAtBeginningOf", &ResultIterator::IsAtBeginningOf, py::arg("pageIterLv"))
        .def("IsAtFinalElement", &ResultIterator::IsAtFinalElement, py::arg("pageIterLv"), py::arg("element"))
        .def("ParagraphIsLtr", &ResultIterator::ParagraphIsLtr)
        .def("BlanksBeforeWord", &ResultIterator::BlanksBeforeWord)
        .def("GetUTF8Text", &ResultIterator::GetUTF8Text, py::arg("pageIterLv"))
        .def("Confidence", &ResultIterator::Confidence, py::arg("pageIterLv"));
/**
 * VERSION_INFO is set from setup.py
 **/
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}