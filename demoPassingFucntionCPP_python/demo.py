import my_service

# Use the C++ Calculator
calc = my_service.Calculator()
print("C++ Calculator multiply:", calc.multiply(3, 4))  # 12

# Use C++ DataService with Calculator
service = my_service.DataService(calc.multiply)
print("DataService with C++ multiply:", service.process(3, 4))  # 12

# Mix with Python class
class PythonCalc:
    def multiply(self, x, y):
        return x * y

py_calc = PythonCalc()
service2 = my_service.DataService(py_calc.multiply)
print("DataService with Python multiply:", service2.process(5, 6))  # 30
