"""
Lamina BigNum - 任意精度计算库
Lamina language's arbitrary precision calculation library

This library provides:
- BigInteger: 大整数 (arbitrary precision integers) ✓ Implemented
- Rational: 有理数 (rational numbers) ✓ Implemented
- ArbitraryFloat: 任意精度浮点数 (arbitrary precision floating point - planned)
- FixedPoint: 任意精度定点数 (arbitrary precision fixed point - planned)
"""

from .biginteger import BigInteger
from .rational import Rational

# Planned features - not yet implemented
# from .arbitraryfloat import ArbitraryFloat  # Future implementation
# from .fixedpoint import FixedPoint          # Future implementation

__version__ = "0.1.0"
__author__ = "Lamina-dev"
__description__ = "Lamina的任意精度计算库"

__all__ = [
    "BigInteger",
    "Rational",
    # Future exports when implemented:
    # "ArbitraryFloat",
    # "FixedPoint",
]