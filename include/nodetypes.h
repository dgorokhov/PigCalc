#pragma once


#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <string_view>
#include <stdexcept>
#include <numbers>


using std::string_view;
using std::string;
using std::vector;

using OperandType = long double;
constexpr OperandType MaxNumber = std::numeric_limits<OperandType>::max()/100.0L;
//constexpr OperandType MinNumber = std::numeric_limits<long double>::min()*100;
constexpr OperandType MinNumber = std::numeric_limits<OperandType>::epsilon() * 100.0L;

constexpr OperandType PI = std::numbers::pi_v<long double>;

constexpr int MaxOperandCount = 50;


//сколько операндов нужно для узла расчета
// -1 нисколько
// 0  сколько угодно
// N  N
using OperNeed = int;

class CalcHelper {
public:

    static OperandType CheckResult(long double value, const string& context) {
        if (std::isnan(value)) {
            throw std::domain_error(context + "not a number (NaN).");
        }
        if (std::isinf(value)) {
            throw std::overflow_error(context + "overflow.");
        }
        // FP_SUBNORMAL, FP_NORMAL и FP_ZERO пропускаем без изменений
        return value;
    }

    static OperandType ParseNumber(const std::string& token) {
        try {
            size_t pos = 0;
            long double val = std::stold(token, &pos);
        
            if (pos != token.size()) {
                throw std::invalid_argument("Invalid number format: " + token);
            }
            if (std::isnan(val) || std::isinf(val)) {
                throw std::invalid_argument("Unsupported literal: " + token);
            }
            return val;
        }
        catch (const std::out_of_range&) {
            throw std::overflow_error("Number out of range: " + token);
        }
    }
    static OperandType CheckDivisor(long double value) {
        if (value == 0.0L) {    //строго на полный ноль.  расчеты с subnormal разрешены
            throw std::domain_error("Division by zero.");
        }
        return value;
    };
};

//тип узла присваемый 
//узлу расчета парсером калькулятора
enum class ParsedElem {
    Unknown,
    NodeType,
    OperandDelimiter,
    Number,
    UnaryOperation,
    BinaryOperation,
    CompoundStart,
    CompoundEnd
};

//приоритет опеераций
enum class Priority {
    Undefined,
    Low,
    Middle,
    High
};

//Резуьтат парсинга
enum class ParseStatus
{
    NoRecognize,      //узел не распознан. строка R123 не является ничем имеющим смысл для калькулятора
    NoStartWith,      //распознан, не может начинать строку
    NoFollow,         //распознан, но не может следоват за предыдущим
    OK                //все в порядке
};

/*
class NodeType
class Number
class BinaryNumber
class HexNumber
class Constant:
class BinaryOperation
class UnaryOperation
class CompoundStart
class CompoundEnd
class OperandDelimiter
class UnaryMinus:
class Addition:
class Substraction:
class Multiplication:
class Division:
class Factorial:
class Abs
class Trunc
class Round
class Pow
class Pi
class Sines:
class Cosines:
class Tangents:
class Cotangents:
class ArcSines
class ArcCosines
class ArcTangents
class ArcCotangents
class HyperbolicSines
class HyperbolicCosines
class HyperbolicTangents
class HyperbolicCotangents
class AreaSines
class AreaCosines
class AreaTangents
class Exponent
class NaturalLogarithm:
class Logarithm:
class Log10
class Minimum
class Maximum
class ArithmeticMean:
class GeometricMean
class HarmonicMean
class Median:
class Variance:
class ArithmeticProgression:
class GeometricProgression
53 classes
*/


//абстрактрый класс для всех узлов расчета
//что это смотрите в Calculator.h
class NodeType
{
protected:
 
    //типы узлов которые могут следовать за этим узлом
    //см пример класса Number
    std::vector<ParsedElem> _followed_by;
    std::string _symbol_set{};
    //краткая запись узла расчета (испольщуется в help)
    std::string _about{};
    //описание узла расчета (испольщуется в help)
    std::string _usecase{};
    Priority _priority{};
    //колво операндов которое должно быть у узла (если равно нулю - то сколькко угодно)
    //(-1) нисколько
    OperNeed _operneed{-1};
    //длина строки которая м.б. преобразована в этот узел расчета
    //если _len==0 то без ограничений
    int _len;
    //может ли бытьь началом выражения
    bool _canstartexpr{ true };
public:
    NodeType(const std::string& symbolset, int len, 
             OperNeed operneed = -1
     ) :
        _priority(Priority::Undefined), 
        _symbol_set(symbolset), 
        _len(len),
        _operneed(operneed)
    { };
    virtual ~NodeType() { };
    NodeType(const NodeType& original) = delete;

