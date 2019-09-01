#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pysseract.h>

namespace py = pybind11;
using tesseract::PageIteratorLevel;
using tesseract::PageSegMode;

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
        .def(py::init<const char*, const char*>(), py::arg("datapath"), py::arg("language"))
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
        .def("SetSourceResolution", &Pysseract::SetSourceResolution, py::arg("ppi"))
        .def("SetImageFromPath", &Pysseract::SetImageFromPath, py::arg("imgpath"))
        .def("SetImageFromBytes", &Pysseract::SetImageFromBytes, py::arg("bytes"))
        .def("SetRectangle", &Pysseract::SetRectangle, py::arg("left"), py::arg("top"), py::arg("width"),
             py::arg("height"),
             R"pbdoc(Restrict recognition to a sub-rectangle of the image. Call after SetImage.)pbdoc");

    py::enum_<PageIteratorLevel>(m, "PageIteratorLevel")
        .value("RIL_BLOCK", PageIteratorLevel::RIL_BLOCK)
        .value("RIL_PARA", PageIteratorLevel::RIL_PARA)
        .value("RIL_TEXTLINE", PageIteratorLevel::RIL_TEXTLINE)
        .value("RIL_WORD", PageIteratorLevel::RIL_WORD)
        .value("RIL_SYMBOL", PageIteratorLevel::RIL_SYMBOL);

    py::enum_<PageSegMode>(m, "PageSegMode")
        .value("PSM_OSD_ONLY", PageSegMode::PSM_OSD_ONLY)
        .value("PSM_AUTO_OSD", PageSegMode::PSM_AUTO_OSD)
        .value("PSM_AUTO_ONLY", PageSegMode::PSM_AUTO_ONLY)
        .value("PSM_AUTO", PageSegMode::PSM_AUTO)
        .value("PSM_SINGLE_COLUMN", PageSegMode::PSM_SINGLE_COLUMN)
        .value("PSM_SINGLE_BLOCK_VERT_TEXT", PageSegMode::PSM_SINGLE_BLOCK_VERT_TEXT)
        .value("PSM_SINGLE_BLOCK", PageSegMode::PSM_SINGLE_BLOCK)
        .value("PSM_SINGLE_LINE", PageSegMode::PSM_SINGLE_LINE)
        .value("PSM_SINGLE_WORD", PageSegMode::PSM_SINGLE_WORD)
        .value("PSM_CIRCLE_WORD", PageSegMode::PSM_CIRCLE_WORD)
        .value("PSM_SINGLE_CHAR", PageSegMode::PSM_SINGLE_CHAR)
        .value("PSM_SPARSE_TEXT", PageSegMode::PSM_SPARSE_TEXT)
        .value("PSM_SPARSE_TEXT_OSD", PageSegMode::PSM_SPARSE_TEXT_OSD)
        .value("PSM_RAW_LINE", PageSegMode::PSM_RAW_LINE)
        .value("PSM_COUNT", PageSegMode::PSM_COUNT);

    py::class_<Box>(m, "Box", R"pbdoc(individual bounding box)pbdoc")
        .def_readonly("left", &Box::x)
        .def_readonly("top", &Box::y)
        .def_readonly("width", &Box::w)
        .def_readonly("height", &Box::h);
/**
 * VERSION_INFO is set from setup.py
 **/
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}