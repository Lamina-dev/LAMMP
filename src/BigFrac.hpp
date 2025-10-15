/*
 * [LAMINA_SCI_CAL]
 * Copyright (C) [2025] [HJimmyK/LAMINA]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
/*
MIT License

Copyright (c) 2024-2050 Twilight-Dream & With-Sky

https://github.com/Twilight-Dream-Of-Magic/
https://github.com/With-Sky

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef LAMINA_BIG_FRACTION_HPP
#define LAMINA_BIG_FRACTION_HPP
#include "BigInt.hpp"

namespace __LAMINA::BIGFRAC
{
	/*
		BigFraction 类的两种精度模式（DecimalPrecisionMode::FixedFixed 和 DecimalPrecisionMode::Full）
		区别主要体现在小数形式的字符串转换（ComputeAndToDecimalString 方法）中，具体如下：
		1. DecimalPrecisionMode::Fixed（固定精度模式）
			特点：
				按照预设的固定小数位数生成字符串，位数由 FixedPrecisionCount 成员变量指定。
			转换逻辑：
				计算分数的整数部分和余数（小数部分的初始值）。
				循环 FixedPrecisionCount 次，每次将余数乘以 10（基数），取商作为当前小数位，更新余数为新的模。
				若余数提前变为 0，可提前终止循环（后续小数位均为 0）。
			示例：
				若 FixedPrecisionCount = 3，则无论分数是否能被除尽，最终字符串都最多保留 3 位小数。
		2. DecimalPrecisionMode::Full（完全精度模式）
			特点：
				根据 BigFractionFullPrecision 成员变量定义的精度（分母为 10 的幂），生成尽可能完整的小数位数，直到达到预设精度或余数为 0。
			转换逻辑：
				通过 GetFullPrecision() 获取预设精度（其分母必须是 10 的幂，如 1000 表示保留 3 位小数）。
				计算所需的小数位数：通过对精度分母反复除以 10，得到 10 的幂次（如 1000 → 3 位）。
				循环生成小数位，直到达到所需位数或余数为 0。
			示例：
				若 BigFractionFullPrecision 的分母为 10000（10⁴），则最多生成 4 位小数。
	*/
	enum class DecimalPrecisionMode : uint32_t
	{
		Fixed = 0, // Specify the precision mode
		Full = 1   // Full the precision mode
	};

	inline const __LAMINA::BIGINT::BigInteger ONE = 1;
	inline const __LAMINA::BIGINT::BigInteger TWO = 2;
	inline const __LAMINA::BIGINT::BigInteger THREE = 3;
	inline const __LAMINA::BIGINT::BigInteger FIVE = 5;
	inline const __LAMINA::BIGINT::BigInteger TEN = 10;
	inline const __LAMINA::BIGINT::BigInteger RADIX = TEN;

	class BigFraction
	{
	public:
		DecimalPrecisionMode PrecisionMode = DecimalPrecisionMode::Fixed;
		uint64_t FixedPrecisionCount = 2;

		using BigInteger = __LAMINA::BIGINT::BigInteger;
		using BigSignedInteger = __LAMINA::BIGINT::BigSignedInteger;

		BigFraction();
		BigFraction(const BigFraction &other) noexcept;
		BigFraction(BigFraction &&other) noexcept;
		explicit BigFraction(const BigInteger &numerator);
		BigFraction(const BigInteger &numerator, const BigInteger &denominator);
		BigFraction(const BigInteger &numerator, const BigInteger &denominator, int32_t sign);
		explicit BigFraction(const std::string &complexString);

		void SetSimplifyReduced(bool value);
		bool IsNaN() const;
		bool IsInfinityPoint() const;
		bool IsZero() const;
		bool IsNegative() const;
		bool IsInteger() const;
		BigInteger GetNumerator() const;
		BigInteger GetDenominator() const;
		std::optional<BigInteger> TryGetInteger() const;
		// 设置分子
		void SetNumerator(const BigInteger &number);
		// 设置分母
		void SetDenominator(const BigInteger &number);
		BigFraction GetFullPrecision() const;
		void SetFullPrecision(BigInteger number);

		void ComputeAndFromDecimalString(const std::string &complexString);
		std::string ComputeAndToDecimalString() const;

		BigFraction &operator=(const BigFraction &other);
		BigFraction &operator=(BigFraction &&other);
		BigFraction &operator+=(const BigFraction &other);
		BigFraction &operator+=(const BigInteger &other);
		BigFraction &operator-=(const BigFraction &other);
		BigFraction &operator-=(const BigInteger &other);
		BigFraction &operator*=(const BigFraction &other);
		BigFraction &operator*=(const BigInteger &other);
		BigFraction &operator/=(const BigFraction &other);
		BigFraction &operator/=(const BigInteger &other);

		BigFraction operator+(const BigFraction &other) const;
		BigFraction operator+(const BigInteger &other) const;
		BigFraction operator-(const BigFraction &other) const;
		BigFraction operator-(const BigInteger &other) const;
		BigFraction operator*(const BigFraction &other) const;
		BigFraction operator*(const BigInteger &other) const;
		BigFraction operator/(const BigFraction &other) const;
		BigFraction operator/(const BigInteger &other) const;

		BigFraction operator-() const;

		bool operator<(const BigFraction &other) const;
		bool operator<=(const BigFraction &other) const;
		bool operator>(const BigFraction &other) const;
		bool operator>=(const BigFraction &other) const;
		bool operator==(const BigFraction &other) const;
		bool operator!=(const BigFraction &other) const;

		BigFraction Abs() const;
		BigFraction Reciprocal() const;
		BigFraction Sqrt();
		BigFraction Cbrt();
		BigFraction Log(const BigInteger &value) const;
		BigFraction Log() const;
		BigFraction Log10(const BigInteger &fraction) const;
		BigFraction Log10() const;
		// nth Power of a BigFraction
		BigFraction Power(const BigInteger &exponent) const;
		BigFraction Power(const BigFraction &exponent) const;
		// nth Root of a BigFraction
		BigFraction NthRoot(const BigInteger &n) const;
		BigFraction NthRoot(const BigFraction &fraction, const BigInteger &n) const;
		BigFraction Sine(const BigFraction &x) const;
		BigFraction Cosine(const BigFraction &x) const;
		BigFraction Tangent(const BigFraction &x) const;
		BigFraction Arctangent(const BigFraction &x) const;
		BigFraction Arcsine(const BigFraction &x) const;
		BigFraction Arccosine(const BigFraction &x) const;

		BigInteger Floor() const;
		BigInteger Ceil() const;
		BigInteger Round() const;

		template <typename FloatingType>
		BigFraction FromFloatingNumber(FloatingType value) const
		{
			BigFraction result;

			if (std::isnan(value))
			{
				result.sign = 1;
				result.numerator = 0;
				result.denominator = ONE;
				return result;
			}

			if (std::isinf(value))
			{
				result.sign = value < 0 ? -1 : 1;
				result.denominator = 0;
				result.numerator = ONE;
				return result;
			}

			result.sign = value < 0 ? -1 : 1;
			value = std::fabs(value);

			FloatingType integerPart;
			FloatingType fractionPart = std::modf(value, &integerPart);

			result.numerator = static_cast<BigInteger>(integerPart);
			result.denominator = ONE;

			switch (this->PrecisionMode)
			{
			case DecimalPrecisionMode::Fixed:
			{
				for (uint64_t round = this->FixedPrecisionCount; round > 0 && fractionPart != 0.0; --round)
				{
					fractionPart *= 10;
					FloatingType newIntPart;
					fractionPart = std::modf(fractionPart, &newIntPart);
					result.numerator = result.numerator * TEN + static_cast<BigInteger>(newIntPart);
					result.denominator *= TEN;
				}
				break;
			}

			case DecimalPrecisionMode::Full:
			{
				// Convert the precision to a floating-point number for comparison
				FloatingType floatingPrecision = static_cast<FloatingType>(GetFullPrecision());

				while (true)
				{
					fractionPart *= 10;
					FloatingType newIntPart;
					fractionPart = std::modf(fractionPart, &newIntPart);
					result.numerator = result.numerator * TEN + static_cast<BigInteger>(newIntPart);
					result.denominator *= TEN;

					// Convert current fractional difference to floating-point and compare
					if (std::fabs(fractionPart) < floatingPrecision)
					{
						break;
					}
				}
				break;
			}

			default:
				throw std::invalid_argument("Unknown PrecisionMode");
			}

			if (simplify_reduced)
			{
				result.ReduceSimplify();
			}

			return result;
		}

		long double BigIntegerToLongDouble(const BigInteger &big_integer) const;
		operator BigInteger() const;
		operator long double() const;
		operator double() const;
		operator float() const;

		// 生成由拉马努金提出的 π 的高精度表示。
		static BigFraction GenerateSrinivasaRamanujanPI();
		// 用于通过 Nilakantha 数列计算 π 的近似值，迭代次数由参数 iteration 指定。
		static BigFraction GenerateNilakanthaArrayPI(uint64_t iteration);

		/**
		 * @brief 计算 e ^ value 的高精度分数近似值。
		 *
		 * @param value 所需要计算的指数值（整数形式）。
		 * @param precision_mode 指定计算精度模式（定点或全精度）。默认值为 Full。
		 * @param fixed_precision_count 在定点模式下指定泰勒展开的项数。默认值为 2。
		 * @return 返回值类型为 BigFraction，表示 e ^ value 的高精度分数近似值。
		 */
		static BigFraction Exponential_Taylor(const BigInteger &value, DecimalPrecisionMode precision_mode = DecimalPrecisionMode::Full, uint64_t fixed_precision_count = 2);
		static BigFraction Exponential_Taylor(const BigFraction &value);
		static BigFraction Exponential(const BigFraction &x, int64_t precision_digits = 16);

		friend std::istream &operator>>(std::istream &is, BigFraction &fraction);
		friend std::ostream &operator<<(std::ostream &os, const BigFraction &fraction);

		friend BigFraction operator+(const BigInteger &left, const BigFraction &right);
		friend BigFraction operator-(const BigInteger &left, const BigFraction &right);
		friend BigFraction operator*(const BigInteger &left, const BigFraction &right);
		friend BigFraction operator/(const BigInteger &left, const BigFraction &right);

	private:
		BigInteger numerator;
		BigInteger denominator;
		int32_t sign;
		bool simplify_reduced = true;

		void ReduceSimplify();
		BigFraction LogarithmCF(const BigInteger &value) const;
		BigFraction LogarithmHalley(const BigInteger &a) const;
	};

	inline BigFraction BigFractionFullPrecision(1, 0);

} // namespace BIGFRAC

#endif
