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

PYBIND11_MODULE(_pysseract, m) {
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
          "return the default location Tesseract expects models to be located in");

    py::class_<TessBaseAPI>(m, "Pysseract", R"pbdoc(
        This is the main class for interacting with the Tesseract API. There are several ways to initialise this class. 
        The simplest way with all defaults and assuming that the English model is needed is as follows:
        
        .. code-block:: python

            import pysseract
            with pysseract.Pysseract() as t:
                t.SetImageFromPath('/Users/hxia/Downloads/001-helloworld.png')
                print(t.utf8Text)
        
        In order to set a language (such as French in this example), and model location ('/Users/shogg/models'), you might choose to initialise in the below manner instead:

        .. code-block:: python

            t = pysseract.Pysseract('/Users/shogg/models', 'fra')

        To initialise with French and Arabic you would run the following command:

        .. code-block:: python

            t = pysseract.Pysseract('/Users/shogg/models', 'fra+ara')

        It's also possible to set an OCR mode as well on initialisation, though there are ways to do that afterwards. 
        If for any reason you need to perform detailed initialization for the base API, you can use the third signature listed below.
        two conversions have been done as follows.
        As part of that initialisation signature, two type conversions will occur, as follows:

        1) char **configs, int configs_size <==> configsList
        
        2) const GenericVector<STRING> *vars_vec, const GenericVector<STRING> *vars_values <==> settingDict

        `configsList` is a list of files from which configuration variables can be read
        `settingDict` is a dict object containing parameters and their values to be fed to Tesseract.
        You will typically not need both of these things, in which case one can be set as an empty List (or Dict) as the case may be.
        
        For information about working with the results of analysis, please see the documentation for ResultIterator or Pysseract.IterAt
    )pbdoc")
        .def(py::init([]() {
            TessBaseAPI *api = new (TessBaseAPI);
            api->Init(nullptr, nullptr);
            return std::unique_ptr<TessBaseAPI>(api);
        }))
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
                     configs_[i] = const_cast<char *>(configs[i].c_str());
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
             py::arg("settingDict"), py::arg("setOnlyNonDebugParams"))
        .def("Clear", &TessBaseAPI::Clear, R"pbdoc(
        Free up recognition results and any stored image data, without actually
        freeing any recognition data that would be time-consuming to reload.
        Afterwards, you must call SetImage or TesseractRect before doing
        any Recognize or Get* operation.
    )pbdoc")
        .def("End", &TessBaseAPI::End,
             "Close down tesseract and free up all memory, after which the instance should not be reused.")
        .def("__enter__", [](TessBaseAPI &api) { return &api; }, "for use in `with` statement")
        .def("__exit__",
             [](TessBaseAPI &api, py::object exc_type, py::object exc_value, py::object traceback) { api.End(); },
             "for `with` statement; after exiting the `with` code block the instance is not reusable.")
        .def_property_readonly("dataPath", &TessBaseAPI::GetDatapath,
                               R"pbdoc(Read-only: Returns the path where Tesseract model objects are stored)pbdoc")
        .def_property(
            "pageSegMode", &TessBaseAPI::GetPageSegMode, &TessBaseAPI::SetPageSegMode,
            R"pbdoc(This attribute can be used to get or set the page segmentation mode used by the tesseract model)pbdoc")
        .def_property_readonly("utf8Text", &TessBaseAPI::GetUTF8Text,
                               R"pbdoc(Read-only: Return all identified text concatenated into a UTF-8 string)pbdoc")
        .def_property_readonly(
            "unlvText", &TessBaseAPI::GetUNLVText,
            R"pbdoc(Read-only: Return all identified text according to UNLV format Latin-1 with specific reject and suspect codes)pbdoc")
        .def("GetHOCRText", (char *(TessBaseAPI::*)(int)) & TessBaseAPI::GetHOCRText, py::arg("pagenum"),
             "Make an HTML-formatted string with hOCR. 'pagenum' is 0-based, appears as 1-based in results.")
        .def("GetTSVText", &TessBaseAPI::GetTSVText, py::arg("pagenum"),
             "Make a TSV-formatted string from the internal data structures. 'pagenum' is 0-based, appears as 1-based "
             "in results.")
#if TESSERACT_VERSION >= (4 << 16 | 1 << 8)
        .def("GetAltoText", (char *(TessBaseAPI::*)(int)) & TessBaseAPI::GetAltoText, py::arg("pagenum"),
             "Make an ALTO XML string from internal data. 'pagenum' is 0-based, appears as 1-based in results.")
        .def("GetLSTMBoxText", &TessBaseAPI::GetLSTMBoxText, py::arg("pagenum"),
             "Make a box file for LSTM training from the internal data structures. 'pagenum' is 0-based, appears as "
             "1-based in results.")
        .def("GetWordStrBoxText", &TessBaseAPI::GetWordStrBoxText, py::arg("pagenum"),
             "Make a string formatted in the same style as Tesseract training data. 'pagenum' is 0-based, appears as "
             "1-based in results.")
