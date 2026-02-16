# My Service Python Module

This Python package exposes **C++ classes** `Calculator` and `DataService` via **pybind11**. It works on macOS (including M1/M2) and allows Python to seamlessly call both C++ and Python functions.

---

## Installation

Make sure you have Python ≥ 3.8 and `pybind11` installed.

1. Clean old builds (if any):

```bash
rm -rf build *.egg-info
````

2. Set system compilers on macOS:

```bash
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
```

3. Install the package:

```bash
pip install --force-reinstall .
```

---

## Usage

```python
import my_service

# C++ Calculator
calc = my_service.Calculator()
print("C++ multiply:", calc.multiply(3, 4))  # 12

# C++ DataService with C++ method
service = my_service.DataService(calc.multiply)
print("DataService with C++ multiply:", service.process(3, 4))  # 12

# DataService with Python callable
class PythonCalc:
    def multiply(self, x, y):
        return x * y

py_calc = PythonCalc()
service2 = my_service.DataService(py_calc.multiply)
print("DataService with Python multiply:", service2.process(5, 6))  # 30
```

---

## Classes

### `Calculator`

A simple C++ class for multiplication.

**Methods:**

* `multiply(x: int, y: int) -> int` — Returns `x * y`.

### `DataService`

A C++ class that processes two integers using a callable.

**Constructor:**

* `DataService(processor: Callable[[int, int], int])`

**Methods:**

* `process(a: int, b: int) -> int` — Calls the provided processor with `(a, b)`.

---

## Notes

* This package uses **pybind11** for binding C++ classes to Python.
* On **macOS**, it requires:

```text
extra_link_args = ["-undefined", "dynamic_lookup"]
```

to ensure the Python init symbol `PyInit_my_service` is exported.

* Works seamlessly with both **C++ methods** and **Python callables**.

## Implementation in C:

```cpp
#include <iostream>
#include "calculator.h"

int main() {
    // Create a Calculator instance
    Calculator calc;
    int result = calc.multiply(3, 4);
    std::cout << "C++ Calculator multiply: " << result << std::endl;  // 12

    // Create a DataService with a C++ lambda function
    DataService service([](int a, int b) {
        return a * b;
    });

    int processed = service.process(5, 6);
    std::cout << "DataService with lambda multiply: " << processed << std::endl;  // 30

    // Create a DataService using Calculator's member function
    DataService service2([&calc](int a, int b) {
        return calc.multiply(a, b);
    });

    std::cout << "DataService with Calculator multiply: " << service2.process(7, 8) << std::endl;  // 56

    return 0;
}
```
compile:

```zsh
export PATH=/usr/bin:/bin:/usr/sbin:/sbin:$PATH
clang++ demo.cpp -std=c++17 -o demo
./demo
```

