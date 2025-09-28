#!/usr/bin/env python3
"""
Lamina BigNum 库演示程序 (Library Demo Program)
Complete demonstration of all implemented features.
"""

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from lamina_bignum import BigInteger, Rational

def print_section(title):
    """Print a formatted section header"""
    print(f"\n{'='*60}")
    print(f" {title}")
    print(f"{'='*60}")

def demo_biginteger():
    """Demonstrate BigInteger capabilities"""
    print_section("大整数演示 (BigInteger Demo)")
    
    # Very large numbers
    large_num = BigInteger("98765432109876543210987654321098765432109876543210")
    print(f"Large number: {large_num}")
    print(f"Number of digits: {len(str(large_num))}")
    
    # Mathematical operations
    a = BigInteger(123456)
    b = BigInteger(789012)
    
    print(f"\nMathematical operations:")
    print(f"{a} + {b} = {a + b}")
    print(f"{a} * {b} = {a * b}")
    print(f"{a}^3 = {a**3}")
    
    # Mathematical functions
    print(f"\nMathematical functions:")
    print(f"gcd({a}, {b}) = {a.gcd(b)}")
    print(f"lcm({a}, {b}) = {a.lcm(b)}")
    print(f"25! = {BigInteger(25).factorial()}")
    
    # Prime testing
    print(f"\nPrime testing:")
    primes = [97, 98, 99, 100, 101]
    for p in primes:
        is_prime = BigInteger(p).is_prime()
        print(f"{p} is prime: {is_prime}")

def demo_rational():
    """Demonstrate Rational capabilities"""
    print_section("有理数演示 (Rational Demo)")
    
    # Creating rationals
    r1 = Rational(22, 7)        # π approximation
    r2 = Rational("355/113")    # Better π approximation
    r3 = Rational(0.125)        # From decimal
    
    print(f"π approximations:")
    print(f"22/7 = {r1} = {float(r1):.10f}")
    print(f"355/113 = {r2} = {float(r2):.10f}")
    print(f"Actual π ≈ {3.141592653589793:.10f}")
    
    # Arithmetic precision
    print(f"\nPrecision comparison:")
    # Float calculation
    float_calc = 0.1 + 0.2 + 0.3 + 0.4
    print(f"Float: 0.1 + 0.2 + 0.3 + 0.4 = {float_calc}")
    
    # Rational calculation
    rational_calc = Rational(1,10) + Rational(2,10) + Rational(3,10) + Rational(4,10)
    print(f"Rational: 1/10 + 2/10 + 3/10 + 4/10 = {rational_calc} = {float(rational_calc)}")
    
    # Complex fractions
    print(f"\nComplex fraction operations:")
    result = Rational(1,2) + Rational(1,3) + Rational(1,4) + Rational(1,5) + Rational(1,6)
    print(f"1/2 + 1/3 + 1/4 + 1/5 + 1/6 = {result}")
    print(f"As decimal: {float(result):.10f}")
    
    # Mixed numbers
    improper = Rational(17, 5)
    whole, frac = improper.to_mixed_number()
    print(f"\nMixed number: {improper} = {whole} and {frac}")

def demo_interoperability():
    """Demonstrate interoperability between types"""
    print_section("类型互操作演示 (Type Interoperability Demo)")
    
    # BigInteger with Rational
    big_int = BigInteger("12345678901234567890")
    rational = Rational(22, 7)
    
    print(f"BigInteger: {big_int}")
    print(f"Rational: {rational}")
    
    # Convert BigInteger to Rational
    big_rational = Rational(big_int, 1)
    print(f"BigInteger as Rational: {big_rational}")
    
    # Mixed operations
    result1 = rational + BigInteger(3)
    result2 = rational * BigInteger(2)
    
    print(f"{rational} + 3 = {result1}")
    print(f"{rational} × 2 = {result2}")

def demo_real_world_examples():
    """Show real-world calculation examples"""
    print_section("实际应用示例 (Real-world Examples)")
    
    # Financial calculation
    print("Financial calculation (金融计算):")
    principal = Rational(10000)  # $10,000
    rate = Rational(5, 100)      # 5% annual rate
    years = 10
    
    # Compound interest: A = P(1 + r)^t
    amount = principal * (Rational(1) + rate) ** years
    print(f"Investment: ${principal}")
    print(f"Rate: {rate} = {float(rate)*100}%")
    print(f"After {years} years: ${amount} = ${float(amount):.2f}")
    
    # Scientific calculation
    print(f"\nScientific calculation (科学计算):")
    # Calculate e using series expansion: e ≈ Σ(1/n!) for n=0 to large number
    e_approx = Rational(0)
    for n in range(20):  # First 20 terms
        factorial_n = BigInteger(n).factorial()
        term = Rational(1, factorial_n)
        e_approx += term
    
    print(f"e approximation (using 20 terms): {e_approx}")
    print(f"As decimal: {float(e_approx):.15f}")
    print(f"Actual e ≈ {2.718281828459045:.15f}")
    
    # Harmonic series
    print(f"\nHarmonic series (调和级数):")
    harmonic_sum = Rational(0)
    n_terms = 100
    for i in range(1, n_terms + 1):
        harmonic_sum += Rational(1, i)
    
    print(f"H_{n_terms} = Σ(1/n) for n=1 to {n_terms} = {harmonic_sum}")
    print(f"As decimal: {float(harmonic_sum):.10f}")

def demo_performance():
    """Demonstrate performance with large numbers"""
    print_section("性能演示 (Performance Demo)")
    
    import time
    
    # Large factorial
    print("Computing large factorials:")
    for n in [100, 500, 1000]:
        start = time.time()
        result = BigInteger(n).factorial()
        end = time.time()
        print(f"{n}! computed in {end-start:.4f} seconds")
        print(f"  Result has {len(str(result))} digits")
    
    # Large Fibonacci
    print(f"\nLarge Fibonacci numbers:")
    def fibonacci_rational(n):
        """Calculate Fibonacci using rational numbers (exact)"""
        if n <= 1:
            return Rational(n)
        a, b = Rational(0), Rational(1)
        for _ in range(2, n + 1):
            a, b = b, a + b
        return b
    
    for n in [50, 100, 200]:
        start = time.time()
        fib_n = fibonacci_rational(n)
        end = time.time()
        print(f"F({n}) = {fib_n} (computed in {end-start:.4f} seconds)")

def main():
    """Main demo function"""
    print("Lamina BigNum - 完整功能演示")
    print("Lamina BigNum - Complete Feature Demonstration")
    print(f"Library Version: 0.1.0")
    
    demo_biginteger()
    demo_rational()
    demo_interoperability()
    demo_real_world_examples()
    demo_performance()
    
    print_section("演示完成 (Demo Complete)")
    print("Thank you for using Lamina BigNum!")
    print("感谢使用 Lamina BigNum!")
    
    # Show future features
    print(f"\n🔮 Future Features (未来功能):")
    print(f"  • ArbitraryFloat - 任意精度浮点数")
    print(f"  • FixedPoint - 任意精度定点数")
    print(f"  Stay tuned for updates! 敬请期待更新！")

if __name__ == "__main__":
    main()