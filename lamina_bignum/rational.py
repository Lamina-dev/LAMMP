"""
Rational - 有理数类
Rational number implementation for Lamina language.
"""

from .biginteger import BigInteger


class Rational:
    """
    有理数类 (Rational number class)
    
    Represents rational numbers as a ratio of two BigIntegers,
    automatically reducing to lowest terms.
    """
    
    def __init__(self, numerator=0, denominator=1):
        """
        初始化有理数 (Initialize rational number)
        
        Args:
            numerator: The numerator (BigInteger, int, or str)
            denominator: The denominator (BigInteger, int, or str, must be non-zero)
        """
        if isinstance(numerator, str) and '/' in numerator:
            # Parse fraction string like "3/4"
            parts = numerator.split('/')
            if len(parts) != 2:
                raise ValueError("Invalid fraction format")
            numerator = BigInteger(parts[0])
            denominator = BigInteger(parts[1])
        elif isinstance(numerator, float):
            # Convert float to rational
            # This is a simplified implementation
            from fractions import Fraction
            f = Fraction(numerator).limit_denominator()
            numerator = BigInteger(f.numerator)
            denominator = BigInteger(f.denominator)
        else:
            numerator = BigInteger(numerator) if not isinstance(numerator, BigInteger) else numerator
            denominator = BigInteger(denominator) if not isinstance(denominator, BigInteger) else denominator
        
        if int(denominator) == 0:
            raise ZeroDivisionError("Denominator cannot be zero")
        
        # Normalize: move sign to numerator, make denominator positive
        if int(denominator) < 0:
            numerator = -numerator
            denominator = -denominator
        
        # Reduce to lowest terms
        gcd_val = numerator.gcd(denominator)
        self._numerator = BigInteger(int(numerator) // int(gcd_val))
        self._denominator = BigInteger(int(denominator) // int(gcd_val))
    
    @property
    def numerator(self):
        """分子 (Numerator)"""
        return self._numerator
    
    @property
    def denominator(self):
        """分母 (Denominator)"""
        return self._denominator
    
    def __str__(self):
        """字符串表示 (String representation)"""
        if int(self._denominator) == 1:
            return str(self._numerator)
        return f"{self._numerator}/{self._denominator}"
    
    def __repr__(self):
        """调试表示 (Debug representation)"""
        return f"Rational({self._numerator}, {self._denominator})"
    
    def __eq__(self, other):
        """相等比较 (Equality comparison)"""
        if isinstance(other, Rational):
            return (self._numerator == other._numerator and 
                    self._denominator == other._denominator)
        elif isinstance(other, (int, BigInteger)):
            return (int(self._denominator) == 1 and 
                    self._numerator == BigInteger(other))
        elif isinstance(other, float):
            return float(self) == other
        return False
    
    def __lt__(self, other):
        """小于比较 (Less than comparison)"""
        if isinstance(other, Rational):
            # a/b < c/d iff a*d < b*c (when b,d > 0)
            return (self._numerator * other._denominator < 
                    self._denominator * other._numerator)
        elif isinstance(other, (int, BigInteger)):
            return self._numerator < self._denominator * BigInteger(other)
        elif isinstance(other, float):
            return float(self) < other
        return NotImplemented
    
    def __le__(self, other):
        """小于等于比较 (Less than or equal comparison)"""
        return self.__lt__(other) or self.__eq__(other)
    
    def __gt__(self, other):
        """大于比较 (Greater than comparison)"""
        if isinstance(other, Rational):
            return (self._numerator * other._denominator > 
                    self._denominator * other._numerator)
        elif isinstance(other, (int, BigInteger)):
            return self._numerator > self._denominator * BigInteger(other)
        elif isinstance(other, float):
            return float(self) > other
        return NotImplemented
    
    def __ge__(self, other):
        """大于等于比较 (Greater than or equal comparison)"""
        return self.__gt__(other) or self.__eq__(other)
    
    def __add__(self, other):
        """加法 (Addition)"""
        if isinstance(other, Rational):
            # a/b + c/d = (a*d + b*c) / (b*d)
            new_num = self._numerator * other._denominator + self._denominator * other._numerator
            new_den = self._denominator * other._denominator
            return Rational(new_num, new_den)
        elif isinstance(other, (int, BigInteger)):
            return Rational(self._numerator + self._denominator * BigInteger(other), 
                          self._denominator)
        elif isinstance(other, float):
            return float(self) + other
        return NotImplemented
    
    def __radd__(self, other):
        """右加法 (Right addition)"""
        return self.__add__(other)
    
    def __sub__(self, other):
        """减法 (Subtraction)"""
        if isinstance(other, Rational):
            # a/b - c/d = (a*d - b*c) / (b*d)
            new_num = self._numerator * other._denominator - self._denominator * other._numerator
            new_den = self._denominator * other._denominator
            return Rational(new_num, new_den)
        elif isinstance(other, (int, BigInteger)):
            return Rational(self._numerator - self._denominator * BigInteger(other), 
                          self._denominator)
        elif isinstance(other, float):
            return float(self) - other
        return NotImplemented
    
    def __rsub__(self, other):
        """右减法 (Right subtraction)"""
        if isinstance(other, (int, BigInteger)):
            return Rational(self._denominator * BigInteger(other) - self._numerator, 
                          self._denominator)
        elif isinstance(other, float):
            return other - float(self)
        return NotImplemented
    
    def __mul__(self, other):
        """乘法 (Multiplication)"""
        if isinstance(other, Rational):
            # a/b * c/d = (a*c) / (b*d)
            return Rational(self._numerator * other._numerator,
                          self._denominator * other._denominator)
        elif isinstance(other, (int, BigInteger)):
            return Rational(self._numerator * BigInteger(other), self._denominator)
        elif isinstance(other, float):
            return float(self) * other
        return NotImplemented
    
    def __rmul__(self, other):
        """右乘法 (Right multiplication)"""
        return self.__mul__(other)
    
    def __truediv__(self, other):
        """除法 (Division)"""
        if isinstance(other, Rational):
            # a/b ÷ c/d = (a/b) * (d/c) = (a*d) / (b*c)
            if int(other._numerator) == 0:
                raise ZeroDivisionError("Division by zero")
            return Rational(self._numerator * other._denominator,
                          self._denominator * other._numerator)
        elif isinstance(other, (int, BigInteger)):
            if int(other) == 0:
                raise ZeroDivisionError("Division by zero")
            return Rational(self._numerator, self._denominator * BigInteger(other))
        elif isinstance(other, float):
            if other == 0.0:
                raise ZeroDivisionError("Division by zero")
            return float(self) / other
        return NotImplemented
    
    def __rtruediv__(self, other):
        """右除法 (Right division)"""
        if isinstance(other, (int, BigInteger)):
            if int(self._numerator) == 0:
                raise ZeroDivisionError("Division by zero")
            return Rational(self._denominator * BigInteger(other), self._numerator)
        elif isinstance(other, float):
            if int(self._numerator) == 0:
                raise ZeroDivisionError("Division by zero")
            return other / float(self)
        return NotImplemented
    
    def __pow__(self, other):
        """幂运算 (Power)"""
        if isinstance(other, int):
            if other == 0:
                return Rational(1)
            elif other > 0:
                return Rational(self._numerator ** other, self._denominator ** other)
            else:  # other < 0
                if int(self._numerator) == 0:
                    raise ZeroDivisionError("0.0 cannot be raised to a negative power")
                return Rational(self._denominator ** (-other), self._numerator ** (-other))
        elif isinstance(other, BigInteger):
            return self.__pow__(int(other))
        elif isinstance(other, float):
            return float(self) ** other
        return NotImplemented
    
    def __neg__(self):
        """取负 (Negation)"""
        return Rational(-self._numerator, self._denominator)
    
    def __pos__(self):
        """取正 (Positive)"""
        return Rational(self._numerator, self._denominator)
    
    def __abs__(self):
        """绝对值 (Absolute value)"""
        return Rational(abs(self._numerator), self._denominator)
    
    def __float__(self):
        """转换为float (Convert to float)"""
        return float(self._numerator) / float(self._denominator)
    
    def __int__(self):
        """转换为int (Convert to int) - truncates towards zero"""
        return int(self._numerator) // int(self._denominator)
    
    def __hash__(self):
        """哈希值 (Hash value)"""
        return hash((self._numerator, self._denominator))
    
    def limit_denominator(self, max_denominator=1000000):
        """
        限制分母大小 (Limit denominator size)
        
        Returns a rational approximation with denominator <= max_denominator
        
        Args:
            max_denominator: Maximum allowed denominator
            
        Returns:
            Rational: Approximation with limited denominator
        """
        if int(self._denominator) <= max_denominator:
            return Rational(self._numerator, self._denominator)
        
        from fractions import Fraction
        f = Fraction(int(self._numerator), int(self._denominator)).limit_denominator(max_denominator)
        return Rational(f.numerator, f.denominator)
    
    def to_mixed_number(self):
        """
        转换为带分数 (Convert to mixed number)
        
        Returns:
            tuple: (whole_part, fractional_part) where fractional_part is a Rational
        """
        whole = int(self._numerator) // int(self._denominator)
        remainder = int(self._numerator) % int(self._denominator)
        
        if remainder == 0:
            return (BigInteger(whole), Rational(0))
        else:
            return (BigInteger(whole), Rational(remainder, self._denominator))
    
    def is_integer(self):
        """
        检查是否为整数 (Check if this rational is an integer)
        
        Returns:
            bool: True if denominator is 1, False otherwise
        """
        return int(self._denominator) == 1