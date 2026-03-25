/*
 * LAMMP - Copyright (C) 2025-2026 HJimmyK(Jericho Knox)
 * This file is part of lammp, under the GNU LGPL v2 license.
 * See LICENSE in the project root for the full license text.
 */

#include <time.h> 
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <vector>

#include "../../../include/lammp/mprand.h"
#include "../include/test_long.hpp"

// -------------------------------------------------------------
// 基础工具函数
// -------------------------------------------------------------

// 将 uint64_t 归一化为 [0,1) 的 double
inline double normalize(uint64_t x) {
    // 除以 2^64，使用精确浮点表示
    return static_cast<double>(x) / 0x1p64;
}

// 单样本 KS 统计量（理论分布为 U(0,1)）
double ksStatisticUniform(const std::vector<double>& data) {
    int n = data.size();
    if (n == 0)
        return 0.0;
    std::vector<double> sorted = data;
    std::sort(sorted.begin(), sorted.end());
    double maxDiff = 0.0;
    for (int i = 0; i < n; ++i) {
        double ecdf = static_cast<double>(i + 1) / n;
        double cdf = sorted[i];
        double diff = std::abs(ecdf - cdf);
        if (diff > maxDiff)
            maxDiff = diff;

        double ecdf_left = static_cast<double>(i) / n;
        diff = std::abs(ecdf_left - cdf);
        if (diff > maxDiff)
            maxDiff = diff;
    }
    return maxDiff;
}

// 两样本 KS 统计量
double ksStatisticTwoSample(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.empty() || b.empty())
        return 0.0;

    std::vector<double> sorted_a = a;
    std::vector<double> sorted_b = b;
    std::sort(sorted_a.begin(), sorted_a.end());
    std::sort(sorted_b.begin(), sorted_b.end());

    double maxDiff = 0.0;
    size_t i = 0, j = 0;
    size_t na = sorted_a.size(), nb = sorted_b.size();

    while (i < na && j < nb) {
        if (sorted_a[i] < sorted_b[j]) {
            double fa = static_cast<double>(i + 1) / na;
            double fb = static_cast<double>(j) / nb;
            maxDiff = std::max(maxDiff, std::abs(fa - fb));
            ++i;
        } else {
            double fa = static_cast<double>(i) / na;
            double fb = static_cast<double>(j + 1) / nb;
            maxDiff = std::max(maxDiff, std::abs(fa - fb));
            ++j;
        }
    }

    // 处理剩余元素
    while (i < na) {
        double fa = static_cast<double>(i + 1) / na;
        double fb = 1.0;
        maxDiff = std::max(maxDiff, std::abs(fa - fb));
        ++i;
    }

    while (j < nb) {
        double fa = 1.0;
        double fb = static_cast<double>(j + 1) / nb;
        maxDiff = std::max(maxDiff, std::abs(fa - fb));
        ++j;
    }

    return maxDiff;
}

// 单样本KS检验p值计算（针对均匀分布）
double ksPvalueUniform(double D, int n) {
    if (n <= 0 || D < 0)
        return 1.0;

    double lambda = D * std::sqrt(static_cast<double>(n));
    double p = 0.0;
    const int max_terms = 100;

    for (int k = 1; k <= max_terms; ++k) {
        double term = 2.0 * std::exp(-2.0 * k * k * lambda * lambda);
        if (k % 2 == 0) {
            p -= term;
        } else {
            p += term;
        }
        // 提前终止：当项足够小时
        if (term < 1e-12)
            break;
    }

    return std::min(1.0, std::max(0.0, p));
}

// 两样本 KS 检验近似 p 值
double ksPvalueTwoSample(double D, int na, int nb) {
    double n = static_cast<double>(na * nb) / (na + nb);
    double lambda = (std::sqrt(n) + 0.12 + 0.11 / std::sqrt(n)) * D;
    double sum = 0.0;

    for (int k = 1; k <= 100; ++k) {
        double term = 2.0 * std::exp(-2.0 * k * k * lambda * lambda);
        if (term < 1e-12)
            break;  // 提前终止
        if (k % 2 == 0)
            sum -= term;
        else
            sum += term;
    }

    return std::min(1.0, std::max(0.0, sum));
}

// -------------------------------------------------------------
// 数据存储（按维度存储，提高缓存局部性）
// -------------------------------------------------------------
class DataMatrix {
   public:
    DataMatrix(int dim, int n) : dim_(dim), n_(n), data_(dim, std::vector<double>(n)) {}