#endif
        .def("GetOsdText", &TessBaseAPI::GetOsdText, py::arg("pagenum"),
             "Recognised text is returned as UTF-8. 'pagenum' is 0-based, appears as 1-based in results.")
        .def("GetIterator",
             [](TessBaseAPI &api) {
                 api.Recognize(nullptr);
                 return api.GetIterator();
             },
             "Returns the iterator over boxes found in a given source image")
        .def("GetThresholdedImage",
             [](TessBaseAPI &api) {
                 if (!api.GetThresholdedImageScaleFactor()) {
                     throw std::runtime_error("Please call SetImage before retrieving the thresholded image.");
                 }
                 Pix *pix = api.GetThresholdedImage();
                 l_int32 format = pixChooseOutputFormat(pix);
                 l_uint8 *bytearr = NULL;
                 size_t size = 0;
                 pixWriteMem(&bytearr, &size, pix, format);
                 if (bytearr == nullptr) throw std::runtime_error("Error returning the thresholded image");
                 std::string byteStr(bytearr, bytearr + size);
                 pixDestroy(&pix);
                 return py::bytes(byteStr);
             },
             "Get a copy of the image Tesseract has after pre-processing is complete. This can only be called once you "
             "have called SetImage. Unlike the underlying function in Tesseract, this will return the image as a byte "
             "string rather than the underlying Leptonica Pix object.")
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

    py::enum_<OcrEngineMode>(m, "OcrEngineMode", "Enumeration of Engine Mode")
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
                                   if (box.x < 0 || box.y < 0 || box.w <= 0 || box.h <= 0) return false;
                                   return true;
                               },
                               "Returns a boolean indicating whether the dimensions of the box are valid");

    py::class_<ResultIterator>(m, "ResultIterator", R"pbdoc(
        
        Iterator that yields results for an image at chosen level. If you're familiar with C/C++ iterators, the methods of this class should look familiar.
        The returned iterator must be deleted after use, so ideally it should be used in a `with` block. 
        The iterator points to data held within the pysseract instance that spawned it, and therefore can only be used while that instance still exists 
        and has not been subjected to a call of Init, SetImage, Recognize, Clear, End or anything else that changes the internal PAGE_RES.

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
        )pbdoc",
                               py::dynamic_attr())
        .def("__enter__", [](ResultIterator &resIter) { return &resIter; }, "for use in `with` statement")
        .def("__exit__",
             [](ResultIterator &resIter, py::object exc_type, py::object exc_value, py::object traceback) {},
             "for `with` statement; after exiting the `with` code block the instance is not reusable.")
        .def("Begin", &ResultIterator::Begin,
             "Moves the iterator to point to the start of the page to begin an iteration")
        .def("Next", &ResultIterator::Next, py::arg("pageIterLv"),
             "Moves to the start of the next object at the given level in the page hierarchy in the appropriate "
             "reading order and returns false if the end of the page was reached. Note that using the SYMBOL level "
             "will skip non-text blocks, but all other PageIteratorLevel level values will visit each non-text "
             "block once. Think of non text blocks as containing a single para, with a single line, with a single "
             "imaginary word. Calls to Next with different levels may be freely intermixed. This function iterates "
             "words in right-to-left scripts correctly, if the appropriate language has been loaded into Tesseract.")
        .def("Empty", &ResultIterator::Empty, py::arg("pageIterLv"),
             "Returns a boolean flag indicating whether the iterator is empty at the given PageIteratorLevel")
#if TESSERACT_VERSION >= (4 << 16 | 1 << 8)
        .def("GetBestLSTMSymbolChoices", &ResultIterator::GetBestLSTMSymbolChoices,
             "Returns the LSTM choices for every LSTM timestep for the current word.")
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
             py::arg("pageIterLv"), "Returns the bounding box of the current item")
        .def("IsAtBeginningOf", &ResultIterator::IsAtBeginningOf, py::arg("pageIterLv"),
             "IsAtBeginningOf() returns whether we're at the logical beginning of the given level. (as opposed to "
             "ResultIterator's left-to-right top-to-bottom order).")
        .def("IsAtFinalElement", &ResultIterator::IsAtFinalElement, py::arg("pageIterLv"), py::arg("element"),
             "Implement PageIterator's IsAtFinalElement correctly in a BiDi context. For instance, "
             "IsAtFinalElement(RIL_PARA, RIL_WORD) returns whether we point at the last word in a paragraph.")
        .def("ParagraphIsLtr", &ResultIterator::ParagraphIsLtr,
             "Return whether the current paragraph's dominant reading direction is left-to-right (as opposed to "
             "right-to-left).")
        .def("BlanksBeforeWord", &ResultIterator::BlanksBeforeWord,
             "Returns whether there are any blank spaces before the start of the current text object")
        .def("GetUTF8Text", &ResultIterator::GetUTF8Text, py::arg("pageIterLv"),
             "Returns the text of the current object at the specified page hierarchy level in UTF-8 format")
        .def("Confidence", &ResultIterator::Confidence, py::arg("pageIterLv"),
             "Return the confidence level expressed by the model for the current object at the specified page "
             "hierarchy level");
/**
 * VERSION_INFO is set from setup.py
 **/
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
