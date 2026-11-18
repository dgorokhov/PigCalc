#include <gtest/gtest.h>
#include <vector>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

// Подключите ваш заголовочный файл с классами узлов
#include "nodetypes.h" 


// ============================================================================
// 1. Тесты валидатора CalcHelper
// ============================================================================
TEST(CalcHelperTest, NormalValues) {
    EXPECT_NO_THROW(CalcHelper::CheckResult(42.5,""));
    EXPECT_NO_THROW(CalcHelper::CheckDivisor(0.001L));
}

TEST(CalcValidateTest, NaNThrowsDomainError) {
    OperandType nan_val = std::numeric_limits<OperandType>::quiet_NaN();
    EXPECT_THROW(CalcHelper::CheckResult(nan_val,""), std::domain_error);
}

TEST(CalcValidateTest, OverflowThrowsOverflowError) {
    OperandType inf_val = std::numeric_limits<OperandType>::infinity();
    EXPECT_THROW(CalcHelper::CheckResult(inf_val, ""), std::overflow_error);
    EXPECT_THROW(CalcHelper::CheckResult(-inf_val, ""), std::overflow_error);
}

TEST(CalcValidateTest, DivisorTooSmallThrowsOverflowError) {
    EXPECT_THROW(CalcHelper::CheckDivisor(0.0L), std::domain_error);
    EXPECT_NO_THROW(CalcHelper::CheckDivisor(MinNumber / 2.0L));
    EXPECT_NO_THROW(CalcHelper::CheckDivisor(-MinNumber / 2.0L));
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
    EXPECT_EQ(num.DoIRecognizeSelf("1e-5*2", 0), 4);
    EXPECT_EQ(num.DoIRecognizeSelf("+123", 0), -1);

    EXPECT_NEAR(num.Exec(empty_ops, "123.45"), 123.45L, MinNumber);
    
}

TEST(NumberTest, Exceptions) {
    Number num;
    std::vector<OperandType> empty_ops;

    EXPECT_THROW(num.Exec(empty_ops, "a12bc"), std::invalid_argument);
    EXPECT_THROW(num.Exec(empty_ops, "1e99999"), std::overflow_error);
}

TEST(BinaryNumberTest, RecognitionAndExecution) {
    BinaryNumber bin;
    std::vector<OperandType> empty_ops;

    EXPECT_EQ(bin.DoIRecognizeSelf("0b1011+1", 0), 6);
    EXPECT_EQ(bin.DoIRecognizeSelf("0b", 0), -1);      // Нет цифр после 0b
    EXPECT_EQ(bin.DoIRecognizeSelf("1011", 0), -1);    // Нет префикса 0b

    EXPECT_NEAR(bin.Exec(empty_ops, "0b1010"), 10.0L, MinNumber);
    EXPECT_NEAR(bin.Exec(empty_ops, "0b0"), 0.0L, MinNumber);
}

TEST(HexNumberTest, RecognitionAndExecution) {
    HexNumber hex;
    std::vector<OperandType> empty_ops;

    EXPECT_EQ(hex.DoIRecognizeSelf("0x1af+2", 0), 5);
    EXPECT_EQ(hex.DoIRecognizeSelf("0x", 0), -1);
    EXPECT_EQ(hex.DoIRecognizeSelf("1af", 0), -1);

    EXPECT_NEAR(hex.Exec(empty_ops, "0x1a"), 26.0L, MinNumber);
    EXPECT_NEAR(hex.Exec(empty_ops, "0xff"), 255.0L, MinNumber);
    EXPECT_NEAR(hex.Exec(empty_ops, "0x0"), 0.0L, MinNumber);
}

TEST(ConstantsTest, Pi) {
    Pi pi_node;
    std::vector<OperandType> ops = {};

    EXPECT_EQ(pi_node.DoIRecognizeSelf("pi*2", 0), 2);
    EXPECT_EQ(pi_node.DoIRecognizeSelf("p*2", 0), -1);
    EXPECT_NEAR(pi_node.Exec(ops, "pi"), NUMBER_PI, MinNumber);
}

TEST(ExponentTest, Exp) {
    Exponent exp_node;
    std::vector<OperandType> ops = { 2.0L};

    EXPECT_EQ(exp_node.DoIRecognizeSelf("exp(1)", 0), 4);
    EXPECT_NEAR(exp_node.Exec(ops, "exp"), std::exp(2.0L), MinNumber);
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

    EXPECT_NEAR(uminus.Exec(one_op, "-"), -15.5L, MinNumber);
    EXPECT_NEAR(add.Exec(two_ops, "+"), 14.0L, MinNumber);
    EXPECT_NEAR(sub.Exec(two_ops, "-"), 6.0L, MinNumber);
    EXPECT_NEAR(mul.Exec(two_ops, "*"), 40.0L, MinNumber);
}

