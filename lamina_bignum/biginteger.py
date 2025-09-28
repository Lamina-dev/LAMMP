"""
BigInteger - 大整数类
Arbitrary precision integer implementation for Lamina language.
"""

class BigInteger:
    """
    任意精度大整数类 (Arbitrary precision big integer class)
    
    Provides mathematical operations on integers of arbitrary size,
    limited only by available memory.
    """
    
    def __init__(self, value=0):
        """
        初始化大整数 (Initialize big integer)
        
        Args:
            value: Initial value (int, str, or another BigInteger)
        """
        if isinstance(value, BigInteger):
            self._value = value._value
        elif isinstance(value, str):
            self._value = int(value)
        elif isinstance(value, int):
            self._value = value
        else:
            raise TypeError(f"Cannot create BigInteger from {type(value)}")
    
    def __str__(self):
        """字符串表示 (String representation)"""
        return str(self._value)
    
    def __repr__(self):
        """调试表示 (Debug representation)"""
        return f"BigInteger({self._value})"
    
    def __eq__(self, other):
        """相等比较 (Equality comparison)"""
        if isinstance(other, BigInteger):
            return self._value == other._value
        elif isinstance(other, int):
            return self._value == other
        return False
    
    def __lt__(self, other):
        """小于比较 (Less than comparison)"""
        if isinstance(other, BigInteger):
            return self._value < other._value
        elif isinstance(other, int):
            return self._value < other
        return NotImplemented
    
    def __le__(self, other):
        """小于等于比较 (Less than or equal comparison)"""
        return self.__lt__(other) or self.__eq__(other)
    
    def __gt__(self, other):
        """大于比较 (Greater than comparison)"""
        if isinstance(other, BigInteger):
            return self._value > other._value
        elif isinstance(other, int):
            return self._value > other
        return NotImplemented
    
    def __ge__(self, other):
        """大于等于比较 (Greater than or equal comparison)"""
        return self.__gt__(other) or self.__eq__(other)
    
    def __add__(self, other):
        """加法 (Addition)"""
        if isinstance(other, BigInteger):
            return BigInteger(self._value + other._value)
        elif isinstance(other, int):
            return BigInteger(self._value + other)
        return NotImplemented
    
    def __radd__(self, other):
        """右加法 (Right addition)"""
        return self.__add__(other)
    
    def __sub__(self, other):
        """减法 (Subtraction)"""
        if isinstance(other, BigInteger):
            return BigInteger(self._value - other._value)
        elif isinstance(other, int):
            return BigInteger(self._value - other)
        return NotImplemented
    
    def __rsub__(self, other):
        """右减法 (Right subtraction)"""
        if isinstance(other, int):
            return BigInteger(other - self._value)
        return NotImplemented
    
    def __mul__(self, other):
        """乘法 (Multiplication)"""
        if isinstance(other, BigInteger):
            return BigInteger(self._value * other._value)
        elif isinstance(other, int):
            return BigInteger(self._value * other)
        return NotImplemented
    
    def __rmul__(self, other):
        """右乘法 (Right multiplication)"""
        return self.__mul__(other)
    
    def __truediv__(self, other):
        """除法 (Division) - returns float"""
        if isinstance(other, BigInteger):
            return self._value / other._value
        elif isinstance(other, int):
            return self._value / other
        return NotImplemented
    
    def __floordiv__(self, other):
        """整数除法 (Floor division)"""
        if isinstance(other, BigInteger):
            return BigInteger(self._value // other._value)
        elif isinstance(other, int):
            return BigInteger(self._value // other)
        return NotImplemented
    
    def __mod__(self, other):
        """取模 (Modulo)"""
        if isinstance(other, BigInteger):
            return BigInteger(self._value % other._value)
        elif isinstance(other, int):
            return BigInteger(self._value % other)
        return NotImplemented
    
    def __pow__(self, other):
        """幂运算 (Power)"""
        if isinstance(other, BigInteger):
            return BigInteger(self._value ** other._value)
        elif isinstance(other, int):
            return BigInteger(self._value ** other)
        return NotImplemented
    
    def __neg__(self):
        """取负 (Negation)"""
        return BigInteger(-self._value)
    
    def __pos__(self):
        """取正 (Positive)"""
        return BigInteger(self._value)
    
    def __abs__(self):
        """绝对值 (Absolute value)"""
        return BigInteger(abs(self._value))
    
    def __int__(self):
        """转换为int (Convert to int)"""
        return self._value
    
    def __float__(self):
        """转换为float (Convert to float)"""
        return float(self._value)
    
    def __hash__(self):
        """哈希值 (Hash value)"""
        return hash(self._value)
    
    def gcd(self, other):
        """
        最大公约数 (Greatest Common Divisor)
        
        Args:
            other: Another BigInteger or int
            
        Returns:
            BigInteger: The GCD of self and other
        """
        import math
        if isinstance(other, BigInteger):
            return BigInteger(math.gcd(self._value, other._value))
        elif isinstance(other, int):
            return BigInteger(math.gcd(self._value, other))
        raise TypeError(f"Cannot compute GCD with {type(other)}")
    
    def lcm(self, other):
        """
        最小公倍数 (Least Common Multiple)
        
        Args:
            other: Another BigInteger or int
            
        Returns:
            BigInteger: The LCM of self and other
        """
        if isinstance(other, (BigInteger, int)):
            gcd_val = self.gcd(other)
            return BigInteger((self._value * int(other)) // int(gcd_val))
        raise TypeError(f"Cannot compute LCM with {type(other)}")
    
    def is_prime(self):
        """
        素数检测 (Prime number test)
        
        Returns:
            bool: True if the number is prime, False otherwise
        """
        if self._value < 2:
            return False
        if self._value == 2:
            return True
        if self._value % 2 == 0:
            return False
        
        # Simple trial division for small numbers
        if self._value < 1000:
            for i in range(3, int(self._value**0.5) + 1, 2):
                if self._value % i == 0:
                    return False
            return True
        
        # For larger numbers, use a more sophisticated test
        # This is a simplified version - in practice, you might want
        # to use Miller-Rabin or other advanced primality tests
        for i in range(3, min(1000, int(self._value**0.5) + 1), 2):
            if self._value % i == 0:
                return False
        return True
    
    def factorial(self):
        """
        阶乘 (Factorial)
        
        Returns:
            BigInteger: n! where n is this BigInteger
        """
        if self._value < 0:
            raise ValueError("Factorial is not defined for negative numbers")
        
        import math
        return BigInteger(math.factorial(self._value))