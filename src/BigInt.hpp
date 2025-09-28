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

#ifndef LAMINA_BIG_INTEGER_HPP
#define LAMINA_BIG_INTEGER_HPP

#include <cmath>
#include <cstdint>
#include <climits>
#include <cstring>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <complex>
#include <optional>
#include <functional>
#include <type_traits>
#include <random>
#include <bitset>

namespace __LAMINA
{
	struct PrimeNumberTester;
}



namespace __LAMINA::BIGINT
{
	

	using digit_type = uint64_t;
	constexpr uint32_t DIGIT_BITS = std::numeric_limits<digit_type>::digits;
	constexpr uint32_t DIGIT_BYTES = DIGIT_BITS / CHAR_BIT;

	/**
	 * @brief 检查当前平台是否为小端字节序（Little Endian）。
	 *
	 * @return 若平台是小端字节序则返回 true，否则返回 false。
	 */
	inline bool is_little_endian()
	{
		const uint64_t value = 1;
		return *reinterpret_cast<const char*>(&value) == 1;
	}

	/**
	 * @brief 通用字节序转换，基于位运算实现，适用于所有整数类型（跨平台）。
	 *
	 * @tparam T 要执行字节序转换的值的类型。
	 * @param value 要进行字节序转换的值。
	 * @return 字节序转换后的值。
	 */
	template <typename T, typename std::enable_if<std::is_integral_v<T>>::type * = nullptr>
	constexpr T endian_swap(T value)
	{
		static_assert(!std::is_same<T, bool>::value, "bool is not supported");
		// For integer types of 8 bits or less, no byte order conversion is needed
		if (sizeof(T) == 1)
			return value;

		if constexpr (sizeof(T) == 2)  // 16-bit integers
		{
			return (((value & 0x00FFU) << 8) | ((value & 0xFF00U) >> 8));
		}
		else if constexpr (sizeof(T) == 4)	// 32-bit integers
		{
			constexpr uint32_t bit_mask = 0x00FF00FF;
			value = ((value & bit_mask) << 8) | ((value >> 8) & bit_mask);
			return (value << 16) | (value >> 16);
		}
		else if constexpr (sizeof(T) == 8)	// 64-bit integers
		{
			constexpr uint64_t bit_mask0 = 0x00FF00FF00FF00FF;
			constexpr uint64_t bit_mask1 = 0x0000FFFF0000FFFF;
			value = ((value & bit_mask0) << 8) | ((value >> 8) & bit_mask0);
			value = ((value & bit_mask1) << 16) | ((value >> 16) & bit_mask1);
			return (value << 32) | (value >> 32);
		}
		// Add corresponding bitwise operation logic for other sizes of integer types as needed
	}

	class BigInteger
	{
	private:
		friend struct __LAMINA::PrimeNumberTester;

		void Clean();

		std::vector<digit_type> values;

		static constexpr uint32_t byte_bits = std::numeric_limits<unsigned char>::digits;

		BigInteger(const digit_type* _value, size_t size);

	public:

		BigInteger();

		template <size_t N>
		BigInteger(const std::bitset<N>& bits)
		{
			size_t digitCount = (N + DIGIT_BITS - 1) / DIGIT_BITS;

			values.resize(digitCount, 0);

			for (size_t i = 0; i < N; ++i)
			{
				if (i >= DIGIT_BITS * values.size())
				{
					break;
				}

				if (bits.test(i))
				{
					SetBit(i);
				}
			}
		}

		BigInteger(uint64_t value);

		explicit BigInteger(const std::string& number_string);

		BigInteger(const std::string& number_string, uint32_t base);

		BigInteger(const BigInteger& other) noexcept;

		BigInteger(BigInteger&& other) noexcept;

		void Print(uint32_t base_value) const;

		void PrintBinary(bool have_space_with_block = true) const;

		BigInteger& operator=(const BigInteger& other) noexcept;

		BigInteger& operator=(BigInteger&& other) noexcept;

		BigInteger& AddMultiplyNumber(uint64_t add_num, uint64_t multiply_num);

		uint64_t DividModuloNumber(uint64_t divisor);

		BigInteger& operator+=(const BigInteger& other);

		/**
		 * @brief 将另一个 BigInteger 加到当前实例上。
		 *
		 * 此函数执行两个 BigInteger 的加法运算，并使用结果更新当前实例。
		 *
		 * @param other 要被加上的 BigInteger。
		 * @return 加法运算后经过修改的当前实例的引用。
		 */
		BigInteger &Add(const BigInteger &other);

		BigInteger& operator-=(const BigInteger& other);