    OperNeed getOperNeed() const { return _operneed; };
    void ShowHelp() const
    {
        if (!_about.empty()) 
        {
            std::string s = "  " + _about + "                                     ";
            s.resize(40); s += _usecase;
            std::cout << s << std::endl;
        }
    }
    const Priority getPriority() const { return _priority; };
    const bool CanStartExpr() const { return _canstartexpr; }
    virtual ParsedElem getName() const { return ParsedElem::NodeType; };
    virtual bool CanBeFollowedBy(ParsedElem name) const 
    {
        for (const auto& i : _followed_by)
        {
            if (i == name) return true;
        }
        return false;
    }
    std::string getSymbolset() const { return _symbol_set; };
    
    //функция "DoIRecognizeSelf" возвращает
    //= -1 если фрагмент строки str (с начала и до какогонибудь символа  
    //      не может быть объектом этого потомка Nodetype (данный nodetype не узнал себя в начале строки)
    //= Если "признал" ,то возвращается позиция следующая за этим распознанным фрагментом
    //пример для строки ("12+23") процедура вернет:
    //для класса "Number" = 2.... (строка начин-ся с числа, следующая позицуия после 12 это +)
    //для класса "BinaryMinus" = -1 так как строка не начинается с вычитания

    virtual int DoIRecognizeSelf(const std::string& str, int  startpos) const
    {
        int i = startpos;
        switch (_len) {
        case 0:
            while (i < str.size()) {
                if (_symbol_set.find(str.at(i)) == std::string::npos)  break;
                ++i;
            };
            if (i == startpos)
                return -1;
            else
                return i;
            //return static_cast<int>(i - startpos - 1);
            break;
        case 1:
            if (_symbol_set.find(str.at(i)) == std::string::npos)
                return -1;
            else
                return ++startpos;
            break;
        default:
            return -1; //на всякий случй
            break;
        }
    }

    /*  
        Exec - процедура вычисления значения данного узла
        operands - сслыка на массив аргументов, 
        operand_count - кол-во аргументов
        str - строка (только для простых типов (Number) непосредственно преобразуемых в число)
        Т.к.параметры передаются по значению, const для параметров не нужен
    */
    virtual OperandType Exec (vector<OperandType>& operands, const std::string str) const { 
        return 0.0L; 
    };

   
};


//для каждого вновь опознанного фрагмента (распарсенного) строки заводится Узел
// сёо следующей информацией о соответсвии входной подстроки калькулятора
//и тип узла
struct Node
{
    std::string strvalue{};
    NodeType* nodetype{ nullptr };      // не владееет ресурсом  
    ParseStatus nodestatus{ ParseStatus::NoRecognize };
    int args{ 0 };          // кол-во аргументов (заполняется при вычислении Обратной польской нотации)
    int startpos{ 0 };      
    int endpos{ 0 };
    Node() = default;
    Node(const Node&) = default;
    Node& operator=(const Node&) = default;
    Node(NodeType* nodetype, 
        std::string strvalue,
        int startpos, int endpos
    ) :
        nodetype(nodetype), //!!!check unullprtr
        strvalue(strvalue),
        startpos(startpos),
        endpos(endpos)
    { }
    /*std::string getStrvalue() const { return _strvalue; };
    NodeType* getNodetype() const { return _nodetype; };
    ParsedElem getNodetypeName() const { return _nodetype->getName(); };
    Priority getPriority() const { return _nodetype->getPriority(); };*/
};



