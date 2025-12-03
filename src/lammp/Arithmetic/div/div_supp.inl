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