		BigInteger Difference(const BigInteger& other) const;

		/**
		 * @brief 从当前实例中减去另一个 BigInteger。
		 *
		 * 此函数执行两个 BigInteger 的减法运算，并使用结果更新当前实例。
		 *
		 * @param other 要被减去的 BigInteger。
		 * @return 减法运算后经过修改的当前实例的引用。
		 */
		BigInteger &Subtract(const BigInteger &other);

		BigInteger operator++();

		BigInteger operator--();

		BigInteger operator++(int);

		BigInteger operator--(int);

		BigInteger RightShiftBlock(size_t d) const;

		BigInteger LeftShiftBlock(size_t d) const;

		/**
		 * 计算当前 BigInteger 实例的整数平方根（向下取整）。
		 *
		 * @return 返回当前对象的引用，其值已更新为原始值的整数平方根。
		 *
		 * @warning 此函数会修改当前 BigInteger 实例并返回其自身的引用。对该引用的任何后续修改都会影响原始对象。计算得到的平方根是小于或等于实际平方根的最大整数。
		 */
		BigInteger &Sqrt();

		/**
		 * 计算当前 BigInteger 实例的整数立方根。
		 *
		 * @return 返回当前对象的引用，其值已更新为原始值的整数立方根。
		 *
		 * @warning 此函数会修改当前 BigInteger 实例并返回其自身的引用。对该引用的任何后续修改都会影响原始对象。计算得到的立方根是小于或等于实际立方根的最大整数。
		 */
		BigInteger &Cbrt();

		/**
		 * 计算当前 BigInteger 实例的模幂运算。
		 * this = this^exponent mod modulo
		 *
		 * @param exponent 作为指数值的 BigInteger 对象。
		 * @param modulo 作为模数值的 BigInteger 对象。
		 * @return 返回当前对象的引用，其值已更新为模幂运算的结果。
		 *
		 * @warning 此函数会修改当前 BigInteger 实例并返回其自身的引用。对该引用的任何后续修改都会影响原始对象。此外，如果模值为 1，函数会将对象设为 0 并立即返回。
		 */
		BigInteger &PowerWithModulo(const BigInteger &exponent, const BigInteger &modulo);

		/**
		 * 对当前 BigInteger 实例与另一个 BigInteger 执行模乘法运算。
		 * 时间复杂度大致为 O ( m*log m * log n)，其中 m 是当前 BigInteger 的长度，n 是 other 的值
		 * this = this * other mod modulo
		 *
		 * @param other 要相乘的另一个 BigInteger 对象。
		 * @param modulo 作为模数值的 BigInteger 对象。
		 * @return 返回当前对象的引用，其值已更新为模乘法运算的结果。
		 *
		 * @warning 此函数会修改当前 BigInteger 实例并返回其自身的引用。对该引用的任何后续修改都会影响原始对象。如果模值为 1，函数会将对象设为 0 并立即返回。本函数使用二进制 exponentiation 算法，该算法对于大数运算效率较高。
		 */
		BigInteger &MultiplyWithModulo(const BigInteger &other, const BigInteger modulo);

		BigInteger ReciprocalNewtonIteration(size_t bit, size_t offset = 0) const;

		/**
		 * 使用牛顿迭代法对 BigInteger 执行除法运算。
		 *
		 * 此函数实现了基于牛顿迭代法的除法算法。
		 * 该方法通过逼近除数的倒数并迭代优化商，直到余数小于除数为止。
		 *
		 * @param divisor 表示除数的 BigInteger 对象。
		 * @return 一个包含除法运算后的商和余数的对。
		 */
		BigInteger DivideModuloNewtonIteration(const BigInteger &divisor, BigInteger &remainder) const;

		BigInteger DivideModuloDivideConquer(const BigInteger& divisor, BigInteger& remainder) const;

		BigInteger DivideModulo(const BigInteger& divisor, BigInteger& remainder) const;

		BigInteger& operator*=(const BigInteger& other);

		BigInteger& operator/=(const BigInteger& other);

		BigInteger& operator%=(const BigInteger& other);

		bool IsEven() const;

		bool IsPowerOfTwo() const;

		bool IsZero() const;

		bool IsNegative() const;

		size_t Size() const;

		size_t CountLeadingZeros() const;

		size_t CountTrailingZeros() const;

		size_t BitLength() const;

		// Block reference operator
		digit_type& operator[](size_t index)
		{
			// Ensure the index is within bounds
			if (index >= values.size())
			{
				throw std::out_of_range("Index out of range");
			}
			return values[index];
		}

