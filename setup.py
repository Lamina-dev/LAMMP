#!/usr/bin/env python3
"""
Setup file for Lamina BigNum library.
"""

from setuptools import setup, find_packages

setup(
    name="lamina-bignum",
    version="0.1.0",
    description="Lamina language's arbitrary precision calculation library",
    long_description="Lamina的任意精度计算库，包含大整数，有理数，任意精度浮点数（暂未实现），任意精度定点数（暂未实现）",
    author="Lamina-dev",
    packages=find_packages(),
    python_requires=">=3.6",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: GNU Lesser General Public License v2 or later (LGPLv2+)",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Topic :: Scientific/Engineering :: Mathematics",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
)