class Number : public NodeType
{
public:
    //описание конструктора класса Number: 
    // это уузел рассчета ЧИСЛО (number) которое мож.быть записано 
    //любым кол-вом символов (второй параметр конструктора ==0) 
    //из набора символов ".0123456789"
    Number() : NodeType(".0123456789", 0)
    {
        _priority = Priority::Undefined;
        //после узла Number может следовать :
        _followed_by = { 
            ParsedElem::BinaryOperation, //бинарная операция
            ParsedElem::OperandDelimiter,//разделитель операндов
            ParsedElem::CompoundEnd // закрывающая скобка
        };
    };
    //класс number относится к категории чисел "return ParsedElem::Number;"
    //(эту инфор-ю использует парсер калькулятора
    ParsedElem getName() const override { return ParsedElem::Number; };
    //чтобы преобразовать строку содержащую number нужно
    // просто вызвать стандартную функцию преобр-я stof. Stof не получит
    // неправильыне данные т.к. вся подстрока м. содержать символы из набора ".0123456789"
    OperandType Exec(vector<OperandType>& operands, const std::string str) const override
    {
        return CalcHelper::ParseNumber(str);
    }
};

class BinaryNumber : public NodeType
{
public:
    BinaryNumber() : NodeType("01", 0)
    {
        _priority = Priority::Undefined;
        _followed_by = { 
            ParsedElem::BinaryOperation, 
            ParsedElem::OperandDelimiter, 
            ParsedElem::CompoundEnd 
        };

        _about = "0b101110";
        _usecase = "Binary number, without fraction part";
    };
    ParsedElem getName() const override { return ParsedElem::Number; };
    int DoIRecognizeSelf(const std::string& str, int  startpos) const override
    {
        auto i = startpos;
        if (str.substr(i, 2) == "0b")
        {
            i += 2;
            while (i < str.length() && ((str.at(i) == '1') || (str.at(i) == '0')))
               ++i;
            if (i >= 3 + startpos)
                return i;
            else
                return -1;
        }
        else return -1;
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override
    {
        //из строки 0b0111011101 в OperandType
        int i = static_cast<int>(str.length() - 1);
        OperandType sum{ 0.0 };
        while (i >= 2)
            sum += str.at(i) == '0' ? 0 : pow(2, str.length() - 1 - i), --i;
        return CalcHelper::CheckResult(sum, _about);
    };
};


class HexNumber : public NodeType
{
public:
    HexNumber() : NodeType("0123456789abcdef", 0)
    {
        _priority = Priority::Undefined;
        _followed_by = { ParsedElem::BinaryOperation, ParsedElem::OperandDelimiter, ParsedElem::CompoundEnd };

        _about = "0x1AF233C";
        _usecase = "Hex number, without fraction part";
    };
    ParsedElem getName() const override { return ParsedElem::Number; };
    int DoIRecognizeSelf(const std::string& str, int  startpos) const override
    {
        auto i = startpos;
        if (str.substr(i, 2) == "0x")
        {
            i += 2;
            while (i < str.length() && (_symbol_set.find(str.at(i)) != std::string::npos))
                ++i;
            if (i >= 3 + startpos) 
                return i;
            else
                return -1;
        }
        else return -1;
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override
    {
        //из строки 0x в OperandType
        int i = static_cast<int>(str.length() - 1);
        OperandType sum{ 0.0L };
        while (i >= 2) {
            if (str.at(i) >= 85)
                sum += static_cast<OperandType>(str.at(i) - 87) * pow(16, str.length() - 1 - i);//......
            else
                sum += static_cast<OperandType>(str.at(i) - 48) * pow(16, str.length() - 1 - i);
            --i;
        }
        return CalcHelper::CheckResult(sum, _about);
    };
};


class Constant: public NodeType
{
public:
    Constant(std::string symbols) : NodeType(symbols, 0)
    {   _priority=Priority::Undefined;
        _followed_by = { 
            ParsedElem::BinaryOperation, 
            ParsedElem::OperandDelimiter,
            ParsedElem::CompoundEnd };
    };
    ParsedElem getName()  const override { return ParsedElem::Number; };
    int DoIRecognizeSelf(const std::string& str, const int startpos)  const override
    {
        if (str.find(_symbol_set, startpos) == startpos)
            return static_cast<int>(_symbol_set.length()+startpos);  //ww _symbol_set.length()-1
        else
            return -1;
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str) const override { return 0.0;}
};

class BinaryOperation : public NodeType {
public:
    BinaryOperation(std::string sign, Priority priority ): NodeType (sign, 1 )
    {
        _priority=priority;
        _symbol_set=sign;
        _followed_by = { ParsedElem::Number,ParsedElem::CompoundStart };
        _canstartexpr = false;
    }
    ParsedElem getName() const override { return ParsedElem::BinaryOperation; } ;
};

class UnaryOperation : public NodeType {
public:
    UnaryOperation (std::string symbolset, Priority priority ): NodeType(symbolset, 1) 
    {
        this->_priority=priority;
        _followed_by = {
            ParsedElem::Number,
            ParsedElem::CompoundStart,
            ParsedElem::CompoundEnd,
            ParsedElem::BinaryOperation,
            ParsedElem::UnaryOperation };
        _canstartexpr = true;
    };
    ParsedElem getName()  const override { return ParsedElem::UnaryOperation; } ;
};



//Compound - общий класс на основе котрого строятся все "сложные" , классы
// =классы-функции
// =классы унарных и бинарных оперций
class CompoundStart : public NodeType
{
protected :
public:
    CompoundStart(std::string symbolset, int operandsneed) : NodeType(symbolset, 0, operandsneed)
    {
        _priority = Priority::High;
        _symbol_set = symbolset;
        _followed_by = {
                ParsedElem::UnaryOperation,
                ParsedElem::Number,
                ParsedElem::CompoundStart };
    };
    
