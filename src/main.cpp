#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>

#include "calculator.h"

/* TODO
////////////////////////////////////////////////////////
x1) привести все к нижнему регистру
x2)  добавиь sqrt,arcsin,arc.. гиперболич функции
3)в процедуре slicenextnode изменить логику опреледеения следующего tokena
    (сейас проблема - 0xFF не опред-ся как шестнадц число)    
    если нескольько токенов удовлетв первым символам то беереся наиболе длинное совпадение

3a    EPS evaluates to 9.9999999999999998e-13.
4) !! сделать работу с науч нотацией\
    -вываод в НН, узнавание 34e10 =340 переводит правильно, 34e+10 = 44 складывает
5) рабта в других системах счисления - все операции в int?
6) точность расчета?

 !! переполнение - контороль резуль операций и длины числе (number и пр.)
long double a = 
long double b = 
long double result = a * b;

 
 можно добавитть расчет точности подсчета +\-
////////////////////////////////////////////////////////
*/


static const char* prompt = " (..) > ";

int main()
{
    Calculator calc;
    std::string rpn;

    while (true) {
        try {
            std::cout << prompt;
            std::getline(std::cin, rpn);
            //удалитть пробельные символы и нижний регистр
            calc.PreProcessInStr(rpn);
            if (rpn == "exit" || rpn == "quit") break;
            std::cout << rpn << std::endl;
            if (!rpn.empty()) calc.ToReversePolishNotation(rpn);
            if (calc.getRpnSize()) 
                std::cout << calc.Execute() << std::endl;
        }
       // 1. Specific logic errors (derived from std::logic_error)
       //ошибки - аргумент не из области определения функции ()
       catch (const std::domain_error& e) {
           std::cerr << "[Domain]: " << e.what() << '\n';
       }
       // Specific calculation errors (derived from std::runtime_error)
       catch (const std::overflow_error& e) {
           std::cerr << "[Overflow]: " << e.what() << '\n';
       }
       // 3. Base classes (strictly AFTER their derived classes)
       catch (const std::runtime_error& e) {
           std::cerr << "[General]: " << e.what() << '\n';
       }
       // 4. Most general catch-all handler
       catch (const std::exception& e) {
           std::cerr << "[Unknown Error]: " << e.what() << '\n';
       }
    }
    std::cout << "\nBye!\n";
    return 0;

}
