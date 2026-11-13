#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

// Подключите ваш заголовочный файл с классами узлов
#include "nodetypes.h" 

// Вспомогательная константа для сравнения long double
constexpr OperandType EPS = 1e-12L;

// ============================================================================
// 1. Тесты валидатора CalcValidate
// ============================================================================
TEST(CalcValidateTest, NormalValues) {
    EXPECT_NO_THROW(CalcValidate::Uncertainty(42.5));
    EXPECT_NO_THROW(CalcValidate::Divisor(0.001L));
}

TEST(CalcValidateTest, NaNThrowsDomainError) {
    double nan_val = std::numeric_limits<double>::quiet_NaN();
    EXPECT_THROW(CalcValidate::Uncertainty(nan_val), std::domain_error);
}

TEST(CalcValidateTest, OverflowThrowsOverflowError) {
    double inf_val = std::numeric_limits<double>::infinity();
    EXPECT_THROW(CalcValidate::Uncertainty(inf_val), std::overflow_error);
    EXPECT_THROW(CalcValidate::Uncertainty(-inf_val), std::overflow_error);
}

TEST(CalcValidateTest, DivisorTooSmallThrowsOverflowError) {
    EXPECT_THROW(CalcValidate::Divisor(0.0L), std::overflow_error);
    EXPECT_THROW(CalcValidate::Divisor(MinNumber / 2.0L), std::overflow_error);
    EXPECT_THROW(CalcValidate::Divisor(-MinNumber / 2.0L), std::overflow_error);
}

// ============================================================================
// 2. Тесты классов чисел и констант (Number, BinaryNumber, HexNumber, Pi, Exponent)
// ============================================================================
TEST(NumberTest, RecognitionAndExecution) {
    Number num;
    std::vector<OperandType> empty_ops;
    //DoIRecognizeSelf возвращает номер позиции в строке сразу после распознанного токена
    //либо -1 если не распознали
    EXPECT_EQ(num.DoIRecognizeSelf("123.45+6", 0), 6);
    EXPECT_EQ(num.DoIRecognizeSelf("e-5*2", 0), -1);
    EXPECT_EQ(num.DoIRecognizeSelf("1e-5*2", 0), 1);
    EXPECT_EQ(num.DoIRecognizeSelf("+123", 0), -1);

    EXPECT_NEAR(num.Exec(empty_ops, "123.45"), 123.45L, EPS);
    
}

TEST(NumberTest, Exceptions) {
    Number num;
    std::vector<OperandType> empty_ops;

    EXPECT_THROW(num.Exec(empty_ops, "a12bc"), std::runtime_error);
    EXPECT_THROW(num.Exec(empty_ops, "1e99999"), std::runtime_error);
}

TEST(BinaryNumberTest, RecognitionAndExecution) {
    BinaryNumber bin;
    std::vector<OperandType> empty_ops;

    EXPECT_EQ(bin.DoIRecognizeSelf("0b1011+1", 0), 6);
    EXPECT_EQ(bin.DoIRecognizeSelf("0b", 0), -1);      // Нет цифр после 0b
    EXPECT_EQ(bin.DoIRecognizeSelf("1011", 0), -1);    // Нет префикса 0b

    EXPECT_NEAR(bin.Exec(empty_ops, "0b1010"), 10.0L, EPS);
    EXPECT_NEAR(bin.Exec(empty_ops, "0b0"), 0.0L, EPS);
}

TEST(HexNumberTest, RecognitionAndExecution) {
    HexNumber hex;
    std::vector<OperandType> empty_ops;

    EXPECT_EQ(hex.DoIRecognizeSelf("0x1AF+2", 0), 5);
    EXPECT_EQ(hex.DoIRecognizeSelf("0x", 0), -1);
    EXPECT_EQ(hex.DoIRecognizeSelf("1AF", 0), -1);

    EXPECT_NEAR(hex.Exec(empty_ops, "0x1A"), 26.0L, EPS);
    EXPECT_NEAR(hex.Exec(empty_ops, "0xFF"), 255.0L, EPS);
    EXPECT_NEAR(hex.Exec(empty_ops, "0x0"), 0.0L, EPS);
}

