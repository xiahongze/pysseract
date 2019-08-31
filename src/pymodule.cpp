#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pysseract.h>

namespace py = pybind11;

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
        .def("Clear()", &Pysseract::Clear, R"pbdoc(
            Free up recognition results and any stored image data, without actually
            freeing any recognition data that would be time-consuming to reload.
            Afterwards, you must call SetImage or TesseractRect before doing
            any Recognize or Get* operation.
        )pbdoc")
        .def_property_readonly("dataPath", &Pysseract::GetDataPath)
        .def_property("pageSegMode", &Pysseract::GetPageSegMode, &Pysseract::SetPageSegMode)
        .def("SetSourceResolution", &Pysseract::SetSourceResolution, py::arg("ppi"))
        .def("SetImageFromPath", &Pysseract::SetImageFromPath, py::arg("imgpath"))
        .def("SetImageFromBytes", &Pysseract::SetImageFromBytes, py::arg("bytes"));

/**
 * VERSION_INFO is set from setup.py
 **/
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}