    ParsedElem getName()  const override { return ParsedElem::CompoundStart; } ;
    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return operands.at(0);
    };

    int DoIRecognizeSelf(const std::string& str, const int startpos) const override
    {
        //если признак начала Compoundа стоит вначале
        if (str.find(_symbol_set, startpos) == startpos)
            return static_cast<int>(startpos + _symbol_set.length());
        else
            return -1;
    }

};

class CompoundEnd : public NodeType
{
public:
    CompoundEnd(std::string symbolset) : NodeType(symbolset, 0)
    {
        _priority = Priority::High;
        _followed_by = {
            ParsedElem::UnaryOperation,
            ParsedElem::BinaryOperation,
            ParsedElem::CompoundEnd,
            ParsedElem::OperandDelimiter
        
        };
        _canstartexpr = false;
    };
    ParsedElem getName()  const override { return ParsedElem::CompoundEnd; } ;
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        return operands.at(0);
    };
    int DoIRecognizeSelf(const std::string& str, int  startpos) const override
    {
        //если признак конца Compoundа стоит вначале
        if (str.find(_symbol_set, startpos) == startpos)
            return static_cast<int>(startpos + _symbol_set.length());
        else
            return -1;
    }
};

//разделитель аргументов в функциях
class OperandDelimiter : public NodeType
{
public:
    OperandDelimiter(std::string symbolset) : NodeType(symbolset, 1) {
        _followed_by = {
            ParsedElem::Number,
            ParsedElem::CompoundStart,
            ParsedElem::UnaryOperation,
        };
        _canstartexpr = false;
    };
    ParsedElem getName()  const override { return ParsedElem::OperandDelimiter; }
};

    /*
     * Классы операций (унарных и бинарных и функции)
     * **********************************************************
     * которые строятся на базе абстрактного класса nodetype
     * Парсер калькулятора () работает
     * на уровне абстрактных классов, через их виртульные методы т.к. информации которые
     * они предоставляют достаточно для разбора.
     * пользователь (программер) может определять свои
     * =классы-операции (наследуя от BinaryOperation, UnaryOperation)
     * =классы-числа (наследуя от Number или Constant или напрямую от NodeType)
     * =классы-функции (наследуя от СompoundStart)
     *
     */

//унарные и бинарные
//*******************************************

class UnaryMinus: public UnaryOperation {
public:
    UnaryMinus() : UnaryOperation ("-",Priority::High) //приоритее юольше чем у * и /
    {
        _followed_by = {
        ParsedElem::Number,
        ParsedElem::CompoundStart,
        ParsedElem::UnaryOperation
        };
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return ( - operands.at(0));
    };
};

class Addition: public BinaryOperation {
public:
    Addition() : BinaryOperation ("+",Priority::Low) {};
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override {
        OperandType op2 = operands.at(0) + operands.at(1);
        return ( CalcHelper::CheckResult(op2, _about));
    };
};

class Substraction: public BinaryOperation {
public:
    Substraction() : BinaryOperation("-", Priority::Low) {};
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType op2= operands.at(0) - (operands.at(1));
        return (CalcHelper::CheckResult(op2 ,_about));
    };
};


