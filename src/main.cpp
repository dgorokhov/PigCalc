#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>

#include "calculator.h"

//to do
//добавиь sqrt
// !! переполнение - контороль резуль операций и длины числе (number и пр.)
// long double a = /* ... */;
//long double b = /* ... */;
//long double result = a * b;
//

//UnaryMinus() : UnaryOperation("-", Priority::Low) //....низкий?

//!! 34e10 =340 переводит правильно, 34e+10 = 44 складывает
// 
// можно добавитть расчет точности подсчета +\-



static const char* prompt = " (..) > ";

int main()
{
    Calculator calc;
    std::string rpn;

    while (true) {
        try {
            std::cout << prompt;
            std::getline(std::cin, rpn);
            //удалитть пробельные символы
            rpn.erase(std::remove_if(rpn.begin(), rpn.end(),
                    [](unsigned char c) 
                    { return std::isspace(c); }), rpn.end()
            );
            if (rpn == "exit" || rpn == "quit") return 0;
            std::cout << rpn << std::endl;
            if (!rpn.empty()) calc.ToReversePolishNotation(rpn);
            if (calc.getRpnSize()) 
                std::cout << calc.Execute() << std::endl;
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << "   " << e.what() << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cout << "   " << "Unknown error. Try again.\n";
        }
    }
    std::cout << "\nBye!\n";
    return 0;

}
