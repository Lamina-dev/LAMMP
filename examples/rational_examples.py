#!/usr/bin/env python3
"""
有理数使用示例 (Rational Usage Examples)
Demonstrates the capabilities of the Rational class.
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from lamina_bignum import Rational, BigInteger

def demonstrate_basic_operations():
    """演示基本运算 (Demonstrate basic operations)"""
    print("=== 基本运算示例 (Basic Operations Examples) ===\n")
    
    # 创建有理数的不同方式
    a = Rational(3, 4)          # 分数形式
    b = Rational("22/7")        # 字符串形式
    c = Rational(0.5)           # 从浮点数
    d = Rational(-5, 6)         # 负数
    
    print(f"a = {a} (from integers)")
    print(f"b = {b} (from string)")
    print(f"c = {c} (from float)")
    print(f"d = {d} (negative)")
    print()
    
    # 基本运算
    print("Basic arithmetic operations (基本算术运算):")
    print(f"{a} + {b} = {a + b}")
    print(f"{a} - {b} = {a - b}")
    print(f"{a} * {b} = {a * b}")
    print(f"{a} / {b} = {a / b}")
    print(f"{a} ^ 2 = {a ** 2}")
    print()

def demonstrate_mixed_operations():
    """演示与其他类型的混合运算 (Demonstrate mixed type operations)"""
    print("=== 混合运算示例 (Mixed Operations Examples) ===\n")
    
    r = Rational(3, 4)
    integer = 2
    big_int = BigInteger(5)
    float_val = 1.5
    
    print(f"r = {r}")
    print(f"integer = {integer}")
    print(f"big_int = {big_int}")
    print(f"float_val = {float_val}")
    print()
    
    print("Mixed operations (混合运算):")
    print(f"{r} + {integer} = {r + integer}")
    print(f"{r} * {big_int} = {r * big_int}")
    print(f"{r} + {float_val} = {r + float_val}")
    print(f"{integer} - {r} = {integer - r}")
    print()

def demonstrate_rational_properties():
    """演示有理数属性和方法 (Demonstrate rational properties and methods)"""
    print("=== 有理数属性和方法示例 (Rational Properties and Methods) ===\n")
    
    # 自动化简
    r1 = Rational(6, 8)  # 应该化简为 3/4
    print(f"Automatic reduction (自动化简): 6/8 = {r1}")
    
    r2 = Rational(22, 7)
    print(f"Numerator of {r2}: {r2.numerator}")
    print(f"Denominator of {r2}: {r2.denominator}")
    
    # 带分数
    mixed = r2.to_mixed_number()
    print(f"Mixed number (带分数): {r2} = {mixed[0]} and {mixed[1]}")
    
    # 检查是否为整数
    r3 = Rational(10, 5)  # = 2
    print(f"{r3} is integer: {r3.is_integer()}")
    print(f"{r2} is integer: {r2.is_integer()}")
    
    # 限制分母
    pi_approx = Rational(22, 7)
    limited = pi_approx.limit_denominator(10)
    print(f"Limit denominator (限制分母): {pi_approx} -> {limited}")
    print()

def demonstrate_comparisons():
    """演示比较运算 (Demonstrate comparison operations)"""
    print("=== 比较运算示例 (Comparison Operations Examples) ===\n")
    
    rationals = [
        Rational(1, 2),
        Rational(3, 4),
        Rational(2, 3),
        Rational(5, 6),
        Rational(1, 1)
    ]
    
    print("Sorting rationals (有理数排序):")
    for r in rationals:
        print(f"  {r} = {float(r):.6f}")
    
    sorted_rationals = sorted(rationals)
    print("\nAfter sorting (排序后):")
    for r in sorted_rationals:
        print(f"  {r} = {float(r):.6f}")
    print()

def demonstrate_precise_calculations():
    """演示精确计算 (Demonstrate precise calculations)"""
    print("=== 精确计算示例 (Precise Calculations Examples) ===\n")
    
    # 展示浮点数精度问题与有理数精确性的对比
    print("Float precision vs Rational precision (浮点精度 vs 有理数精度):")
    
    # 浮点数计算
    float_result = 0.1 + 0.2
    print(f"Float: 0.1 + 0.2 = {float_result}")
    print(f"Is equal to 0.3? {float_result == 0.3}")
    
    # 有理数计算
    r1 = Rational(1, 10)
    r2 = Rational(2, 10)  # 或 Rational(1, 5)
    rational_result = r1 + r2
    print(f"Rational: {r1} + {r2} = {rational_result}")
    print(f"Is equal to 3/10? {rational_result == Rational(3, 10)}")
    print()
    
    # 复杂分数运算
    print("Complex fraction operations (复杂分数运算):")
    a = Rational(1, 3)
    b = Rational(1, 7)
    c = Rational(1, 11)
    
    result = a + b + c
    print(f"{a} + {b} + {c} = {result}")
    print(f"As decimal: {float(result):.10f}")
    print()
    
    # 连分数近似
    print("Continued fraction approximation (连分数近似):")
    pi_approx = Rational(355, 113)  # 很好的π近似
    print(f"π approximation: {pi_approx} = {float(pi_approx):.10f}")
    print(f"Actual π: {3.141592653589793:.10f}")
    print(f"Error: {abs(float(pi_approx) - 3.141592653589793):.2e}")
    print()

if __name__ == "__main__":
    print("Lamina BigNum - 有理数示例程序")
    print("Lamina BigNum - Rational Example Program")
    print("=" * 50)
    print()
    
    demonstrate_basic_operations()
    demonstrate_mixed_operations()
    demonstrate_rational_properties()
    demonstrate_comparisons()
    demonstrate_precise_calculations()
    
    print("示例程序完成 (Example program completed)")