class Multiplication: public BinaryOperation {
public:
    Multiplication() : BinaryOperation ("*",Priority::Middle) {};
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType op2 = operands.at(0) * (operands.at(1));
        return ( CalcHelper::CheckResult(op2, _about));
    };
};

class Division: public BinaryOperation {
public:
    Division() : BinaryOperation ("/",Priority::Middle) {};
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        CalcHelper::CheckDivisor(operands.at(1));
        return  CalcHelper::CheckResult(operands.at(0) / operands.at(1),_about);
    };
};


//
/* Классы функций
 * Функция начинается с имени с прилегающей скобкой и заканчиваться закрывающей скобкой.
 * В конструктор передается "имя(" и кол-во аргументов которые принимает функция (если =0 то
 * любое кол-во арументов
 * Функция имеет вид name(p1;p2;...pn)
 * где р[n] - аргумент(ы). Колво аргументов >=1 и
 * ";" - это разделитель аргументов.
 * */

class Factorial: public CompoundStart {
public:
     Factorial() : CompoundStart("fac(",1)
{
    _about = "fac(X)";
    _usecase = "Factorial of X";
};
     
     OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
     {
         OperandType result = 1;
         for (int i = 1; i <= round(operands.at(0)); ++i) {
             result = CalcHelper::CheckResult(result * i, _about);
         }
         return result;
     };
};

class Abs : public CompoundStart {
public:
    Abs() : CompoundStart("abs(",1) 
    {
        _about = "abs(X)"; 
        _usecase = "Absolute value of X";
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        return std::abs(operands.at(0));
    };
};

class Trunc : public CompoundStart {
public:
    Trunc() : CompoundStart("trunc(", 1)
    {
        _about = "trunc(X)"; 
        _usecase = "Eliminates fractional part of X";
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        return std::trunc(operands.at(0));
    };
};

class Round : public CompoundStart {
public:
    Round() : CompoundStart("round(", 1)
    {
        _about = "round(X)"; 
        _usecase = "Rounds X to nearest integer number";
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        return std::round(operands.at(0));
    };
};

