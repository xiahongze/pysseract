#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pysseract.h>
#include <sstream>

namespace py = pybind11;
using tesseract::PageIteratorLevel;
using tesseract::PageSegMode;
using tesseract::ResultIterator;

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

    m.def("apiVersion", &tesseract::TessBaseAPI::Version, R"pbdoc(
        Tesseract API version as seen in the library
    )pbdoc");
    m.def("availableLanguages", &availableLanguages, R"pbdoc(
        return a list of available languages from TESSDATA_PREFIX
    )pbdoc");
    m.def("defaultDataPath", &defaultDataPath, R"pbdoc(
        return TESSDATA_PREFIX
    )pbdoc");

    py::class_<Pysseract>(m, "Pysseract", R"pbdoc(
        Pysseract
        ---------------------
        main class to interact with Tesseract API
    )pbdoc")
        .def(py::init<const char *, const char *>(), py::arg("datapath"), py::arg("language"))
        .def(py::init<>())
        .def("Clear", &Pysseract::Clear, R"pbdoc(
            Free up recognition results and any stored image data, without actually
            freeing any recognition data that would be time-consuming to reload.
            Afterwards, you must call SetImage or TesseractRect before doing
            any Recognize or Get* operation.
        )pbdoc")
        .def_property_readonly("dataPath", &Pysseract::GetDataPath)
        .def_property("pageSegMode", &Pysseract::GetPageSegMode, &Pysseract::SetPageSegMode)
        .def_property_readonly("utf8Text", &Pysseract::GetUTF8Text)
        .def_property_readonly("unlvText", &Pysseract::GetUNLVText)
        .def("GetHOCRText", &Pysseract::GetHOCRText, py::arg("pagenum"))
        .def("GetTSVText", &Pysseract::GetTSVText, py::arg("pagenum"))
        .def("GetAltoText", &Pysseract::GetAltoText, py::arg("pagenum"))
        .def("GetLSTMBoxText", &Pysseract::GetLSTMBoxText, py::arg("pagenum"))
        .def("GetWordStrBoxText", &Pysseract::GetWordStrBoxText, py::arg("pagenum"))
        .def("GetOsdText", &Pysseract::GetOsdText, py::arg("pagenum"))
        .def("GetIterator", &Pysseract::GetIterator)
        .def("SetSourceResolution", &Pysseract::SetSourceResolution, py::arg("ppi"))
        .def("SetImageFromPath", &Pysseract::SetImageFromPath, py::arg("imgpath"))
        .def("SetImageFromBytes", &Pysseract::SetImageFromBytes, py::arg("bytes"))
        .def("SetVariable", &Pysseract::SetVariable, py::arg(", name"), py::arg("value"),
             R"pbdoc(Note: Must be called after Init(). Only works for non-init variables.)pbdoc")
        .def("SetRectangle", &Pysseract::SetRectangle, py::arg("left"), py::arg("top"), py::arg("width"),
             py::arg("height"),
             R"pbdoc(Restrict recognition to a sub-rectangle of the image. Call after SetImage.)pbdoc");

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