		const digit_type& operator[](size_t index) const
		{
			// Ensure the index is within bounds
			if (index >= values.size())
			{
				throw std::out_of_range("Index out of range");
			}
			return values[index];
		}

		void SetBlock(size_t block_position, uint64_t value);

		digit_type GetBlock(size_t block_position) const;

		bool GetBit(size_t bit_position) const;

		void SetBit(size_t bit_position);

		void SetBit(bool value, size_t bit_position);

		std::byte GetByte(size_t byte_position) const;

		void SetByte(size_t byte_position, std::byte byte_in);

		// Remove 'count' number of bits from the most significant end of the integer.
		void SqueezeLeadingBits(size_t count);

		void FromUnsignedInt(uint64_t value);

		uint64_t ToUnsignedInt() const;

		BigInteger operator&(const BigInteger& other) const;

		BigInteger& operator&=(const BigInteger& other);

		BigInteger operator|(const BigInteger& other) const;

		BigInteger& operator|=(const BigInteger& other);

		BigInteger operator~() const;

		BigInteger operator^(const BigInteger& other) const;

		BigInteger& operator^=(const BigInteger& other);

		/**
		 * 计算当前 BigInteger 实例的幂。
		 * 结果 = result.Power(exponent)
		 *
		 * @param exponent 指数值，必须是一个非负整数。
		 * @return 返回当前对象的引用，其值已更新为计算结果。
		 *
		 * @warning 请注意，此函数返回的是当前对象的引用，而非副本。对返回的引用进行任何修改都会影响原始的 BigInteger 对象。
		 */
		BigInteger &Power(const size_t exponent);

		/**
		 * 计算当前 BigInteger 实例的幂，其中指数也是一个 BigInteger 对象。
		 * 结果 = result.BigPower(exponent)
		 *
		 * @param exponent 指数值，必须是一个正整数。
		 * @return 返回当前对象的引用，其值已更新为计算结果。
		 *
		 * @warning 请注意，此函数返回的是当前对象的引用，而非副本。对返回的引用进行任何修改都会影响原始的 BigInteger 对象。
		 */
		BigInteger &BigPower(const BigInteger &exponent);

		BigInteger ModuloBasePower(size_t n) const;

		static BigInteger BasePowerN(size_t n);

		static BigInteger TwoPowerN(size_t);

		BigInteger LeftShiftBit(size_t shift) const;

		BigInteger RightShiftBit(size_t shift) const;

		BigInteger& operator<<=(const uint32_t shift);

		BigInteger& operator>>=(const uint32_t shift);

		friend BigInteger operator<<(const BigInteger& lhs, const uint32_t shift)
		{
			BigInteger result(lhs);
			result = result.LeftShiftBit(shift);
			return result;
		}

		friend BigInteger operator>>(const BigInteger& lhs, const uint32_t shift)
		{
			BigInteger result(lhs);
			result = result.RightShiftBit(shift);
			return result;
		}

		/**
		 * @brief 对 BigInteger 的位进行指定位置的左旋转操作。
		 *
		 * @param bits 要进行旋转的 BigInteger 对象。
		 * @param shift 要将位向右旋转的位置数。
		 * @param reference_bit_capacity 旋转操作时参考的位容量（默认为 0）。
		 * @return 一个表示右旋转操作结果的新 BigInteger 对象。
		 *
		 * 该方法对给定的 BigInteger 对象执行左位旋转操作。旋转操作将位向左移动指定的位置数，
		 * 并将左侧溢出的位重新放置到右侧。
		 *
		 * 旋转操作根据参考位容量进行：
		 * - 如果参考位容量大于或等于输入的实际位长度，旋转操作是直接的，并且前导位会被压缩以适应参考位容量。
		 * - 如果参考位容量小于实际位长度，旋转操作将分为两部分：要旋转的部分和保持不变的部分。
		 *   旋转部分会被处理并与稳定部分组合形成最终结果。
		 *
		 * @example
		 * BigInteger num("10001", 2); // 二进制表示: 10001
		 * BigInteger result = BigInteger::BitRotateLeft(num, 2, 4); // 结果: 00110 (二进制)
		 */
		static BigInteger BitRotateLeft(const BigInteger& bits, size_t shift, size_t reference_bit_capacity);