TEST(DivisionTest, NormalAndZeroDivision) {
    Division div;
    std::vector<OperandType> normal_ops = {20.0L, 4.0L};
    std::vector<OperandType> zero_ops = {10.0L, 0.0L};
    std::vector<OperandType> tiny_ops = {10.0L, MinNumber / 10.0L};

    EXPECT_NEAR(div.Exec(normal_ops, "/"), 5.0L, MinNumber);
    EXPECT_THROW(div.Exec(zero_ops, "/"), std::logic_error);
    EXPECT_NO_THROW(div.Exec(tiny_ops, "/"));
}

// ============================================================================
// 4. Тесты простых математических функций (Factorial, Abs, Trunc, Round, Pow)
// ============================================================================
TEST(FactorialTest, NormalAndEdgeCases) {
    Factorial fac;
    std::vector<OperandType> op5 = {5.0L};
    std::vector<OperandType> op0 = {0.0L};

    EXPECT_NEAR(fac.Exec(op5, "fac("), 120.0L, MinNumber);
    EXPECT_NEAR(fac.Exec(op0, "fac("), 1.0L, MinNumber);
}

TEST(RoundingAndAbsTest, Executions) {
    Abs abs_node;
    Trunc trunc_node;
    Round round_node;

    std::vector<OperandType> neg_frac = {-3.7L};
    std::vector<OperandType> pos_frac = {3.2L};

    EXPECT_NEAR(abs_node.Exec(neg_frac, "abs("), 3.7L, MinNumber);
    EXPECT_NEAR(abs_node.Exec(pos_frac, "abs("), 3.2L, MinNumber);

    EXPECT_NEAR(trunc_node.Exec(neg_frac, "trunc("), -3.0L, MinNumber);
    EXPECT_NEAR(trunc_node.Exec(pos_frac, "trunc("), 3.0L, MinNumber);

    EXPECT_NEAR(round_node.Exec(neg_frac, "round("), -4.0L, MinNumber);
    EXPECT_NEAR(round_node.Exec(pos_frac, "round("), 3.0L, MinNumber);
}

TEST(PowTest, NormalAndEdgeCases) {
    Pow pow_node;
    std::vector<OperandType> op_normal = {2.0L, 3.0L};
    std::vector<OperandType> op_zero_exp = {5.0L, 0.0L};
    std::vector<OperandType> op_neg_exp = {2.0L, -2.0L};

    EXPECT_NEAR(pow_node.Exec(op_normal, "pow("), 8.0L, MinNumber);
    EXPECT_NEAR(pow_node.Exec(op_zero_exp, "pow("), 1.0L, MinNumber);
    EXPECT_NEAR(pow_node.Exec(op_neg_exp, "pow("), 0.25L, MinNumber);
}

// ============================================================================
// 5. Тесты тригонометрии (Sines, Cosines, Tangents, Cotangents)
// ============================================================================
TEST(TrigTest, SinAndCos) {
    Sines sin_node;
    Cosines cos_node;
    
    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> half_pi = {NUMBER_PI / 2.0L};
    std::vector<OperandType> quorter_pi = {NUMBER_PI / 4.0L};
    auto res_for_qpi = static_cast<OperandType>(std::sqrt(2.0L)/2.0L);
    EXPECT_NEAR(sin_node.Exec(quorter_pi, "sin("), res_for_qpi, MinNumber);
    EXPECT_NEAR(cos_node.Exec(quorter_pi, "cos("), res_for_qpi, MinNumber);

    EXPECT_EQ(sin_node.Exec(zero, "sin("), 0.0L);
    EXPECT_EQ(sin_node.Exec(half_pi, "sin("), 1.0L);
    EXPECT_EQ(cos_node.Exec(zero, "cos("), 1.0L);
    EXPECT_EQ(cos_node.Exec(half_pi, "cos("), 0.0L);

}