class Pow : public CompoundStart {
public:
    Pow() : CompoundStart("pow(", 2)
    {
        _about = "pow(X, Y)";
        _usecase = "Pow(X, Y). Ex: pow(2, 3) = 8";
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override
    {
        OperandType op2 = CalcHelper::CheckResult(pow(operands.at(0),
         operands.at( + 1)), _about);
        return (op2);
    };
};

class Pi : public Constant {
public:
    Pi() : Constant("pi")
    {
        _about = "pi";
        _usecase = "Pi = 3.14159265358979...";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        return PI;
    }
};


class Sines: public CompoundStart {
public:
    Sines() : CompoundStart("sin(",1)
    {
        _about = "sin(X)";
        _usecase = "Sines of X";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        if (std::abs(std::remainder(operands.at(0), PI)) < MinNumber) 
            return 0.0L; // Возвращаем идеальный ноль, теперь 1 / sin(pi) даст Division by zero!
        
        return CalcHelper::CheckResult(std::sin(operands.at(0)), _about);
    };
};

class Cosines: public CompoundStart {
public:
    Cosines() : CompoundStart("cos(", 1)
    {
        _about = "cos(X)";
        _usecase = "Cosines of X";
    };

    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override
    {
        if (std::abs(std::remainder(operands.at(0) - PI / 2.0L, PI)) < MinNumber) 
            return 0.0L;
        return CalcHelper::CheckResult(std::cos(operands.at(0)), _about);
    };
};

class Tangents: public CompoundStart {
public:
    Tangents() : CompoundStart("tan(",1)
    {
        _about = "tan(X)";
        _usecase = "Tangents of X";
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        if (std::abs(std::remainder(operands.at(0) - PI / 2.0L, PI)) < MinNumber) 
            throw std::domain_error("Tangents: function is not defined for this argument.");
        // Если x кратен PI, возвращаем точный ноль
        if (std::abs(std::remainder(operands.at(0), PI)) < MinNumber) 
            return 0.0L;    
        return CalcHelper::CheckResult(std::tan(operands.at(0)), _about);
    };
};


class Cotangents: public CompoundStart {
public:
    Cotangents() : CompoundStart("cotan(",1)
    {
        _about = "cotan(X)";
        _usecase = "Cotangents of X";
    };
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override {

        if (std::abs(std::remainder(operands.at(0), PI)) < MinNumber) 
            throw std::domain_error("Cotangents: function is not defined for this argument.");
        
        if (std::abs(std::remainder(operands.at(0) - PI / 2.0L, PI)) < MinNumber) 
            return 0.0L;
        return CalcHelper::CheckResult(
            1/CalcHelper::CheckDivisor(std::tan(operands.at(0))), _about);
            //std::cos(operands.at(0)/std::sin(operands.at(0))), _about);
    };
};
// ============================================================================
// Обратные тригонометрические функции
// ============================================================================

class ArcSines : public CompoundStart {
public:
    ArcSines() : CompoundStart("asin(", 1) {
        _about = "asin(X)";
        _usecase = "Arcsine of X in radians, X in [-1, 1]";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        
        OperandType x = operands.at(0);
        if (x > 1.0L && x <= 1.0L + MinNumber) x = 1.0L;
        if (x < -1.0L && x >= -1.0L - MinNumber) x = -1.0L;

        if (x < -1.0L || x > 1.0L) {
            throw std::domain_error("ArcSines: argument must be in [-1, 1].");
        }
        return CalcHelper::CheckResult(std::asin(x), _about);
    }
};

class ArcCosines : public CompoundStart {
public:
    ArcCosines() : CompoundStart("acos(", 1) {
        _about = "acos(X)";
        _usecase = "Arccosine of X in radians, X in [-1, 1]";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        OperandType x = operands.at(0);
        if (x > 1.0L && x <= 1.0L + MinNumber) x = 1.0L;
        if (x < -1.0L && x >= -1.0L - MinNumber) x = -1.0L;

        if (x < -1.0L || x > 1.0L) {
            throw std::domain_error("ArcCosines: argument must be in [-1, 1].");
        }
        return CalcHelper::CheckResult(std::acos(x), _about);
    }
};

class ArcTangents : public CompoundStart {
public:
    ArcTangents() : CompoundStart("atan(", 1) {
        _about = "atan(X)";
        _usecase = "Arctangent of X in radians";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return std::atan(operands.at(0));
    }
};

class ArcCotangents : public CompoundStart {
public:
    ArcCotangents() : CompoundStart("acot(", 1) {
        _about = "acot(X)";
        _usecase = "Arccotangent of X in radians (0, pi)";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        // Формула pi/2 - atan(x) определена для всех x (включая x = 0, где acot(0) = pi/2)
        // и возвращает значение в главном диапазоне (0, pi)
        return (std::numbers::pi_v<OperandType> / 2.0L) - std::atan(operands.at(0));
    }
};

// ============================================================================
// Гиперболические функции
// ============================================================================

class HyperbolicSines : public CompoundStart {
public:
    HyperbolicSines() : CompoundStart("sinh(", 1) {
        _about = "sinh(X)";
        _usecase = "Hyperbolic sine of X";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return CalcHelper::CheckResult(std::sinh(operands.at(0)), _about);
    }
};

class HyperbolicCosines : public CompoundStart {
public:
    HyperbolicCosines() : CompoundStart("cosh(", 1) {
        _about = "cosh(X)";
        _usecase = "Hyperbolic cosine of X";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return CalcHelper::CheckResult(std::cosh(operands.at(0)), _about);
    }
};

class HyperbolicTangents : public CompoundStart {
public:
    HyperbolicTangents() : CompoundStart("tanh(", 1) {
        _about = "tanh(X)";
        _usecase = "Hyperbolic tangent of X";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return std::tanh(operands.at(0));
    }
};

class HyperbolicCotangents : public CompoundStart {
public:
    HyperbolicCotangents() : CompoundStart("coth(", 1) {
        _about = "coth(X)";
        _usecase = "Hyperbolic cotangent of X, X != 0";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        OperandType x = operands.at(0);
        OperandType sh = std::sinh(x);
        if (std::abs(sh) == 0.0L) {
            throw std::domain_error("HyperbolicCotangents: function is not defined for X = 0.");
        }
        // Для очень больших |x| и cosh(x), и sinh(x) уходят в inf, давая NaN при делении.
        // Математически при |x| > 50 coth(x) с точностью long double равен +1 или -1.

        if (std::isinf(sh)) {
            return (x > 0.0L) ? 1.0L : -1.0L;
        }
        return CalcHelper::CheckResult(std::cosh(x) / sh, _about);
    }
};

// ============================================================================
// Обратные гиперболические функции (ареа-функции)
// ============================================================================

class AreaSines : public CompoundStart {
public:
    AreaSines() : CompoundStart("asinh(", 1) {
        _about = "asinh(X)";
        _usecase = "Inverse hyperbolic sine of X";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        return CalcHelper::CheckResult(std::asinh(operands.at(0)), _about);
    }
};

class AreaCosines : public CompoundStart {
public:
    AreaCosines() : CompoundStart("acosh(", 1) {
        _about = "acosh(X)";
        _usecase = "Inverse hyperbolic cosine of X, X >= 1";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        OperandType x = operands.at(0);
        if (x < 1.0L) {
            throw std::domain_error("AreaCosines: argument must be >= 1.");
        }
        return CalcHelper::CheckResult(std::acosh(x), _about);
    }
};

class AreaTangents : public CompoundStart {
public:
    AreaTangents() : CompoundStart("atanh(", 1) {
        _about = "atanh(X)";
        _usecase = "Inverse hyperbolic tangent of X, X in (-1, 1)";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str) const override {
        OperandType x = operands.at(0);
        if (x <= -1.0L || x >= 1.0L) {
            throw std::domain_error("AreaTangents: argument must be in (-1, 1).");
        }
        return CalcHelper::CheckResult(std::atanh(x), _about);
    }
};

//Logarithmic
//*****************************************************

class Exponent : public Constant {
public:
    Exponent() : Constant("exp")
    {
        _about = "exp";
        _usecase = "Exponent = 2.71828182845904523536...";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override
    {
        return std::numbers::e_v<long double>;
    }
};


class NaturalLogarithm: public CompoundStart {
public:
    NaturalLogarithm() : CompoundStart("ln(", 1)
    {
        _about = "ln(X)";
        _usecase = "Natural logarithm of X (exponent is the base)";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        if ( (operands.at(0)) <= 0 )
            throw std::domain_error("Logarithm: argument must be > 0.");
        else
            return CalcHelper::CheckResult(std::log(operands.at(0)), _about);
    };
};

class Logarithm: public CompoundStart {
public:
    Logarithm() : CompoundStart("log(",2)
    {
        _about = "log(base; X)";
        _usecase = "Logarithm of X. Ex: log(2, 8) = 3";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        auto base = operands.at(0);
        auto x = operands.at(1);
       
        if ( x <= 0 || base <= 0 )
            throw std::domain_error("Logarithm: argument(s) must be > 0.");
        if (std::abs(1.0L - base) < MinNumber) 
            throw std::domain_error("Logarithm: the base is too close to 1.");
        return CalcHelper::CheckResult(
                std::log(operands.at(1))/std::log(operands.at(0)), _about);
        
    };
};

class Log10 : public CompoundStart {
public:
    Log10() : CompoundStart("lg(", 1) 
    {
        _about = "lg(X)";
        _usecase = "Decimal logarithm of X. Ex: lg(100) = 2";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        if ((operands.at(0)) <= 0)
            throw std::domain_error("Logarithm: argument must be > 0.");
        else {
            return CalcHelper::CheckResult(std::log10(operands.at(0)),_about);
        }
    };
};


class Minimum : public CompoundStart {
public:
    Minimum() : CompoundStart("min(", 0) 
    {
        _about = "min(X1; X2;...Xn)";
        _usecase = "Minimum of any given numbers";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType min = operands.at(0);
        for (size_t i = 1; i < operands.size(); ++i) {
            min = (operands.at(i) < min) ? operands.at(i) : min;
        }
        return min;
    };
};

class Maximum : public CompoundStart {
public:
    Maximum() : CompoundStart("max(", 0)
    {
        _about = "max(X1; X2;...Xn)";
        _usecase = "Maximum of any given numbers";
    }

    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType max = operands.at(0);
        for (size_t i = 1; i < operands.size(); ++i) {
            max = (operands.at(i) > max) ? operands.at(i) : max;
        }
        return max;
    };
};

class ArithmeticMean: public CompoundStart {
public:
    ArithmeticMean() : CompoundStart("arithmean(",0) 
    {
        _about = "arithmean(X1; X2;...Xn)";
        _usecase = "Arithmetic mean of given numbers";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType sum = 0;
        for (size_t i = 0; i < operands.size(); ++i) {
            sum += operands.at(i);
        }
        return sum /= operands.size();
   };
};

class GeometricMean : public CompoundStart {
public:
    GeometricMean() : CompoundStart("geomean(", 0)
    {
        _about = "geomean(X1; X2;...Xn)";
        _usecase = "Geometric mean of given numbers";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType res = 1.0;
        for (size_t i = 0; i < operands.size(); ++i) {
            res = CalcHelper::CheckResult(res * operands.at(i), _about);
        }
        return CalcHelper::CheckResult(pow(res, 1.0/operands.size()), _about);
    };
};


class HarmonicMean : public CompoundStart {
public:
    HarmonicMean() : CompoundStart("harmean(", 0)
    {
        _about = "harmean(X1; X2;...Xn)";
        _usecase = "Harmonic mean of given numbers";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType res = 0;
        for (size_t i = 0; i < operands.size(); ++i) {
            res += 1 / CalcHelper::CheckDivisor(operands.at(i));
        }
        return operands.size() / CalcHelper::CheckDivisor(res);
    };
};

class Median: public CompoundStart {
public:
    Median() : CompoundStart("median(", 0)
    {
        _about = "median(X1; X2;...Xn)";
        _usecase = "Median of given numbers";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        std::vector<OperandType> vec = operands;
        std::sort(vec.begin(), vec.end());
        auto vs = vec.size();
        return (operands.size() % 2) ? vec[vs / 2] : (vec[vs / 2 - 1] + vec[vs / 2]) / 2;
    };
};

class Variance: public CompoundStart {
public:
    Variance() : CompoundStart("var(", 0)
    {
        _about = "var(X1; X2;...Xn)";
        _usecase = "Variance of given numbers";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        OperandType average{ 0 }, result{ 0 };
        for (size_t i = 0; i < operands.size(); ++i)
            average += operands.at(i);
        average /= operands.size();
        for (size_t i = 0; i < operands.size(); ++i)
            result += CalcHelper::CheckResult(pow((operands.at(i)) - average, 2),_about);
        result /= operands.size();
        return CalcHelper::CheckResult(result, _about);
    };
};


class ArithmeticProgression: public CompoundStart {
public:
    ArithmeticProgression() : CompoundStart("arithprog(",3) 
    {
        _about = "arithprog(base; ratio; elem_count)";
        _usecase = "Arithmetic progression. Ex: arithprog(10; 1; 3) = 10 + 11 + 12 = 33";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override 
    {
        if (operands.at(2) <= 0 )
            throw std::domain_error("Arithmetic progression: Number of elements cannot be negative or zero.");
        OperandType result = operands.at(2) * (2* operands.at(0) + operands.at(1) * (operands.at(2) - 1))/2;
        return CalcHelper::CheckResult(result, _about);
    }
};

class GeometricProgression : public CompoundStart {
public:
    GeometricProgression() : CompoundStart("geoprog(", 3)
    {
        _about = "geoprog(base; ratio; elem_count)";
        _usecase = "Geometric progression. Ex: geoprog(8; 1/2; 3) = 8 + 4 + 2 = 14";
    }
    OperandType Exec(vector<OperandType>& operands, const std::string str)  const override
    {
        if (round(operands.at(2)) < 0)
            throw std::domain_error("Geometric progression: Number of elements can't be negative.");
        if (round(operands.at(2)) == 0 && (operands.at(1) >= 1))
            throw std::domain_error("Geometric progression: Common ratio (second operand) must be < 1.");

        OperandType oper3 = round(operands.at( 2));
        OperandType result{};
        if ((operands.at(1)) == 1) // если это не прогрессия
            result = (operands.at(0)) * oper3;  //то вернуть колво членов умнож-е на первоый член
        else if ((!oper3) && (operands.at(1)) < 1) //это убывающая прогр-сия и нужна вся сумма
            result = operands.at(0) / CalcHelper::CheckDivisor(1 - operands.at(1));
        else //обычный случай
            result = (operands.at(0)) * (1 - pow(operands.at(1), oper3)) / CalcHelper::CheckDivisor(1 - operands.at(1));
        return CalcHelper::CheckResult(result, _about);
    };

};