		/**
		 * @brief 对 BigInteger 的位进行指定位置的右旋转操作。
		 *
		 * @param bits 要进行旋转的 BigInteger 对象。
		 * @param shift 要将位向右旋转的位置数。
		 * @param reference_bit_capacity 旋转操作时参考的位容量（默认为 0）。
		 * @return 一个表示右旋转操作结果的新 BigInteger 对象。
		 *
		 * 该方法对给定的 BigInteger 对象执行右位旋转操作。旋转操作将位向右移动指定的位置数，
		 * 并将右侧溢出的位重新放置到左侧。
		 *
		 * 旋转操作根据参考位容量进行：
		 * - 如果参考位容量大于或等于输入的实际位长度，旋转操作是直接的，并且前导位会被压缩以适应参考位容量。
		 * - 如果参考位容量小于实际位长度，旋转操作将分为两部分：要旋转的部分和保持不变的部分。
		 *   旋转部分会被处理并与稳定部分组合形成最终结果。
		 *
		 * @example
		 * BigInteger num("1001", 2); // 二进制表示: 1001
		 * BigInteger result = BigInteger::BitRotateRight(num, 2, 4); // 结果: 0110 (二进制)
		 */
		static BigInteger BitRotateRight(const BigInteger& bits, size_t shift, size_t reference_bit_capacity);

		explicit operator bool()
		{
			return !this->IsZero();
		}

		// Absolute value
		BigInteger Abs() const;

		/*
		* Reference: https://lemire.me/blog/2024/04/13/greatest-common-divisor-the-extended-euclidean-algorithm-and-speed/
		*/
		static BigInteger GCD(BigInteger a, BigInteger b);

		static BigInteger LCM(const BigInteger& a, const BigInteger& b);

		/**
		 * @brief Pollard's Rho 算法用于整数因式分解。
		 *
		 * 该函数应用 Pollard's Rho 算法来寻找给定整数 `n` 的非平凡因子。
		 * 如果找到因子，则返回该因子；如果没有找到非平凡因子，则返回 -1。
		 *
		 * @param n 要因式分解的整数，BigInteger 类型
		 * @return 找到的非平凡因子，如果没有找到则返回 BigInteger(-1)。
		 */
		static BigInteger PollardRho(const BigInteger& n);

		/**
		 * @brief 使用 Baby-Step Giant-Step 算法求解离散对数问题
		 *
		 * 该函数用于求解形如 a^x ≡ b (mod p) 的离散对数问题，
		 * 其中 a 和 b 是给定的 BigInteger 类型的大整数，p 是模数。
		 *
		 * @param base   底数 a，BigInteger 类型
		 * @param result 结果 b，BigInteger 类型
		 * @param mod    模数 p，BigInteger 类型
		 * @return BigInteger 满足 a^x ≡ b (mod p) 的 x 的值，
		 *                     如果不存在这样的 x，则返回 BigInteger(0)
		 */
		static BigInteger BabyStepGiantStep(const BigInteger& base, const BigInteger& result, const BigInteger& mod);

		static BigInteger RandomGenerateNBit(size_t n);

		static BigInteger Factorial(size_t n);

		friend void Swap(BigInteger& lhs, BigInteger& rhs)
		{
			lhs.values.swap(rhs.values);
		}

		/**
		* Calculates the base-2 logarithm (log2) of a BigInteger using binary search.
		*
		* This function iterates through the bits of the BigInteger from the most significant bit to the least
		* significant bit, checking if the bit is set (1). The position of the first set bit is the log2 of the number.
		*
		* @return The value of log2(n) as an integer, or -1 if n is 0 or 1.
		*/
		size_t Log2() const;

		/**
		* Convert the BigInteger to a binary string representation.
		*
		* @param reference_bit_capacity The desired bit capacity of the resulting binary string.
		* @param have_leading_zeros     Flag indicating whether to include leading zeros in the binary string.
		* @return                       Binary string representation of the BigInteger.
		*/
		std::string ToBinaryString(const uint32_t reference_bit_capacity, bool have_leading_zeros = true) const;

		/**
		* Convert the BigInteger to a string representation with the specified base.
		*
		* @param base_value The desired base for the string representation.
		* @warning Binary base is unsupported in ToString function.
		* @return String representation of the BigInteger in the specified base.
		*/
		std::string ToString(uint32_t base_value = 10) const;

		std::string to_string() const;


		BigInteger todecimal(const uint64_t* a, const size_t size, BigInteger& base_power, bool base_bool) const;

		void FromString(const std::string& number_string);

		void FromString(const std::string& number_string, uint32_t base_value);

		template <size_t N>
		void BitsetData(std::bitset<N>& bits) const
		{
			bool bit = false;
			for (size_t i = 0; i < N; i++)
			{
				if (i >= DIGIT_BITS * values.size())
				{
					break;
				}

				bool bit = GetBit(i);

				if (bit)
				{
					bits[i] = true;
				}
			}
		}