TEST(TrigTest, TanAndCotan) {
    Tangents tan_node;
    Cotangents cot_node;
    OperandType pi = std::numbers::pi_v<OperandType>;

    std::vector<OperandType> half_pi = {NUMBER_PI / 2.0L};
    std::vector<OperandType> quarter_pi = {NUMBER_PI / 4.0L};
    std::vector<OperandType> zero = {0.0L};

    EXPECT_NEAR(tan_node.Exec(quarter_pi, "tan("), 1.0L, MinNumber);
    EXPECT_NEAR(cot_node.Exec(quarter_pi, "cotan("), 1.0L, MinNumber);

    // Граничный случай: cotan(0) -> sin(0) == 0 -> исключение
    EXPECT_THROW(tan_node.Exec(half_pi, "tan("), std::domain_error);
    EXPECT_THROW(cot_node.Exec(zero, "cotan("), std::domain_error);
    
}

// ============================================================================
// Тесты обратных тригонометрических функций
// ============================================================================
TEST(InverseTrigTest, ArcSines) {
    ArcSines node;
    const OperandType pi = std::numbers::pi_v<OperandType>;

    // Нормальная работа и граничные точки [-1, 1]
    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> one = {1.0L};
    std::vector<OperandType> neg_one = {-1.0L};

    EXPECT_NEAR(node.Exec(zero, "asin("), 0.0L, MinNumber);
    EXPECT_NEAR(node.Exec(one, "asin("), pi / 2.0L, MinNumber);
    EXPECT_NEAR(node.Exec(neg_one, "asin("), -pi / 2.0L, MinNumber);
    // Исключения при выходе за [-1, 1]
    std::vector<OperandType> out_high = {1.0001L};
    std::vector<OperandType> out_low = {-1.0001L};
    EXPECT_THROW(node.Exec(out_high, "asin("), std::domain_error);
    EXPECT_THROW(node.Exec(out_low, "asin("), std::domain_error);
}

TEST(InverseTrigTest, ArcCosines) {
    ArcCosines node;
    const OperandType pi = std::numbers::pi_v<OperandType>;

    std::vector<OperandType> one = {1.0L};
    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> neg_one = {-1.0L};

    EXPECT_NEAR(node.Exec(one, "acos("), 0.0L, MinNumber);
    EXPECT_NEAR(node.Exec(zero, "acos("), pi / 2.0L, MinNumber);
    EXPECT_NEAR(node.Exec(neg_one, "acos("), pi, MinNumber);

    std::vector<OperandType> out_high = {1.5L};
    std::vector<OperandType> out_low = {-2.0L};
    EXPECT_THROW(node.Exec(out_high, "acos("), std::domain_error);
    EXPECT_THROW(node.Exec(out_low, "acos("), std::domain_error);
}

TEST(InverseTrigTest, ArcTangentsAndArcCotangents) {
    ArcTangents atan_node;
    ArcCotangents acot_node;
    const OperandType pi = std::numbers::pi_v<OperandType>;

    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> one = {1.0L};
    std::vector<OperandType> neg_one = {-1.0L};

    EXPECT_NEAR(atan_node.Exec(zero, "atan("), 0.0L, MinNumber);
    EXPECT_NEAR(atan_node.Exec(one, "atan("), pi / 4.0L, MinNumber);
    EXPECT_NEAR(atan_node.Exec(neg_one, "atan("), -pi / 4.0L, MinNumber);

    EXPECT_NEAR(acot_node.Exec(zero, "acot("), pi / 2.0L, MinNumber);
    EXPECT_NEAR(acot_node.Exec(one, "acot("), pi / 4.0L, MinNumber);
    EXPECT_NEAR(acot_node.Exec(neg_one, "acot("), 3.0L * pi / 4.0L, MinNumber);
}

// ============================================================================
// Тесты гиперболических функций
// ============================================================================
TEST(HyperbolicTest, SinhAndCosh) {
    HyperbolicSines sinh_node;
    HyperbolicCosines cosh_node;

    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> one = {1.0L};
    std::vector<OperandType> huge = {20000.0L}; // Переполнение для 80-bit long OperandType (макс ~11356)

    EXPECT_NEAR(sinh_node.Exec(zero, "sinh("), 0.0L, MinNumber);
    EXPECT_NEAR(sinh_node.Exec(one, "sinh("), 1.17520119364380145688L, MinNumber);
    EXPECT_THROW(sinh_node.Exec(huge, "sinh("), std::overflow_error);

    EXPECT_NEAR(cosh_node.Exec(zero, "cosh("), 1.0L, MinNumber);
    EXPECT_NEAR(cosh_node.Exec(one, "cosh("), 1.54308063481524377848L, MinNumber);
    EXPECT_THROW(cosh_node.Exec(huge, "cosh("), std::overflow_error);
}

