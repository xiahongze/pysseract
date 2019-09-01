#include <leptonica/allheaders.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tesseract/baseapi.h>
#include <tesseract/genericvector.h>
#include <sstream>

namespace py = pybind11;
using tesseract::PageIteratorLevel;
using tesseract::PageSegMode;
using tesseract::ResultIterator;
using tesseract::TessBaseAPI;

PYBIND11_MODULE(pysseract, m) {
    m.doc() = R"pbdoc(
        Pysseract
        -----------------------
        .. currentmodule:: pysseract
        .. autosummary::
           :toctree: _generate
           Pysseract
           apiVersion
           availableLanguages
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
        Pysseract
        ---------------------
        main class to interact with Tesseract API
    )pbdoc")
        .def(py::init([](const char *datapath, const char *language) {
                 TessBaseAPI *api = new (TessBaseAPI);
                 api->Init(datapath, language);
                 return std::unique_ptr<TessBaseAPI>(api);
             }),
             py::arg("datapath"), py::arg("language"))
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
        .def("GetAltoText", (char *(TessBaseAPI::*)(int)) & TessBaseAPI::GetAltoText, py::arg("pagenum"))
        .def("GetLSTMBoxText", &TessBaseAPI::GetLSTMBoxText, py::arg("pagenum"))
        .def("GetWordStrBoxText", &TessBaseAPI::GetWordStrBoxText, py::arg("pagenum"))
        .def("GetOsdText", &TessBaseAPI::GetOsdText, py::arg("pagenum"))
        .def("GetIterator",
             [](TessBaseAPI &api) {
                 api.Recognize(nullptr);
                 return api.GetIterator();
             })
        .def("SetSourceResolution", &TessBaseAPI::SetSourceResolution, py::arg("ppi"))
        .def("SetImageFromPath",
             [](TessBaseAPI &api, const char *imgpath) {
                 Pix *image = pixRead(imgpath);
                 api.SetImage(image);
             },
             py::arg("imgpath"))
        .def("SetImageFromBytes",
             [](TessBaseAPI &api, const std::string &bytes) {
                 Pix *image = pixReadMem((unsigned char *)bytes.data(), bytes.size());
                 api.SetImage(image);
             },
             py::arg("bytes"))
        .def("SetVariable", &TessBaseAPI::SetVariable, py::arg("name"), py::arg("value"),
             "Note: Must be called after Init(). Only works for non-init variables.")
        .def("SetRectangle", &TessBaseAPI::SetRectangle, py::arg("left"), py::arg("top"), py::arg("width"),
             py::arg("height"), "Restrict recognition to a sub-rectangle of the image. Call after SetImage.");

    py::enum_<PageIteratorLevel>(m, "PageIteratorLevel")
        .value("BLOCK", PageIteratorLevel::RIL_BLOCK)
        .value("PARA", PageIteratorLevel::RIL_PARA)
        .value("TEXTLINE", PageIteratorLevel::RIL_TEXTLINE)
        .value("WORD", PageIteratorLevel::RIL_WORD)
        .value("SYMBOL", PageIteratorLevel::RIL_SYMBOL);

    py::enum_<PageSegMode>(m, "PageSegMode")
        .value("OSD_ONLY", PageSegMode::PSM_OSD_ONLY)
        .value("AUTO_OSD", PageSegMode::PSM_AUTO_OSD)
        .value("AUTO_ONLY", PageSegMode::PSM_AUTO_ONLY)
        .value("AUTO", PageSegMode::PSM_AUTO)
        .value("SINGLE_COLUMN", PageSegMode::PSM_SINGLE_COLUMN)
        .value("SINGLE_BLOCK_VERT_TEXT", PageSegMode::PSM_SINGLE_BLOCK_VERT_TEXT)
        .value("SINGLE_BLOCK", PageSegMode::PSM_SINGLE_BLOCK)
        .value("SINGLE_LINE", PageSegMode::PSM_SINGLE_LINE)
        .value("SINGLE_WORD", PageSegMode::PSM_SINGLE_WORD)
        .value("CIRCLE_WORD", PageSegMode::PSM_CIRCLE_WORD)
        .value("SINGLE_CHAR", PageSegMode::PSM_SINGLE_CHAR)
        .value("SPARSE_TEXT", PageSegMode::PSM_SPARSE_TEXT)
        .value("SPARSE_TEXT_OSD", PageSegMode::PSM_SPARSE_TEXT_OSD)
        .value("RAW_LINE", PageSegMode::PSM_RAW_LINE)
        .value("COUNT", PageSegMode::PSM_COUNT);

    py::class_<Box>(m, "Box", R"pbdoc(individual bounding box)pbdoc")
        .def_readonly("left", &Box::x)
        .def_readonly("top", &Box::y)
        .def_readonly("width", &Box::w)
        .def_readonly("height", &Box::h)
        .def("__repr__",
             [](const Box &box) {
                 std::ostringstream os;
                 os << "class Box<left:" << box.x << ",top:" << box.y << ",width:" << box.w << ",height:" << box.h
                    << ">";
                 return os.str();
             })
        .def_property_readonly("valid", [](const Box &box) {
            if (box.x < 0 || box.y <= 0 || box.w <= 0 || box.h <= 0) return false;
            return true;
        });

    py::class_<ResultIterator>(m, "ResultIterator")
        .def("Begin", &ResultIterator::Begin)
        .def("Next", &ResultIterator::Next)
        .def("Empty", &ResultIterator::Empty)
        .def("BoundingBox",
             [](const ResultIterator &ri, const PageIteratorLevel &lv) {
                 Box box;
                 // it is actually left, bottom, right, top
                 ri.BoundingBox(lv, &box.x, &box.h, &box.w, &box.y);
                 box.w -= box.x;
                 box.h = box.y - box.h;
                 return box;
             },
             py::arg("pageIteratorLevel"))
        .def("IsAtBeginningOf", &ResultIterator::IsAtBeginningOf)
        .def("IsAtFinalElement", &ResultIterator::IsAtFinalElement)
        .def("ParagraphIsLtr", &ResultIterator::ParagraphIsLtr)
        .def("BlanksBeforeWord", &ResultIterator::BlanksBeforeWord)
        .def("GetUTF8Text", &ResultIterator::GetUTF8Text)
        .def("Confidence", &ResultIterator::Confidence)
        .def("GetBestLSTMSymbolChoices", &ResultIterator::GetBestLSTMSymbolChoices);
/**
 * VERSION_INFO is set from setup.py
 **/
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}