		/**
		 * @brief Convert the LittleEndian data in this->values to host byte order and export it to OutputData.
		 *
		 * @param OutputData The vector to store the exported data.
		 */
		void ExportData(std::vector<std::byte>& OutputData, size_t length = 0, bool is_big_endian = false)
		{
			size_t bit_length = BitLength();
			size_t bytes = (bit_length + CHAR_BIT - 1) / CHAR_BIT;
			if (length == 0)
			{
				length = bytes;
			}
			OutputData.clear();
			OutputData.resize(length);
			if (IsZero())
			{
				return;
			}
			size_t count = std::min(length, bytes);
			for (size_t i = 0; i < count; i++)
			{
				OutputData[i] = GetByte(i);
			}
			if (is_big_endian)
			{
				std::reverse(OutputData.begin(), OutputData.end());
			}
		}

		/**
		 * @brief Convert host byte order data to this->values of the LittleEndian data and import from InData.
		 *
		 * @param InData The vector containing the data to be imported.
		 */
		void ImportData(std::vector<std::byte> InData, bool is_big_endian = false)
		{
			if (InData.empty())
			{
				this->values.clear();
				return;
			}
			if (is_big_endian)
			{
				std::reverse(InData.begin(), InData.end());
			}
			size_t bytes = InData.size();
			size_t block_length = (bytes + DIGIT_BYTES - 1) / DIGIT_BYTES;
			this->values.resize(block_length);
			for (size_t i = 0; i < bytes; i++)
			{
				this->SetByte(i, InData[i]);
			}
			Clean();
		}

		/**
		 * @brief Convert the LittleEndian data in this->values to host byte order and export it to OutputData.
		 *
		 * @param OutputData The vector to store the exported data.
		 */
		void ExportData(std::vector<uint8_t>& OutputData, size_t length = 0, bool is_big_endian = false)
		{
			std::vector<std::byte> output_byte;
			ExportData(output_byte, length, is_big_endian);
			OutputData.resize(output_byte.size());
			for (size_t i = 0; i < output_byte.size(); i++)
			{
				OutputData[i] = uint8_t(output_byte[i]);
			}
		}

		/**
		 * @brief Convert host byte order data to this->values of the LittleEndian data and import from InData.
		 *
		 * @param InData The vector containing the data to be imported.
		 */
		void ImportData(std::vector<uint8_t> InData, bool is_big_endian = false)
		{
			std::vector<std::byte> input_byte(InData.size());
			for (size_t i = 0; i < input_byte.size(); i++)
			{
				input_byte[i] = std::byte{ InData[i] };
			}
			ImportData(input_byte, is_big_endian);
		}

		size_t SipHash(const BigInteger& Integer, std::vector<uint8_t>* keys = nullptr) const;

		friend class HashFunction;

		// 加法运算符重载
		BigInteger operator+(const BigInteger& other) const
		{
			BigInteger result(*this);
			result += other;
			return result;
		}

		// 减法运算符重载
		BigInteger operator-(const BigInteger& other) const
		{
			BigInteger result(*this);
			result -= other;
			return result;
		}

		// 乘法运算符重载
		BigInteger operator*(const BigInteger& other) const
		{
			BigInteger result(*this);
			result *= other;
			return result;
		}

		// 除法运算符重载
		BigInteger operator/(const BigInteger& other) const
		{
			BigInteger result(*this);
			result /= other;
			return result;
		}

		// 取模运算符重载
		BigInteger operator%(const BigInteger& other) const
		{
			BigInteger result(*this);
			result %= other;
			return result;
		}

		friend int compare(const BigInteger& a, const BigInteger& b)
		{
			const size_t len_a = a.values.size();
			const size_t len_b = b.values.size();

			if (len_a != len_b)
			{
				return len_a > len_b ? 1 : -1;
			}
			size_t i = len_a;
			while (i > 0)
			{
				i--;
				if (a.values[i] != b.values[i])
				{
					return a.values[i] > b.values[i] ? 1 : -1;
				}
			}
			return 0;
		}

		// 等于运算符重载
		bool operator==(const BigInteger& other) const
		{
			return compare(*this, other) == 0;
		}

		// 不等于运算符重载
		bool operator!=(const BigInteger& other) const
		{
			return !(*this == other);
		}

		// 大于运算符重载
		bool operator>(const BigInteger& other) const
		{
			return compare(*this, other) > 0;
		}

		// 大于等于运算符重载
		bool operator>=(const BigInteger& other) const
		{
			return compare(*this, other) >= 0;
		}