TEST(HyperbolicTest, TanhAndCoth) {
    HyperbolicTangents tanh_node;
    HyperbolicCotangents coth_node;

    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> one = {1.0L};
    std::vector<OperandType> big_pos = {20000.0L};
    std::vector<OperandType> big_neg = {-20000.0L};

    EXPECT_NEAR(tanh_node.Exec(zero, "tanh("), 0.0L, MinNumber);
    EXPECT_NEAR(tanh_node.Exec(one, "tanh("), 0.76159415595576488812L, MinNumber);
    EXPECT_NEAR(tanh_node.Exec(big_pos, "tanh("), 1.0L, MinNumber);

    // Проверка coth: нормальное значение, асимптота на бесконечности и исключение в 0
    EXPECT_NEAR(coth_node.Exec(one, "coth("), 1.31303528549933130364L, MinNumber);
    EXPECT_NEAR(coth_node.Exec(big_pos, "coth("), 1.0L, MinNumber);
    EXPECT_NEAR(coth_node.Exec(big_neg, "coth("), -1.0L, MinNumber);
    EXPECT_THROW(coth_node.Exec(zero, "coth("), std::domain_error);
}

// ============================================================================
// Тесты обратных гиперболических функций
// ============================================================================
TEST(AreaHyperbolicTest, AsinhAcoshAtanh) {
    AreaSines asinh_node;
    AreaCosines acosh_node;
    AreaTangents atanh_node;

    std::vector<OperandType> zero = {0.0L};
    std::vector<OperandType> one = {1.0L};
    std::vector<OperandType> two = {2.0L};
    std::vector<OperandType> half = {0.5L};
    std::vector<OperandType> neg_one = {-1.0L};


    // asinh: определена на всей числовой прямой
    EXPECT_NEAR(asinh_node.Exec(zero, "asinh("), 0.0L, MinNumber);
    EXPECT_NEAR(asinh_node.Exec(one, "asinh("), 0.88137358701954302523L, MinNumber);

    // acosh: определена при x >= 1
    EXPECT_NEAR(acosh_node.Exec(one, "acosh("), 0.0L, MinNumber);
    EXPECT_NEAR(acosh_node.Exec(two, "acosh("), 1.31695789692481670863L, MinNumber);
    EXPECT_THROW(acosh_node.Exec(half, "acosh("), std::domain_error);
    EXPECT_THROW(acosh_node.Exec(zero, "acosh("), std::domain_error);

    // atanh: определена строго внутри (-1, 1)
    EXPECT_NEAR(atanh_node.Exec(zero, "atanh("), 0.0L, MinNumber);
    EXPECT_NEAR(atanh_node.Exec(half, "atanh("), 0.54930614433405484570L, MinNumber);
    EXPECT_THROW(atanh_node.Exec(one, "atanh("), std::domain_error);
    EXPECT_THROW(atanh_node.Exec(neg_one, "atanh("), std::domain_error);
}


// ============================================================================
// 6. Тесты логарифмов (NaturalLogarithm, Logarithm, Log10)
// ============================================================================
TEST(LogarithmTest, NaturalLog) {
    NaturalLogarithm ln_node;
    std::vector<OperandType> op_e = {std::numbers::e_v<OperandType>};
    std::vector<OperandType> op_one = {1.0L};
    std::vector<OperandType> op_neg = {-5.0L};

    EXPECT_NEAR(ln_node.Exec(op_e, "ln("), 1.0L, MinNumber);
    EXPECT_NEAR(ln_node.Exec(op_one, "ln("), 0.0L, MinNumber);
    EXPECT_THROW(ln_node.Exec(op_neg, "ln("), std::domain_error);
}

TEST(LogarithmTest, CustomBaseLog) {
    Logarithm log_node;
    std::vector<OperandType> valid = {2.0L, 8.0L};
    std::vector<OperandType> neg_arg1= {8.0L, -2.0L};
    std::vector<OperandType> neg_arg2= {-2.0L, 12.0L};
    std::vector<OperandType> base_one = {1.0L, 11.0L}; // log(1) == 0 -> деление на 0

    EXPECT_NEAR(log_node.Exec(valid, "log("), 3.0L, MinNumber);
    EXPECT_THROW(log_node.Exec(neg_arg1, "log("), std::domain_error);
    EXPECT_THROW(log_node.Exec(neg_arg2, "log("), std::domain_error);
    EXPECT_THROW(log_node.Exec(base_one, "log("), std::domain_error);
}