    void set(int dimIdx, int sampleIdx, double val) { data_[dimIdx][sampleIdx] = val; }
    double get(int dimIdx, int sampleIdx) const { return data_[dimIdx][sampleIdx]; }
    int dim() const { return dim_; }
    int n() const { return n_; }

    const std::vector<double>& dimData(int d) const { return data_[d]; }

   private:
    int dim_, n_;
    std::vector<std::vector<double>> data_;
};

// -------------------------------------------------------------
// 均匀性检验基类
// -------------------------------------------------------------
class UniformityTest {
   public:
    virtual ~UniformityTest() = default;
    virtual bool run(const DataMatrix& data, double alpha) = 0;
    virtual void printResult(std::ostream& os) const = 0;
};

// -------------------------------------------------------------
// 边缘 KS 检验（每个维度单独检验）
// -------------------------------------------------------------
class MarginalKSTest : public UniformityTest {
   public:
    bool run(const DataMatrix& data, double alpha) override {
        int k = data.dim();
        int n = data.n();
        pvalues_.clear();
        bool reject = false;

        // Bonferroni校正
        double corrected_alpha = alpha / k;

        for (int d = 0; d < k; ++d) {
            const auto& vec = data.dimData(d);
            double D = ksStatisticUniform(vec);
            double p = ksPvalueUniform(D, n);
            pvalues_.push_back(p);

            // 检查是否拒绝原假设
            if (p < corrected_alpha) {
                reject = true;
            }
        }

        // 返回是否通过检验（未被拒绝）
        return !reject;
    }

    void printResult(std::ostream& os) const override {
        os << "dim-wise KS result (p values):\n";
        int rejected_count = 0;
        for (size_t i = 0; i < pvalues_.size(); ++i) {
            os << "dim " << i << ": p = " << std::setprecision(6) << pvalues_[i];
            if (pvalues_[i] < 0.05 / pvalues_.size()) {  // 显示被拒绝的维度
                os << " [REJECTED]";
                rejected_count++;
            }
            os << "\n";
            if ((i + 1) % 10 == 0)
                os << "\n";
        }
        os << "Total rejected dimensions: " << rejected_count << "/" << pvalues_.size() << "\n";
        os << std::flush;
    }

   private:
    std::vector<double> pvalues_;
};

// -------------------------------------------------------------
// 随机投影检验
// -------------------------------------------------------------
class RandomProjectionTest : public UniformityTest {
   public:
    RandomProjectionTest(int numDirs, int simSize = 10000) : numDirs_(numDirs), simSize_(simSize) {}

    bool run(const DataMatrix& data, double alpha) override {
        int k = data.dim();
        int n = data.n();

        // 使用固定种子的RNG，保证可复现性
        std::mt19937_64 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::normal_distribution<double> norm(0.0, 1.0);
        std::uniform_real_distribution<double> uni(0.0, 1.0);

        pvalues_.clear();
        bool reject = false;
        double corrected_alpha = alpha / numDirs_;

        for (int d = 0; d < numDirs_; ++d) {
            // 生成单位随机方向向量
            std::vector<double> w(k);
            double norm2 = 0.0;
            for (int i = 0; i < k; ++i) {
                w[i] = norm(rng);
                norm2 += w[i] * w[i];
            }
            double invNorm = 1.0 / std::sqrt(norm2);
            for (int i = 0; i < k; ++i) w[i] *= invNorm;

            // 计算样本投影
            std::vector<double> sampleProj(n);
            for (int idx = 0; idx < n; ++idx) {
                double proj = 0.0;
                for (int i = 0; i < k; ++i) {
                    proj += w[i] * data.get(i, idx);
                }
                sampleProj[idx] = proj;
            }

            // 生成理论投影分布（均匀分布随机向量的投影）
            std::vector<double> theoProj(simSize_);
            for (int s = 0; s < simSize_; ++s) {
                double proj = 0.0;
                for (int i = 0; i < k; ++i) {
                    proj += w[i] * uni(rng);
                }
                theoProj[s] = proj;
            }

            // 计算两样本KS统计量和p值
            double D = ksStatisticTwoSample(sampleProj, theoProj);
            double p = ksPvalueTwoSample(D, n, simSize_);
            pvalues_.push_back(p);

            if (p < corrected_alpha) {
                reject = true;
            }
        }

        return !reject;
    }

