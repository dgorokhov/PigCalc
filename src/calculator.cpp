#include <iostream>
#include <stdexcept>
#include <cmath>
#include <string>
#include <vector>
#include <stack>
#include <memory>
//#include <string_view>
#include <print>
#include "calculator.h"




/* Класс "Калькулятор"
 * 
 *
 * */

Calculator::Calculator() 
{
    //base
    //nodetypes.emplace_back(std::make_unique<StartingNode>());
    nodetypes.emplace_back(std::make_unique<Number>());
    nodetypes.emplace_back(std::make_unique<Addition>());
    nodetypes.emplace_back(std::make_unique<Substraction>());
    nodetypes.emplace_back(std::make_unique<Multiplication>());
    nodetypes.emplace_back(std::make_unique<Division>());
    nodetypes.emplace_back(std::make_unique<CompoundStart>("(", 0));
    nodetypes.emplace_back(std::make_unique<CompoundEnd>(")"));
    nodetypes.emplace_back(std::make_unique<OperandDelimiter>(","));

    nodetypes.emplace_back(std::make_unique<BinaryNumber>());
    nodetypes.emplace_back(std::make_unique<HexNumber>());
    //Operations
    nodetypes.emplace_back(std::make_unique<UnaryMinus>());
    nodetypes.emplace_back(std::make_unique<Pow>()); 
    nodetypes.emplace_back(std::make_unique<Abs>());
    nodetypes.emplace_back(std::make_unique<Round>());
    nodetypes.emplace_back(std::make_unique<Trunc>());
    
    nodetypes.emplace_back(std::make_unique<Factorial>());
    nodetypes.emplace_back(std::make_unique<ArithmeticProgression>());
    nodetypes.emplace_back(std::make_unique<GeometricProgression>());
    
    nodetypes.emplace_back(std::make_unique<Minimum>());
    nodetypes.emplace_back(std::make_unique<Maximum>());
    nodetypes.emplace_back(std::make_unique<Variance>());
    nodetypes.emplace_back(std::make_unique<Median>());
    nodetypes.emplace_back(std::make_unique<ArithmeticMean>());
    nodetypes.emplace_back(std::make_unique<GeometricMean>());
    nodetypes.emplace_back(std::make_unique<HarmonicMean>());
    
    nodetypes.emplace_back(std::make_unique<Sines>());
    nodetypes.emplace_back(std::make_unique<Cosines>());
    nodetypes.emplace_back(std::make_unique<Tangents>());
    nodetypes.emplace_back(std::make_unique<Cotangents>());
    nodetypes.emplace_back(std::make_unique<ArcSines>());
    nodetypes.emplace_back(std::make_unique<ArcCosines>());
    nodetypes.emplace_back(std::make_unique<ArcTangents>());
    nodetypes.emplace_back(std::make_unique<ArcCotangents>());
    nodetypes.emplace_back(std::make_unique<HyperbolicSines>());
    nodetypes.emplace_back(std::make_unique<HyperbolicCosines>());
    nodetypes.emplace_back(std::make_unique<HyperbolicTangents>());
    nodetypes.emplace_back(std::make_unique<HyperbolicCotangents>());
    nodetypes.emplace_back(std::make_unique<AreaSines>());
    nodetypes.emplace_back(std::make_unique<AreaCosines>());
    nodetypes.emplace_back(std::make_unique<AreaTangents>());
    
    nodetypes.emplace_back(std::make_unique<Logarithm>());
    nodetypes.emplace_back(std::make_unique<NaturalLogarithm>());
    nodetypes.emplace_back(std::make_unique<Log10>());
    
    nodetypes.emplace_back(std::make_unique<Exponent>());
    nodetypes.emplace_back(std::make_unique<Pi>());

    ShowHelp();
}