TEST(LogarithmTest, LgFunction) {
    Log10 lg_node;
    std::vector<OperandType> op_eight = {100.0L};
    std::vector<OperandType> op_neg = {-1.0L};

    // Согласно _usecase в коде: "Binary logarithm of X (2 is the base)"
    EXPECT_NEAR(lg_node.Exec(op_eight, "lg("), 2.0L, MinNumber);
    EXPECT_THROW(lg_node.Exec(op_neg, "lg("), std::domain_error);
}

// ============================================================================
// 7. Тесты статистических функций с переменным числом аргументов
// ============================================================================
TEST(VariadicStatsTest, MinAndMax) {
    Minimum min_node;
    Maximum max_node;

    std::vector<OperandType> ops = {10.0L, -5.5L, 42.0L, 0.0L};
    std::vector<OperandType> single = {7.0L};

    EXPECT_NEAR(min_node.Exec(ops, "min("), -5.5L, MinNumber);
    EXPECT_NEAR(min_node.Exec(single, "min("), 7.0L, MinNumber);

    EXPECT_NEAR(max_node.Exec(ops, "max("), 42.0L, MinNumber);
    EXPECT_NEAR(max_node.Exec(single, "max("), 7.0L, MinNumber);
}

TEST(VariadicStatsTest, MeansAndMedian) {
    ArithmeticMean amean;
    GeometricMean gmean;
    HarmonicMean hmean;
    Median median;

    std::vector<OperandType> ops_even = {2.0L, 8.0L};
    std::vector<OperandType> ops_odd = {10.0L, 2.0L, 8.0L};

    // Проверка на выявление бага с пропуском 0-го элемента
    EXPECT_NEAR(amean.Exec(ops_even, "arithmean("), 5.0L, MinNumber);
    EXPECT_NEAR(gmean.Exec(ops_even, "geomean("), 4.0L, MinNumber);
    EXPECT_NEAR(hmean.Exec(ops_even, "harmean("), 3.2L, MinNumber);
    EXPECT_NEAR(median.Exec(ops_odd, "median("), 8.0L, MinNumber);
    EXPECT_NEAR(median.Exec(ops_even, "median("), 5.0L, MinNumber);
}

TEST(VariadicStatsTest, Variance) {
    Variance var_node;
    std::vector ops = {2.0L, 4.0L, 4.0L, 4.0L, 5.0L, 5.0L, 7.0L, 9.0L};
    std::vector same = {5.0L, 5.0L, 5.0L};
    // Дисперсия для этого набора равна 4.0
    EXPECT_NEAR(var_node.Exec(ops, "var("), 4.0L, MinNumber);
    EXPECT_NEAR(var_node.Exec(same, "var("), 0.0L, MinNumber);
}

// ============================================================================
// 8. Тесты прогрессий (ArithmeticProgression, GeometricProgression)
// ============================================================================

TEST(ProgressionTest, Arithmetic) {
    ArithmeticProgression ap;
    std::vector normal = {10.0L, 1.0L, 3.0L};     // 10 + 11 + 12 = 33
    std::vector invalid_n = {10.0L, 1.0L, 0.0L};  // N <= 0
    EXPECT_NEAR(ap.Exec(normal, "arithprog("), 33.0L, MinNumber);
    EXPECT_THROW(ap.Exec(invalid_n, "arithprog("), std::domain_error);
}

TEST(ProgressionTest, Geometric) {
    GeometricProgression gp;
    std::vector normal = {8.0L, 0.5L, 3.0L};       // 8 + 4 + 2 = 14
    std::vector ratio_one = {5.0L, 1.0L, 4.0L};    // 5 * 4 = 20
    std::vector inf_sum = {8.0L, 0.5L, 0.0L};      // Бесконечная убывающая: 8 / (1 - 0.5) = 16
    std::vector neg_n = {8.0L, 0.5L, -2.0L};       // N < 0 -> ошибка
    std::vector bad_inf = {8.0L, 1.5L, 0.0L};      // Бесконечная при q >= 1 -> ошибка
    EXPECT_NEAR(gp.Exec(normal, "geoprog("), 14.0L, MinNumber);
    EXPECT_NEAR(gp.Exec(ratio_one, "geoprog("), 20.0L, MinNumber);
    EXPECT_NEAR(gp.Exec(inf_sum, "geoprog("), 16.0L, MinNumber);
    EXPECT_THROW(gp.Exec(neg_n, "geoprog("), std::domain_error);
    EXPECT_THROW(gp.Exec(bad_inf, "geoprog("), std::domain_error);
}
