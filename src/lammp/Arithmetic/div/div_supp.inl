/*
 * [LAMMP]
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

Copyright (c) 2024-2050 Twilight-Dream & With-Sky & HJimmyK

https://github.com/Twilight-Dream-Of-Magic/
https://github.com/With-Sky
https://github.com/HJimmyK

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

#ifndef __LAMMP_DIV_SUPP_INL__
#define __LAMMP_DIV_SUPP_INL__
template <typename NumTy, typename ProdTy>
class DivSupporter {
   private:
    NumTy divisor = 0;
    NumTy inv = 0;
    int shift = 0, shift1 = 0, shift2 = 0;
    enum : int { NUM_BITS = sizeof(NumTy) * CHAR_BIT };

   public:
    constexpr DivSupporter(NumTy divisor_in) : divisor(divisor_in) {
        inv = getInv(divisor, shift);
        divisor <<= shift;
        shift1 = shift / 2;
        shift2 = shift - shift1;
    }
    // Return dividend / divisor, dividend %= divisor
    NumTy divMod(ProdTy& dividend) const {
        dividend <<= shift;
        NumTy r = NumTy(dividend);
        dividend = (dividend >> NUM_BITS) * inv + dividend;
        NumTy q1 = NumTy(dividend >> NUM_BITS) + 1;
        r -= q1 * divisor;
        if (r > NumTy(dividend)) {
            q1--;
            r += divisor;
        }
        if (r >= divisor) {
            q1++;
            r -= divisor;
        }
        dividend = r >> shift;
        return q1;
    }

    void prodDivMod(NumTy a, NumTy b, NumTy& quot, NumTy& rem) const {
        ProdTy dividend = ProdTy(a << shift1) * (b << shift2);
        rem = NumTy(dividend);
        dividend = (dividend >> NUM_BITS) * inv + dividend;
        quot = NumTy(dividend >> NUM_BITS) + 1;
        rem -= quot * divisor;
        if (rem > NumTy(dividend)) {
            quot--;
            rem += divisor;
        }
        if (rem >= divisor) {
            quot++;
            rem -= divisor;
        }
        rem >>= shift;
    }

    NumTy div(ProdTy dividend) const { return divMod(dividend); }
    NumTy mod(ProdTy dividend) const {
        divMod(dividend);
        return dividend;
    }

    static constexpr NumTy getInv(NumTy divisor, int& leading_zero) {
        constexpr NumTy MAX = all_one<NumTy>(NUM_BITS);
        leading_zero = lammp_clz(divisor);
        divisor <<= leading_zero;
        ProdTy x = ProdTy(MAX - divisor) << NUM_BITS;
        return NumTy((x + MAX) / divisor);
    }
};
#endif // __LAMMP_DIV_SUPP_INL__
