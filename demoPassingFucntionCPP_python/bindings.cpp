#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include "calculator.h"

namespace py = pybind11;

PYBIND11_MODULE(my_service, m) {
    m.doc() = "Calculator and DataService classes";

    py::class_<Calculator>(m, "Calculator")
        .def(py::init<>())
        .def("multiply", &Calculator::multiply);

    py::class_<DataService>(m, "DataService")
        .def(py::init<std::function<int(int,int)>>())
        .def("process", &DataService::process);
}
