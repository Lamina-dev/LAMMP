#include "../../../include/lammp/impl/3ntt_crt_const.h"

/* 原根 */
const uint32_t global_ROOT1 = 5u;
const uint32_t global_ROOT2 = 5u;
const uint32_t global_ROOT3 = 3u;

/* 原根关于对应模数的逆 */
const uint64_t global_root_inv1 = 1491592196585108276ull;
const uint64_t global_root_inv2 = 1167333023414432564ull;
const uint64_t global_root_inv3 = 1393113484733273430ull;

/* 模数 */
const uint64_t global_mod1 = 2485986994308513793ull;
const uint64_t global_mod2 = 1945555039024054273ull;
const uint64_t global_mod3 = 4179340454199820289ull;

/* R^2 mod 模数 */
const uint64_t global_r21 = 1974795801822054070ull;
const uint64_t global_r22 = 269548777697434221ull;
const uint64_t global_r23 = 1878466934230121386ull;

/* 模数的平方 */
const uint64_t global_mod21 = 4971973988617027586ull;
const uint64_t global_mod22 = 3891110078048108546ull;
const uint64_t global_mod23 = 8358680908399640578ull;

/* mont64(ROOT) */
const mont64 global_mont_ROOT1 = 252201579132747739ull;
const mont64 global_mont_ROOT2 = 792633534417207249ull;
const mont64 global_mont_ROOT3 = 1008806316530991091ull;

/* ROOTinv = ROOT^-1 % mod */
/* mont64(ROOTinv)         */
const mont64 global_mont_ROOT_inv1 = 208967022709991013ull;
const mont64 global_mont_ROOT_inv2 = 965571760108234341ull;
const mont64 global_mont_ROOT_inv3 = 3362687721769970346ull;

/* (mod_inv * mod) % R = 1         */
/* (mod_inv_neg + mod_inv) % R = 0 */
const uint64_t global_modInvNeg1 = 2485986994308513791ull;
const uint64_t global_modInvNeg2 = 1945555039024054271ull;
const uint64_t global_modInvNeg3 = 4179340454199820287ull;

/*  W_4_1 = qpow(mont64(ROOT), (mod - 1) / 4);  */
const mont64 global_w41_1 = 1114193638674092305ull;
const mont64 global_w41_2 = 1227753429952047858ull;
const mont64 global_w41_3 = 2751416685589087298ull;

/*  W_4_1 = qpow(mont64(ROOTinv), (mod - 1) / 4);  */
const mont64 global_w41_inv1 = 1371793355634421488ull;
const mont64 global_w41_inv2 = 717801609072006415ull;
const mont64 global_w41_inv3 = 1427923768610732991ull;

/*
 mont64 w1 = qpow(mont64(ROOT), (mod() - 1) / 8);
 mont64 w2 = qpow(w1, 2);
 mont64 w3 = qpow(w1, 3);
*/
const mont64 global_w1_1 = 1397546744561501820ull;
const mont64 global_w2_1 = 1114193638674092305ull;
const mont64 global_w3_1 = 1540210943987252404ull;

const mont64 global_w1_2 = 352122307744000116ull;
const mont64 global_w2_2 = 1227753429952047858ull;
const mont64 global_w3_2 = 1132446406557179953ull;

const mont64 global_w1_3 = 457531513967587773ull;
const mont64 global_w2_3 = 2751416685589087298ull;
const mont64 global_w3_3 = 2098898615074297118ull;

/*
 mont64 w1 = qpow(mont64(ROOTinv), (mod() - 1) / 8);
 mont64 w2 = qpow(w1, 2);
 mont64 w3 = qpow(w1, 3);
*/
const mont64 global_w1_inv1 = 945776050321261389ull;
const mont64 global_w2_inv1 = 1371793355634421488ull;
const mont64 global_w3_inv1 = 1088440249747011973ull;

const mont64 global_w1_inv2 = 813108632466874320ull;
const mont64 global_w2_inv2 = 717801609072006415ull;
const mont64 global_w3_inv2 = 1593432731280054157ull;

const mont64 global_w1_inv3 = 2080441839125523171ull;
const mont64 global_w2_inv3 = 1427923768610732991ull;
const mont64 global_w3_inv3 = 3721808940232232516ull;

/* mont64(1) */
const mont64 global_one1 = 1044835113549955065ull;
const mont64 global_one2 = 936748722493063159ull;
const mont64 global_one3 = 1729382256910270460ull;