void Calculator::ShowHelp()
{
    std::cout << "\nConsole *** Kiss the Pig *** Calculator"
        << "\nSupports arithmetic, trigonometric, logarithmic, some statistic functions.\n";
        
    for (auto& i : nodetypes)
        i->ShowHelp();

    std::cout << "Operands inside function calls must be delimited by ;\n"
        << "You can build complex expressions like this: \n"
        << "--> ((2.2212+3*10) * max(10; 33+arithmean(12; ln(11); 5*6.23; pow(3;8); 11))) / (13 * 12)\n\n";
}

void Calculator::PreProcessInStr(std::string& str) {
    
    // 1. Удаляем все пробельные символы (пробелы, табуляции \t, переносы \n, \r)
    // Важно: приводим char к unsigned char, чтобы избежать UB на отрицательных символах (например, кириллице)
    str.erase( std::remove_if(
            str.begin(), str.end(), []
            (unsigned char c) {
            return std::isspace(c);}), 
            str.end()
    );
    // 2. Приводим оставшиеся символы к нижнему регистру
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
};

void Calculator::FinalizeResult(OperandType& result) {
    // Убирает и микроскопические погрешности, и отрицательный ноль (-0.0)
    if (std::abs(result) < MinNumber) {
        result = 0.0L;
    }
}
///поисковая процедуа
///возвращает 
///     
/// 
/// 
/// 
/// 
std::unique_ptr<Node>
Calculator::SliceNextNode(
    //распознаваемая подстрока
    const std::string& str,
    //предыдущий узел
    //std::unique_ptr<Node> prevnode   
    const Node& prevnode
) const
{
    auto resultnode = std::make_unique<Node>(); 

    resultnode->nodestatus = ParseStatus::NoRecognize;  //начальное значение

    int pos = -1;  // макс знач длины строки
    int ntidx = 0;  //Nodetypes arrary index
    while (ntidx < nodetypes.size()) {
        //берем подстроку с конца предыдущего распознанного токена
        pos = nodetypes[ntidx]->DoIRecognizeSelf(str, prevnode.endpos);
        if (pos != -1) {
            //узeл  опознан //node type is recognized
            // теперь надо понять может ли он занмать эо место в строке
            resultnode->startpos = prevnode.endpos;
            resultnode->endpos = pos;
            resultnode->strvalue = str.substr(resultnode->startpos, pos - resultnode->startpos);
            resultnode->nodetype = nodetypes[ntidx].get();
            //если это не первый нод в строке
            if (prevnode.nodetype) {
                //и если опознанному node type разрешено следовать за предыдущим то ОК    
                if (prevnode.nodetype->CanBeFollowedBy(nodetypes[ntidx]->getName()))
                    resultnode->nodestatus = ParseStatus::OK;
                else
                    //если нет то опознан но следовать не может.
                    resultnode->nodestatus = ParseStatus::NoFollow;
            }
            else {
                //если prevnode.nodetype==nullptr то это первый нод в строке 
                if (nodetypes[ntidx]->CanStartExpr())
                    //... и может начинать строку
                    resultnode->nodestatus = ParseStatus::OK;
                else
                    //если нет то все
                    resultnode->nodestatus = ParseStatus::NoStartWith;
            }
            //break;
        }
        if (resultnode->nodestatus == ParseStatus::OK) break;
        ++ntidx;  //к следующему nodetype
    }
    if (resultnode->nodestatus == ParseStatus::NoRecognize)  //если  ==OK то все уже заполнено
    {
        resultnode->startpos = prevnode.endpos;
        resultnode->endpos = pos;
        resultnode->strvalue = str.substr(resultnode->startpos, 5);
        resultnode->nodetype = nullptr;
    }

//  std::println(" {} ",resultnode->strvalue);
//return std::move(resultnode);  gg
    return resultnode;  



}