TEST(ConstantsTest, PiAndExponent) {
    Pi pi_node;
    Exponent exp_node;
    std::vector<OperandType> empty_ops;

    EXPECT_EQ(pi_node.DoIRecognizeSelf("pi*2", 0), 2);
    EXPECT_EQ(pi_node.DoIRecognizeSelf("p*2", 0), -1);
    EXPECT_NEAR(pi_node.Exec(empty_ops, "pi"), std::numbers::pi_v<OperandType>, EPS);

    EXPECT_EQ(exp_node.DoIRecognizeSelf("exp+1", 0), 3);
    EXPECT_NEAR(exp_node.Exec(empty_ops, "exp"), std::numbers::e_v<OperandType>, EPS);
}

// ============================================================================
// 3. Тесты базовых операций (UnaryMinus, Addition, Substraction, Multiplication, Division)
// ============================================================================
TEST(BasicOperationsTest, UnaryAndBinaryMath) {
    UnaryMinus uminus;
    Addition add;
    Substraction sub;
    Multiplication mul;

    std::vector<OperandType> one_op = {15.5L};
    std::vector<OperandType> two_ops = {10.0L, 4.0L};

    EXPECT_NEAR(uminus.Exec(one_op, "-"), -15.5L, EPS);
    EXPECT_NEAR(add.Exec(two_ops, "+"), 14.0L, EPS);
    EXPECT_NEAR(sub.Exec(two_ops, "-"), 6.0L, EPS);
    EXPECT_NEAR(mul.Exec(two_ops, "*"), 40.0L, EPS);
}

TEST(DivisionTest, NormalAndZeroDivision) {
    Division div;
    std::vector<OperandType> normal_ops = {20.0L, 4.0L};
    std::vector<OperandType> zero_ops = {10.0L, 0.0L};
    std::vector<OperandType> tiny_ops = {10.0L, MinNumber / 10.0L};

    EXPECT_NEAR(div.Exec(normal_ops, "/"), 5.0L, EPS);
    EXPECT_THROW(div.Exec(zero_ops, "/"), std::runtime_error);
    EXPECT_THROW(div.Exec(tiny_ops, "/"), std::runtime_error);
}

// ============================================================================
// 4. Тесты простых математических функций (Factorial, Abs, Trunc, Round, Pow)
// ============================================================================
TEST(FactorialTest, NormalAndEdgeCases) {
    Factorial fac;
    std::vector<OperandType> op5 = {5.0L};
    std::vector<OperandType> op0 = {0.0L};

    EXPECT_NEAR(fac.Exec(op5, "fac("), 120.0L, EPS);
    EXPECT_NEAR(fac.Exec(op0, "fac("), 1.0L, EPS);
}

TEST(RoundingAndAbsTest, Executions) {
    Abs abs_node;
    Trunc trunc_node;
    Round round_node;

    std::vector<OperandType> neg_frac = {-3.7L};
    std::vector<OperandType> pos_frac = {3.2L};

    EXPECT_NEAR(abs_node.Exec(neg_frac, "abs("), 3.7L, EPS);
    EXPECT_NEAR(abs_node.Exec(pos_frac, "abs("), 3.2L, EPS);

    EXPECT_NEAR(trunc_node.Exec(neg_frac, "trunc("), -3.0L, EPS);
    EXPECT_NEAR(trunc_node.Exec(pos_frac, "trunc("), 3.0L, EPS);

    EXPECT_NEAR(round_node.Exec(neg_frac, "round("), -4.0L, EPS);
    EXPECT_NEAR(round_node.Exec(pos_frac, "round("), 3.0L, EPS);
}