		// 小于运算符重载
		bool operator<(const BigInteger& other) const
		{
			return compare(*this, other) < 0;
		}

		// 小于等于运算符重载
		bool operator<=(const BigInteger& other) const
		{
			return compare(*this, other) <= 0;
		}
	};

	class BigSignedInteger
	{
	private:
		friend struct __LAMINA::PrimeNumberTester;

		BigInteger uint_data;
		bool	   sign = false;  //MSB is true, then the number is negative

	public:
		enum class ArithmeticMode : uint8_t
		{
			Addition = 0,
			Subtraction = 1,
			Multiplication = 2,
			Division = 4
		};

		BigSignedInteger();

		BigSignedInteger(int64_t n);

		BigSignedInteger(const BigSignedInteger& other) noexcept;

		BigSignedInteger(BigSignedInteger&& other) noexcept;

		BigSignedInteger(const BigInteger& other, bool new_sign = false);

		BigSignedInteger(BigInteger&& other, bool new_sign = false);

		explicit BigSignedInteger(const std::string& number_string);

		BigSignedInteger(const std::string& number_string, uint32_t base);

		operator BigInteger() const;

		BigSignedInteger Abs() const;

		bool IsZero() const;

		bool IsNegative() const;

		size_t Size() const;

		size_t BitLength() const;

		void Print(uint32_t base_value) const;

		void PrintBinary(bool have_space_with_block = true) const;

		/**
		* Convert the BigInteger to a binary string representation.
		*
		* @param reference_bit_capacity The desired bit capacity of the resulting binary string.
		* @param have_leading_zeros     Flag indicating whether to include leading zeros in the binary string.
		* @return                       Binary string representation of the BigInteger.
		*/
		std::string ToBinaryString(const uint32_t reference_bit_capacity, bool have_leading_zeros = true) const;

		/**
		* Convert the BigInteger to a string representation with the specified base.
		*
		* @param base_value The desired base for the string representation.
		* @warning Binary base is unsupported in ToString function.
		* @return String representation of the BigInteger in the specified base.
		*/
		std::string ToString(uint32_t base_value = 10) const;

		void FromString(const std::string& number_string, bool new_sign = false);

		void FromString(const std::string& number_string, uint32_t base_value, bool new_sign = false);

		void FromUnsignedInt(uint64_t value);

		uint64_t ToUnsignedInt() const;

		void FromSignedInt(int64_t value);

		int64_t ToSignedInt(bool is_negative) const;

		void ExportData(bool& is_negative, std::vector<uint8_t>& OutputData, size_t length = 0, bool is_big_endian = false)
		{
			is_negative = sign;
			uint_data.ExportData(OutputData, length, is_big_endian);
		}

		void ImportData(bool is_negative, std::vector<uint8_t> InData, bool is_big_endian = false)
		{
			sign = is_negative;
			uint_data.ImportData(InData, is_big_endian);
		}

		BigSignedInteger operator-() const;

		BigSignedInteger operator+() const;

