#include <iostream>
#include <memory>
#include <algorithm>
#include <cctype>

#include "calculator.h"

/* TODO
////////////////////////////////////////////////////////
ОК x1) привести все к нижнему регистру
ОК 2)  добавиь sqrt,arcsin,arc.. гиперболич функции
OK 3)в процедуре slicenextnode изменить логику опреледеения следующего tokena
    (сейас проблема - 0xFF не опред-ся как шестнадц число)    
    если нескольько токенов удовлетв первым символам то беереся наиболе длинное совпадение
OK    !! сделать работу с науч нотацией\

!тесты для slice, ...exec , topolishnotation
Если позиционировать как научный то musthave
    !2a)radiand/degreee
!редактирование строки, history
!не спотык-ся на utf
=vars

comamds: help

5) рабта в других системах счисления - все операции в int?
6) точность расчета?

  
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
        catch (const std::overflow_error& e) {
            std::cerr << "[Overflow]: " << e.what() << '\n';
        }
        // Главным образом ошибки парсинга
        catch (const std::runtime_error& e) {
            std::cerr << "[General]: " << e.what() << '\n';
        }
        //все остальное
        catch (const std::exception& e) {
            std::cerr << "[Unknown Error]: " << e.what() << '\n';
        }
    }
    std::cout << "\nBye!\n";
    return 0;

}