TEST(PowTest, NormalAndEdgeCases) {
    Pow pow_node;
    std::vector<OperandType> op_normal = {2.0L, 3.0L};
    std::vector<OperandType> op_zero_exp = {5.0L, 0.0L};
    std::vector<OperandType> op_neg_exp = {2.0L, -2.0L};

    EXPECT_NEAR(pow_node.Exec(op_normal, "pow("), 8.0L, EPS);
    EXPECT_NEAR(pow_node.Exec(op_zero_exp, "pow("), 1.0L, EPS);
    EXPECT_NEAR(pow_node.Exec(op_neg_exp, "pow("), 0.25L, EPS);
}

// ============================================================================
// 5. Тесты тригонометрии (Sines, Cosines, Tangents, Cotangents)
// ============================================================================
TEST(TrigTest, SinAndCos) {
    Sines sin_node;
    Cosines cos_node;
    OperandType pi = std::numbers::pi_v<OperandType>;

    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> half_pi = {pi / 2.0L};

    EXPECT_NEAR(sin_node.Exec(zero, "sin("), 0.0L, EPS);
    EXPECT_NEAR(sin_node.Exec(half_pi, "sin("), 1.0L, EPS);

    EXPECT_NEAR(cos_node.Exec(zero, "cos("), 1.0L, EPS);
    EXPECT_NEAR(cos_node.Exec(half_pi, "cos("), 0.0L, EPS);
}

TEST(TrigTest, TanAndCotan) {
    Tangents tan_node;
    Cotangents cot_node;
    OperandType pi = std::numbers::pi_v<OperandType>;

    std::vector<OperandType> quarter_pi = {pi / 4.0L};
    std::vector<OperandType> zero = {0.0L};

    EXPECT_NEAR(tan_node.Exec(quarter_pi, "tan("), 1.0L, EPS);
    EXPECT_NEAR(cot_node.Exec(quarter_pi, "cotan("), 1.0L, EPS);

    // Граничный случай: cotan(0) -> sin(0) == 0 -> исключение
    EXPECT_THROW(cot_node.Exec(zero, "cotan("), std::runtime_error);
}

// ============================================================================
// 6. Тесты логарифмов (NaturalLogarithm, Logarithm, Log10)
// ============================================================================
TEST(LogarithmTest, NaturalLog) {
    NaturalLogarithm ln_node;
    std::vector<OperandType> op_e = {std::numbers::e_v<OperandType>};
    std::vector<OperandType> op_one = {1.0L};
    std::vector<OperandType> op_neg = {-5.0L};

    EXPECT_NEAR(ln_node.Exec(op_e, "ln("), 1.0L, EPS);
    EXPECT_NEAR(ln_node.Exec(op_one, "ln("), 0.0L, EPS);
    EXPECT_THROW(ln_node.Exec(op_neg, "ln("), std::runtime_error);
}

TEST(LogarithmTest, CustomBaseLog) {
    Logarithm log_node;
    std::vector<OperandType> valid = {8.0L, 2.0L};
    std::vector<OperandType> neg_arg = {-8.0L, 2.0L};
    std::vector<OperandType> base_one = {8.0L, 1.0L}; // log(1) == 0 -> деление на 0

    EXPECT_NEAR(log_node.Exec(valid, "log("), 3.0L, EPS);
    EXPECT_THROW(log_node.Exec(neg_arg, "log("), std::runtime_error);
    EXPECT_THROW(log_node.Exec(base_one, "log("), std::runtime_error);
}

TEST(LogarithmTest, LgFunction) {
    Log10 lg_node;
    std::vector<OperandType> op_eight = {100.0L};
    std::vector<OperandType> op_neg = {-1.0L};

    // Согласно _usecase в коде: "Binary logarithm of X (2 is the base)"
    EXPECT_NEAR(lg_node.Exec(op_eight, "lg("), 2.0L, EPS);
    EXPECT_THROW(lg_node.Exec(op_neg, "lg("), std::runtime_error);
}