		friend BigSignedInteger operator+(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend BigSignedInteger operator-(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend BigSignedInteger operator*(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend BigSignedInteger operator/(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend BigSignedInteger operator%(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		BigSignedInteger& operator=(const BigSignedInteger& other) noexcept;

		BigSignedInteger& operator=(BigSignedInteger&& other) noexcept;

		BigSignedInteger& operator+=(const BigSignedInteger& other);

		BigSignedInteger& operator-=(const BigSignedInteger& other);

		BigSignedInteger& operator*=(const BigSignedInteger& other);

		BigSignedInteger& operator/=(const BigSignedInteger& other);

		BigSignedInteger& operator%=(const BigSignedInteger& other);

		BigSignedInteger operator&(const BigSignedInteger& other) const;

		BigSignedInteger& operator&=(const BigSignedInteger& other);

		BigSignedInteger operator|(const BigSignedInteger& other) const;

		BigSignedInteger& operator|=(const BigSignedInteger& other);

		BigSignedInteger operator~() const;

		BigSignedInteger operator^(const BigSignedInteger& other) const;

		BigSignedInteger& operator^=(const BigSignedInteger& other);

		BigSignedInteger& operator<<=(size_t shift);

		BigSignedInteger& operator>>=(size_t shift);

		friend BigSignedInteger operator<<(const BigSignedInteger& lhs, const uint32_t shift)
		{
			BigSignedInteger result(lhs);
			result.uint_data = result.uint_data.LeftShiftBit(shift);
			if (result.uint_data.IsZero())
			{
				result.sign = false;
			}
			return result;
		}

		friend BigSignedInteger operator>>(const BigSignedInteger& lhs, const uint32_t shift)
		{
			BigSignedInteger result(lhs);
			result.uint_data = result.uint_data.RightShiftBit(shift);
			if (result.uint_data.IsZero())
			{
				result.sign = false;
			}
			return result;
		}

		explicit operator bool()
		{
			return !this->IsZero();
		}

		friend bool operator==(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend bool operator!=(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend bool operator>(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend bool operator>=(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend bool operator<(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		friend bool operator<=(const BigSignedInteger& lhs, const BigSignedInteger& rhs);

		static void EGCD(BigSignedInteger a, BigSignedInteger b, BigSignedInteger& gcd, BigSignedInteger& co1, BigSignedInteger& co2);

		/**
		 * @brief Performs modular arithmetic operations on two BigIntegers.
		 *
		 * This method supports modular addition, subtraction, multiplication, and division of
		 * two BigIntegers. The specific operation is determined by the `mode` parameter.
		 *
		 * @param mode The arithmetic mode specifying the operation to be performed
		 *             (Addition, Subtraction, Multiplication, or Division).
		 * @param a The first BigInteger operand.
		 * @param b The second BigInteger operand.
		 * @param modulus The modulus with respect to which the modular operation is performed.
		 * @return The result of the specified modular arithmetic operation (a op b) % modulus.
		 *
		 * @throws std::invalid_argument If the modulus is zero or, in the case of division,
		 *         the divisor is zero or the modulus is not prime.
		 * @throws std::invalid_argument If the modular inverse for division cannot be found,
		 *         ensuring (a * b) mod modulus != (a * inverse(b)) mod modulus.
		 */
		static BigSignedInteger ModuloArithmetic(ArithmeticMode mode, const BigSignedInteger& a, const BigSignedInteger& b, const BigSignedInteger& modulus);

		static BigSignedInteger ModuloInverse(const BigSignedInteger& a, const BigSignedInteger& b);

		size_t SipHash(const BigSignedInteger& Integer, std::vector<uint8_t>* keys = nullptr) const;
	};

	class HashFunction
	{
	public:
		// 使用 lambda 表达式来调用自定义的 hash 方法
		size_t operator()(const BigInteger& key) const
		{
			return key.SipHash(key);
		}

		// 使用 lambda 表达式来调用自定义的 hash 方法
		size_t operator()(const BigSignedInteger& key) const
		{
			return key.SipHash(key);
		}
	};

	class Montgomery
	{
	private:
		// Modulus
		BigInteger modulo;
		// Modulus * 2
		BigInteger modulo2;
		// Base (radix)
		BigInteger radix;
		BigInteger radix_square;
		// Negative inverse of the modulus
		BigInteger modulo_inverse_neg;
		size_t	   radix_blocks;

		BigInteger GetR() const;

		BigInteger ModuloR(const BigInteger& a) const;

		BigInteger DivisionR(const BigInteger& a) const;

		BigInteger ReduceLazy(const BigInteger& a) const;

		BigInteger Reduce(const BigInteger& a) const;

	public:
		Montgomery(const BigInteger& new_modulo);

		BigInteger ToInt(const BigInteger& a) const;

		BigInteger ToMontgomery(const BigInteger& a) const;

		BigInteger Addtion(const BigInteger& a, const BigInteger& b) const;

		BigSignedInteger Addtion(const BigSignedInteger& a, const BigSignedInteger& b) const;

		BigInteger Substraction(const BigInteger& a, const BigInteger& b) const;

		BigSignedInteger Substraction(const BigSignedInteger& a, const BigSignedInteger& b) const;

		BigInteger Multiplication(const BigInteger& a, const BigInteger& b) const;

		BigSignedInteger Multiplication(const BigSignedInteger& a, const BigSignedInteger& b) const;

		BigInteger Inverse(const BigInteger& a) const;

		BigSignedInteger Inverse(const BigSignedInteger& a) const;

		BigInteger Power(BigInteger base, const BigInteger& index) const;

		BigSignedInteger Power(BigSignedInteger base, const BigInteger& index) const;
	};

	//If the number is greater than or equal to 1, then this algorithm can be used to calculate the kth root of a BigInteger.
	class ShiftingKthRoot
	{
	public:
		ShiftingKthRoot(const uint64_t k)
			: k_(k)
		{
		}

		BigInteger operator()(const BigInteger& n) const
		{
			return ComputeRoot(n);
		}

		void SetKthRoot(const uint64_t k)
		{
			if (0 == k)
			{
				throw std::invalid_argument("k cannot be zero.");
			}

			this->k_ = k;
		}

	private:
		uint64_t k_ = 1; // kth of root

		BigInteger BinarySearch(const BigInteger& low, const BigInteger& high, const std::function<bool(const BigInteger&)>& condition) const
		{
			BigInteger left = low;
			BigInteger right = high;
			while (left < right)
			{
				BigInteger mid = (left + right) / 2;
				if (condition(mid))
				{
					left = mid + 1;
				}
				else
				{
					right = mid;
				}
			}
			return left;
		}

		BigInteger ComputeRoot(const BigInteger& n) const
		{
			// Number of bits per block
			uint64_t blockBits = 64;

			// Initialize remainder and current root estimate
			BigInteger remainder = 0;
			BigInteger rootEstimate = 0;

			// B represents 2^blockBits (1 << blockBits)
			BigInteger B = BigInteger(1) << blockBits;

			// Bk_bits represents the number of bits needed for k blocks
			uint64_t totalBits = blockBits * k_;

			// Bk_mask is a mask to extract totalBits from n
			BigInteger bitMask = (BigInteger(1) << totalBits) - 1;

			// Calculate the number of iterations required, using uint64_t to prevent overflow
			uint64_t numIterations = (n.BitLength() + totalBits - 1) / totalBits;

			// Loop over each block of bits in reverse order
			for (uint64_t i = numIterations; i > 0; --i)
			{
				// Adjust the index to match the correct bit position
				uint64_t index = i - 1;

				// Extract the current block of bits from n
				BigInteger alpha = (n >> (index * totalBits)) & bitMask;

				// Shift the current root estimate by blockBits
				BigInteger shiftedRoot = rootEstimate << blockBits;

				// Calculate the current power of the root estimate
				BigInteger currentPower = (rootEstimate.Power(k_)) << totalBits;

				// Add alpha to the remainder shifted by totalBits
				BigInteger shiftedRemainder = (remainder << totalBits) + alpha;

				// Calculate the sum of the current power and the shifted remainder
				BigInteger powerPlusRemainder = currentPower + shiftedRemainder;

				// Define the condition for binary search
				auto condition = [&](const BigInteger& beta)
					{
						return (shiftedRoot + beta).Power(k_) <= powerPlusRemainder;
					};

				// Perform binary search to find the appropriate beta
				BigInteger beta = BinarySearch(BigInteger(1), B, condition) - 1;

				// Update the current root estimate with beta
				rootEstimate = shiftedRoot + beta;

				// Update the remainder
				remainder = shiftedRemainder - ((shiftedRoot + beta).Power(k_) - currentPower);
			}

			// Return the final root estimate
			return rootEstimate;
		}
	};

	namespace Factorial
	{

		using BigInteger = __LAMINA::BIGINT::BigInteger;

		// Compute the prime factorization table for (n!)
		inline std::vector<std::pair<size_t, size_t>> get_prime_factors(size_t n)
		{
			std::vector<std::pair<size_t, size_t>> factors;
			std::vector<uint8_t> is_prime(n + 1, true);

			for (size_t i = 2; i <= n; ++i)
			{
				if (is_prime[i])
				{
					size_t count = 0;
					for (size_t j = i; j <= n; j += i)
					{
						is_prime[j] = false;
					}
					for (size_t j = i; j <= n; j *= i)
					{
						count += n / j;
					}
					factors.emplace_back(i, count);
				}
			}
			return factors;
		}

		// Recursively calculate the product of odd powers
		BigInteger calculate_odd_product(std::vector<std::pair<size_t, size_t>>::iterator beg, std::vector<std::pair<size_t, size_t>>::iterator end)
		{
			if (beg == end)
				return 1;
			if (beg + 1 == end)
			{
				if (beg->second % 2 == 1)
				{
					beg->second /= 2;
					return BigInteger(beg->first);
				}
				else
				{
					beg->second /= 2;
					return 1;
				}
			}

			auto mid = beg + (end - beg) / 2;
			return calculate_odd_product(beg, mid) * calculate_odd_product(mid, end);
		}

		// Recursively calculate the value of (n!)
		BigInteger calculate_factorial(std::vector<std::pair<size_t, size_t>> &factors)
		{
			BigInteger result = 1;
			if (!factors.empty())
			{
				result = calculate_odd_product(factors.begin(), factors.end());
				while (!factors.empty() && factors.back().second == 0)
				{
					factors.pop_back();
				}
				BigInteger sub_result = calculate_factorial(factors);
				result *= sub_result * sub_result;
			}
			return result;
		}
	} // namespace Factorial

}  // namespace __LAMINA::BIGINT

#endif

