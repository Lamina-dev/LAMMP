# Lamina_BigNum
Lamina的任意精度计算库 (Lamina's Arbitrary Precision Calculation Library)

## 功能特性 (Features)

- **BigInteger** - 大整数 (Arbitrary precision integers)
  - 支持所有基本算术运算 (Supports all basic arithmetic operations)
  - 数学函数：最大公约数、最小公倍数、素数检测、阶乘 (Mathematical functions: GCD, LCM, prime test, factorial)
  
- **Rational** - 有理数 (Rational numbers)
  - 自动化简为最简分数 (Automatically reduces to lowest terms)
  - 支持与整数、浮点数的混合运算 (Supports mixed operations with integers and floats)
  - 带分数转换 (Mixed number conversion)

- **计划中的功能 (Planned Features)**
  - 任意精度浮点数 (Arbitrary precision floating point numbers)
  - 任意精度定点数 (Arbitrary precision fixed point numbers)

## 安装 (Installation)

```bash
pip install -e .
```

## 使用示例 (Usage Examples)

### 大整数 (BigInteger)

```python
from lamina_bignum import BigInteger

# 创建大整数
a = BigInteger("123456789012345678901234567890")
b = BigInteger(987654321)

# 基本运算
result = a + b
print(f"{a} + {b} = {result}")

# 数学函数
factorial_10 = BigInteger(10).factorial()
print(f"10! = {factorial_10}")

# 素数检测
prime_test = BigInteger(17).is_prime()
print(f"17 is prime: {prime_test}")
```

### 有理数 (Rational)

```python
from lamina_bignum import Rational

# 创建有理数
a = Rational(3, 4)        # 3/4
b = Rational("22/7")      # 从字符串创建
c = Rational(0.5)         # 从浮点数创建

# 运算
result = a + b
print(f"{a} + {b} = {result}")

# 带分数
mixed = Rational(22, 7).to_mixed_number()
print(f"22/7 as mixed number: {mixed[0]} and {mixed[1]}")
```

## 许可证 (License)

本项目使用 GNU Lesser General Public License v2.1 许可证。详见 [LICENSE](LICENSE) 文件。