//Преобразовать строку поступившую от 
//юзера в обратную польскую нотацию (ОПН)
void Calculator::ToReversePolishNotation(std::string& str)
{
    std::stack<std::unique_ptr<Node>> stack{};
    
    auto TopStackToVec = [&stack, this]()
        {   
            //!!!check for empty
            if (stack.empty()) throw std::runtime_error("Stack: nothing to move");
            std::unique_ptr<Node> top_node = std::move(stack.top());
            stack.pop();
            vec.emplace_back(std::move(top_node));
        };

    //очистить вектор перед новым расчетом
    vec.clear();
    //параметры предыдущего Node
    auto prevnode = Node();  //endpos = startpos = 0
    //параметры текущего Node
    auto node = std::unique_ptr<Node>();
    
    while (prevnode.endpos < str.length())
    {
       //опознать следующий кусок строки
        node = SliceNextNode(str, prevnode);
        //если не опознано то ошибка
        switch (node->nodestatus) {
        case ParseStatus::NoRecognize :
            throw std::runtime_error(
                "Parsing error: unknown symbols starting with " + 
                node->strvalue  + "...");
            break;
        case ParseStatus::NoFollow :
            //foundstr = str.substr(0, nextpos);
            throw std::runtime_error("Parsing error: " + prevnode.strvalue +
                " can't be followed by " + node->strvalue + ".");
            break;
        case ParseStatus::NoStartWith :
            throw std::runtime_error("Parsing error: expression can't start with " + 
                   node->strvalue + ".");
            break;
        case ParseStatus::OK :
            break;
        }

        auto nodetype = node->nodetype;//!check if norecognize
        auto rawnode = node.get();
        //В зависимости от ТИПА КИДА разные сценарии
        switch (nodetype->getName()) {

        case ParsedElem::Number:
            vec.emplace_back(std::move(node));
            break;
        case ParsedElem::UnaryOperation:
            stack.push(std::move(node));
            break;
        case ParsedElem::BinaryOperation:
            while (!stack.empty() && 
                   (stack.top()->nodetype->getName() == ParsedElem::BinaryOperation ||
                    stack.top()->nodetype->getName() == ParsedElem::UnaryOperation))
            {
                if (stack.top()->nodetype->getPriority() >= 
                    (nodetype->getPriority())
                   )
                {
                    TopStackToVec();
                }
                else
                    break;
            }
            stack.push(std::move(node));
            break;
            //открыввающая скобка
        case ParsedElem::CompoundStart:
            stack.push(std::move(node));
            break;
            //закрыввающая скобка
        case ParsedElem::CompoundEnd: {
            int counter = 1;
            while ((!stack.empty() && 
                   (stack.top()->nodetype->getName() != ParsedElem::CompoundStart)))
            {
                if (stack.top()->nodetype->getName() == ParsedElem::OperandDelimiter) {
                    counter++;
                }
                else { 
                    std::unique_ptr<Node> top_node = std::move(stack.top());
                    vec.emplace_back(std::move(top_node));
                }
                stack.pop();
            };
            //должны уже добраться до начало сcompounda (в этой точке)
            if (stack.empty())
                throw std::runtime_error("Parsing error: missed open bracket or function name.");
            else
            {// если (stack.top()->getNodetypeName() == anCompoundStart ) {
                if (counter > MaxOperandCount) throw std::runtime_error("Too many operands.");
                stack.top()->args = counter;

                //by google -----
                OperNeed need = stack.top()->nodetype->getOperNeed();
                if (need > 0 && counter != need) {
                    throw std::runtime_error("Parsing error: wrong number of arguments for " + stack.top()->strvalue);
                }
                if (need == -1 && counter > 1) { // Если это обычные скобки "(...)" без функции
                    throw std::runtime_error("Parsing error: unexpected delimiter inside brackets.");
                }
                //-------
                TopStackToVec();
            }
            break;
        }
        case ParsedElem::OperandDelimiter:
            //если предыдущий аргумент Compounda сложный то перед тем как положить делимитер в стек нужно
            //вытащить в vector все операции относящиеся к предыд аргументу
            while (!stack.empty() && 
                  (stack.top()->nodetype->getName() == ParsedElem::BinaryOperation ||
                  (stack.top()->nodetype->getName() == ParsedElem::UnaryOperation))
                )
            {
                TopStackToVec();
            }
            //Если стек пуст значит встретился делимитер без compounda
            if (stack.empty())
                throw std::runtime_error("Parsing error: " + nodetype->getSymbolset() + " w/o function name.");
            stack.push(std::move(node));
            
            break;

        default:
            throw std::runtime_error("Unknown parsing error.");
            break;
        }
        
        prevnode = (*rawnode);
    }
    //записать оставшееся в стеке в вектор
    while (!stack.empty()) {
        if (stack.top()->nodetype->getName() == ParsedElem::CompoundStart) {
            // && !stack.top()->args) {
            throw std::runtime_error("Parsing error: missed close bracket.");
        }
        TopStackToVec();
    }
std::println("polishnotation");
for (const auto& i : vec) {
    std::print("| {} ", i->strvalue);
}

}