    void printResult(std::ostream& os) const override {
        os << "random projection result (p values):\n";
        int rejected_count = 0;
        for (size_t i = 0; i < pvalues_.size(); ++i) {
            os << "direction " << i << ": p = " << std::setprecision(6) << pvalues_[i];
            if (pvalues_[i] < 0.05 / pvalues_.size()) {
                os << " [REJECTED]";
                rejected_count++;
            }
            os << "\n";
            if ((i + 1) % 5 == 0)
                os << "\n";
        }
        os << "Total rejected directions: " << rejected_count << "/" << pvalues_.size() << "\n";
        os << std::flush;
    }

   private:
    int numDirs_;
    int simSize_;
    std::vector<double> pvalues_;
};

// -------------------------------------------------------------
// 统一测试入口（使用生成器）
// -------------------------------------------------------------
template <typename Generator>
void runUniformityTest(Generator&& gen,
                       int k,
                       int n,
                       const std::string& testMethod = "marginal",
                       int numDirs = 50,
                       double alpha = 0.05) {
    // 生成数据
    DataMatrix data(k, n);
    std::cout << "Generating " << n << " " << k << "-dimensional samples..." << std::endl;

    for (int i = 0; i < n; ++i) {
        std::vector<uint64_t> raw = gen();  // 生成一个样本
        if (raw.size() != static_cast<size_t>(k)) {
            std::cerr << "Error: sample " << i << " has wrong dimension (expected " << k << ", got " << raw.size()
                      << ")" << std::endl;
            return;
        }
        for (int d = 0; d < k; ++d) {
            data.set(d, i, normalize(raw[d]));
        }

        // 进度显示
        if ((i + 1) % 10000 == 0) {
            std::cout << "Generated " << (i + 1) << "/" << n << " samples..." << std::endl;
        }
    }
    std::cout << "Successfully generated " << n << " " << k << "-dimensional samples." << std::endl;

    // 创建检验器
    std::unique_ptr<UniformityTest> tester;
    if (testMethod == "marginal") {
        tester = std::make_unique<MarginalKSTest>();
    } else if (testMethod == "proj") {
        tester = std::make_unique<RandomProjectionTest>(numDirs);
    } else {
        std::cerr << "Unknown test method: " << testMethod << " (use 'marginal' or 'proj')" << std::endl;
        return;
    }

    // 运行检验
    std::cout << "Running " << testMethod << " test with alpha = " << alpha << "..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    bool uniform = tester->run(data, alpha);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);

    tester->printResult(std::cout);
    std::cout << "\nTest completed in " << duration.count() << " seconds." << std::endl;
    std::cout << "Conclusion: " << (uniform ? "PASS" : "REJECT") << " the uniformity test (alpha = " << alpha << ")"
              << std::endl;
}

// -------------------------------------------------------------
// 示例生成器（使用 lmmp 随机数生成器）
// -------------------------------------------------------------
std::vector<uint64_t> my_generator1(int k) {
    std::vector<uint64_t> vec(k);
    // 使用固定种子保证可复现性，或使用时间戳增加随机性
    static uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
    lmmp_strong_rng_t* rng = lmmp_strong_rng_init_(k, seed);
    lmmp_strong_random_(vec.data(), k, rng);
    lmmp_strong_rng_free_(rng);
    seed += 1;  // 每次调用更新种子，避免重复
    return vec;
}

std::vector<uint64_t> my_generator2(int k) {
    std::vector<uint64_t> vec(k);
    // 使用固定种子保证可复现性，或使用时间戳增加随机性
    static uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
    lmmp_seed_random_(vec.data(), k, seed, 0);
    seed += 1;  // 每次调用更新种子，避免重复
    return vec;
}

void test_random() {
    int k = 2000;                  // 维度
    int n = 100000;                // 样本数
    std::string method = "proj";  // "marginal" 或 "proj"
    int numDirs = 100;            // 投影方向数（仅对 proj 有效）
    double alpha = 0.000001;          // 显著性水平

    std::cout << "=== Random Uniformity Test ========" << std::endl;
    std::cout << "Configuration:                " << std::endl;
    std::cout << "  Dimensions (k):             " << k << std::endl;
    std::cout << "  Samples (n):                " << n << std::endl;
    std::cout << "  Test method:                " << method << std::endl;
    std::cout << "  Projection directions:      " << numDirs << std::endl;
    std::cout << "  Significance level (alpha): " << alpha << std::endl;
    std::cout << "===================================" << std::endl;

    // 定义生成器（绑定k值）
    auto generator = [k]() -> std::vector<uint64_t> { return my_generator2(k); };

    // 运行检验
    runUniformityTest(generator, k, n, method, numDirs, alpha);
}
