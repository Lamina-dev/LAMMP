#!/usr/bin/env python3
"""
大整数使用示例 (BigInteger Usage Examples)
Demonstrates the capabilities of the BigInteger class.
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from lamina_bignum import BigInteger

def demonstrate_basic_operations():
    """演示基本运算 (Demonstrate basic operations)"""
    print("=== 基本运算示例 (Basic Operations Examples) ===\n")
    
    # 创建大整数
    a = BigInteger("123456789012345678901234567890")
    b = BigInteger(987654321)
    
    print(f"a = {a}")
    print(f"b = {b}")
    print()
    
    # 加法
    result = a + b
    print(f"Addition (加法): {a} + {b} = {result}")
    
    # 减法
    result = a - b
    print(f"Subtraction (减法): {a} - {b} = {result}")
    
    # 乘法
    result = a * b
    print(f"Multiplication (乘法): {a} * {b} = {result}")
    
    # 除法
    result = a // b
    print(f"Floor Division (整除): {a} // {b} = {result}")
    
    # 取模
    result = a % b
    print(f"Modulo (取模): {a} % {b} = {result}")
    
    # 幂运算
    small = BigInteger(2)
    power = BigInteger(100)
    result = small ** power
    print(f"Power (幂运算): {small} ** {power} = {result}")
    print()

def demonstrate_mathematical_functions():
    """演示数学函数 (Demonstrate mathematical functions)"""
    print("=== 数学函数示例 (Mathematical Functions Examples) ===\n")
    
    # 最大公约数
    a = BigInteger(48)
    b = BigInteger(18)
    gcd_result = a.gcd(b)
    print(f"GCD (最大公约数): gcd({a}, {b}) = {gcd_result}")
    
    # 最小公倍数
    lcm_result = a.lcm(b)
    print(f"LCM (最小公倍数): lcm({a}, {b}) = {lcm_result}")
    
    # 阶乘
    n = BigInteger(20)
    factorial_result = n.factorial()
    print(f"Factorial (阶乘): {n}! = {factorial_result}")
    
    # 素数检测
    primes_to_test = [BigInteger(x) for x in [17, 25, 97, 100]]
    for num in primes_to_test:
        is_prime = num.is_prime()
        print(f"Prime test (素数检测): {num} is prime: {is_prime}")
    
    print()

def demonstrate_large_numbers():
    """演示超大数运算 (Demonstrate very large number operations)"""
    print("=== 超大数运算示例 (Very Large Number Operations) ===\n")
    
    # 计算非常大的阶乘
    print("Computing 100! (100的阶乘):")
    big_factorial = BigInteger(100).factorial()
    print(f"100! = {big_factorial}")
    print(f"Number of digits (位数): {len(str(big_factorial))}")
    print()
    
    # 大数的幂运算
    base = BigInteger(7)
    exponent = 50
    result = base ** exponent
    print(f"Large power (大数幂运算): {base}^{exponent} = {result}")
    print(f"Number of digits (位数): {len(str(result))}")
    print()
    
    # Fibonacci数列的大数
    def fibonacci(n):
        if n <= 1:
            return BigInteger(n)
        a, b = BigInteger(0), BigInteger(1)
        for _ in range(2, n + 1):
            a, b = b, a + b
        return b
    
    fib_100 = fibonacci(100)
    print(f"Fibonacci(100) = {fib_100}")
    print(f"Number of digits (位数): {len(str(fib_100))}")
    print()

if __name__ == "__main__":
    print("Lamina BigNum - 大整数示例程序")
    print("Lamina BigNum - BigInteger Example Program")
    print("=" * 50)
    print()
    
    demonstrate_basic_operations()
    demonstrate_mathematical_functions()
    demonstrate_large_numbers()
    
    print("示例程序完成 (Example program completed)")