//Вычисление згначения по ОПЗ (Обратной польской нотации)
OperandType Calculator::Execute()
{
    std::stack<OperandType> stack;
    OperandType result{};
    //OperandType operands[MaxOperandCount];
    vector<OperandType> operands{};
    std::size_t v = 0;
    ///рабочая переменная не  владеет 
    NodeType* nodetype = vec[v]->nodetype;

    std::print("rpn : ");    
    for (const auto& i : vec) {
        std::print(" {} | ", i->strvalue);
    }
    std::println();

    while (v < vec.size())
    {
        auto calculate = [&](string str)  {
            result = nodetype->Exec(operands, str);
            FinalizeResult(result);
            stack.push(result);
            std::println("to stack: {}", result);
        };
        nodetype = vec[v]->nodetype;
        operands.clear();
        switch (vec[v]->nodetype->getName())
        {

        case ParsedElem::Number:
            //посчитать число (из строки в OperandType) и положить его в стек
            calculate(vec[v]->strvalue);
            break;

        case ParsedElem::UnaryOperation:
            if (stack.size() >= 1) {
                operands.push_back(stack.top()); stack.pop();
                calculate("");
            }
            else {
                throw std::runtime_error("Parsing error: must be 1 opnd for unary operation.");
            }
            break;
        case ParsedElem::BinaryOperation:
            if (stack.size() >= 2) {
                operands.push_back(stack.top()); stack.pop();
                operands.push_back(stack.top()); stack.pop();
                std::reverse(operands.begin(), operands.end());
                calculate("");
            }
            else {
                throw std::runtime_error("Parsing error: must be 2 operands for binary operation.");
            }
            break;

        case ParsedElem::CompoundStart: {
            //проверить совпадает ли кол-во операндов необходимое Compound и тому что записано
            OperNeed opneed = nodetype->getOperNeed();
            if (opneed != 0 && vec[v]->args != opneed) //выяснить 0 это  от 1 и больше?
                throw std::runtime_error("Parsing error: operand count mismatch.");
            //выбраь операнды из стека
            auto j = static_cast<int>(vec[v]->args) - 1;
            while (j >= 0 && !stack.empty() && j < MaxOperandCount) {
                operands.push_back(stack.top());
                stack.pop();
                --j;
            }
            std::reverse(operands.begin(), operands.end());
            if (stack.empty() && j != -1)
                throw std::runtime_error("Parsing error: Not enough operands.");
            //посчитать Результат Compounda и положить его в стек
            calculate("");
            break;
        }
        default:
            throw std::runtime_error("Unknown error in parsing procedure.");
            break;
        }
        ++v;
    }
   
    /*if (!stack.empty())   ///так было уменя . до рекомендациии от тгугл ИИ
        return stack.top();
    else
        throw std::runtime_error("Unknown parsing error.");*/

    if (stack.size() == 1)
        return stack.top();
    else
        throw std::runtime_error("Unknown parsing error.");
};
   