// ============================================================================
// 7. Тесты статистических функций с переменным числом аргументов
// ============================================================================
TEST(VariadicStatsTest, MinAndMax) {
    Minimum min_node;
    Maximum max_node;

    std::vector<OperandType> ops = {10.0L, -5.5L, 42.0L, 0.0L};
    std::vector<OperandType> single = {7.0L};

    EXPECT_NEAR(min_node.Exec(ops, "min("), -5.5L, EPS);
    EXPECT_NEAR(min_node.Exec(single, "min("), 7.0L, EPS);

    EXPECT_NEAR(max_node.Exec(ops, "max("), 42.0L, EPS);
    EXPECT_NEAR(max_node.Exec(single, "max("), 7.0L, EPS);
}

TEST(VariadicStatsTest, MeansAndMedian) {
    ArithmeticMean amean;
    GeometricMean gmean;
    HarmonicMean hmean;
    Median median;

    std::vector<OperandType> ops_even = {2.0L, 8.0L};
    std::vector<OperandType> ops_odd = {10.0L, 2.0L, 8.0L};

    // Проверка на выявление бага с пропуском 0-го элемента
    EXPECT_NEAR(amean.Exec(ops_even, "arithmean("), 5.0L, EPS);
    EXPECT_NEAR(gmean.Exec(ops_even, "geomean("), 4.0L, EPS);
    EXPECT_NEAR(hmean.Exec(ops_even, "harmean("), 3.2L, EPS);
    EXPECT_NEAR(median.Exec(ops_odd, "median("), 8.0L, EPS);
    EXPECT_NEAR(median.Exec(ops_even, "median("), 5.0L, EPS);
}

TEST(VariadicStatsTest, Variance) {
    Variance var_node;
    std::vector ops = {2.0L, 4.0L, 4.0L, 4.0L, 5.0L, 5.0L, 7.0L, 9.0L};
    std::vector same = {5.0L, 5.0L, 5.0L};
    // Дисперсия для этого набора равна 4.0
    EXPECT_NEAR(var_node.Exec(ops, "var("), 4.0L, EPS);
    EXPECT_NEAR(var_node.Exec(same, "var("), 0.0L, EPS);
}

// ============================================================================
// 8. Тесты прогрессий (ArithmeticProgression, GeometricProgression)
// ============================================================================

TEST(ProgressionTest, Arithmetic) {
    ArithmeticProgression ap;
    std::vector normal = {10.0L, 1.0L, 3.0L};     // 10 + 11 + 12 = 33
    std::vector invalid_n = {10.0L, 1.0L, 0.0L};  // N <= 0
    EXPECT_NEAR(ap.Exec(normal, "arithprog("), 33.0L, EPS);
    EXPECT_THROW(ap.Exec(invalid_n, "arithprog("), std::runtime_error);
}

TEST(ProgressionTest, Geometric) {
    GeometricProgression gp;
    std::vector normal = {8.0L, 0.5L, 3.0L};       // 8 + 4 + 2 = 14
    std::vector ratio_one = {5.0L, 1.0L, 4.0L};    // 5 * 4 = 20
    std::vector inf_sum = {8.0L, 0.5L, 0.0L};      // Бесконечная убывающая: 8 / (1 - 0.5) = 16
    std::vector neg_n = {8.0L, 0.5L, -2.0L};       // N < 0 -> ошибка
    std::vector bad_inf = {8.0L, 1.5L, 0.0L};      // Бесконечная при q >= 1 -> ошибка
    EXPECT_NEAR(gp.Exec(normal, "geoprog("), 14.0L, EPS);
    EXPECT_NEAR(gp.Exec(ratio_one, "geoprog("), 20.0L, EPS);
    EXPECT_NEAR(gp.Exec(inf_sum, "geoprog("), 16.0L, EPS);
    EXPECT_THROW(gp.Exec(neg_n, "geoprog("), std::runtime_error);
    EXPECT_THROW(gp.Exec(bad_inf, "geoprog("), std::runtime_error);
}
