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

