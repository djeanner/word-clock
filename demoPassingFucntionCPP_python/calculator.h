#pragma once
#include <functional>

class Calculator {
public:
    int multiply(int x, int y) {
        return x * y;
    }
};

class DataService {
private:
    std::function<int(int, int)> processor_;

public:
    DataService(std::function<int(int, int)> processor) : processor_(processor) {}
    
    int process(int a, int b) {
        return processor_ ? processor_(a, b) : 0;
    }
};
