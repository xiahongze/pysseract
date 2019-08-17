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

    py::class_<Pysseract>(m, "Pysseract", R"pbdoc(
        Pysseract
        ---------------------
        main class to interact with Tesseract API
    )pbdoc");

/**
 * VERSION_INFO is set from setup.py
 **/
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}