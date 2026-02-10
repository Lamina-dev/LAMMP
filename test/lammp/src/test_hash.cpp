#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../../include/lammp/secret.h"
#include "../include/test_long.hpp"


// Define hash function pointer type:
// Input: uint64_t* ptr (data pointer), size_t len (data length in uint64_t units), uint64_t seed
// Output: uint64_t (hash value)
using HashFunc = uint64_t (*)(uint64_t* ptr, size_t len, uint64_t seed);

// ---------------------- Utility Functions ----------------------
/**
 * Generate random uint64_t data array for test
 * @param length Number of uint64_t elements
 * @return Random uint64_t vector
 */
std::vector<uint64_t> generateRandomData(size_t length = 4) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, UINT64_MAX);

    std::vector<uint64_t> data;
    data.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        data.push_back(dis(gen));
    }
    return data;
}

/**
 * Calculate Hamming distance between two uint64_t values
 * @param hash1 First hash value
 * @param hash2 Second hash value
 * @return Hamming distance (number of different bits)
 */
int hammingDistance(uint64_t hash1, uint64_t hash2) {
    uint64_t xor_result = hash1 ^ hash2;
    int distance = 0;
    // Count number of 1 bits in xor_result (Brian Kernighan's algorithm)
    while (xor_result != 0) {
        xor_result &= xor_result - 1;
        distance++;
    }
    return distance;
}

/**
 * Chi-square test: Evaluate distribution uniformity
 * @param freq Frequency of each bin
 * @param expectedFreq Expected frequency (theoretical value for uniform distribution)
 * @return Chi-square value
 */
double chiSquareTest(const std::vector<int>& freq, double expectedFreq) {
    double chiSquare = 0.0;
    for (int f : freq) {
        double diff = f - expectedFreq;
        chiSquare += (diff * diff) / expectedFreq;
    }
    return chiSquare;
}

// ---------------------- Core Test Function ----------------------
/**
 * Test statistical performance of hash function
 * @param hashFunc Hash function pointer
 * @param testSize Number of test samples
 * @param binNum Number of bins for uniformity test
 * @param seed Seed for hash function
 * @param dataLen Length of each test data (in uint64_t units)
 */
void testHashPerformance(HashFunc hashFunc,
                         size_t testSize = 100000,
                         int binNum = 100,
                         uint64_t seed = 123456,
                         size_t dataLen = 4) {
    if (!hashFunc) {
        std::cerr << "Error: Hash function pointer is null!" << std::endl;
        return;
    }

    // 1. Generate test data samples
    std::vector<std::vector<uint64_t>> testSamples;
    testSamples.reserve(testSize);
    for (size_t i = 0; i < testSize; ++i) {
        testSamples.push_back(generateRandomData(dataLen));
    }

    // 2. Calculate all hash values
    std::vector<uint64_t> hashValues;
    hashValues.reserve(testSize);
    try {
        for (const auto& data : testSamples) {
            uint64_t hashVal = hashFunc(const_cast<uint64_t*>(data.data()), data.size(), seed);
            hashValues.push_back(hashVal);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error calculating hash values: " << e.what() << std::endl;
        return;
    }

    // ---------------------- Test 1: Distribution Uniformity ----------------------
    // Find min/max hash values for bin division
    uint64_t minHash = UINT64_MAX, maxHash = 0;
    for (uint64_t hv : hashValues) {
        minHash = std::min(minHash, hv);
        maxHash = std::max(maxHash, hv);
    }

    // Handle edge case where all hash values are the same
    double binWidth = (maxHash == minHash) ? 1.0 : (maxHash - minHash) / static_cast<double>(binNum);
    std::vector<int> freq(binNum, 0);

    for (uint64_t hv : hashValues) {
        int binIdx = static_cast<int>((hv - minHash) / binWidth);
        // Ensure bin index is within range
        binIdx = std::min(binIdx, binNum - 1);
        freq[binIdx]++;
    }

    // Calculate statistical indicators
    double expectedFreq = static_cast<double>(testSize) / binNum;
    double chiSquare = chiSquareTest(freq, expectedFreq);
    int df = binNum - 1;  // Degrees of freedom

    // Calculate frequency standard deviation
    double meanFreq = 0.0;
    for (int f : freq) {
        meanFreq += f;
    }
    meanFreq /= binNum;
    double stdFreq = 0.0;
    for (int f : freq) {
        double diff = f - meanFreq;
        stdFreq += diff * diff;
    }
    stdFreq = std::sqrt(stdFreq / binNum);

    // ---------------------- Test 2: Collision Rate ----------------------
    std::unordered_set<uint64_t> uniqueHashes;
    size_t collisionNum = 0;
    for (uint64_t hv : hashValues) {
        if (!uniqueHashes.insert(hv).second) {
            collisionNum++;
        }
    }
    double collisionRate = static_cast<double>(collisionNum) / testSize * 100;

    // ---------------------- Test 3: Avalanche Effect ----------------------
    const size_t avalancheTestSize = std::min(static_cast<size_t>(1000), testSize);
    int totalHd = 0;
    const int hashBitLength = 64;           // uint64_t is 64 bits
    const int idealHd = hashBitLength / 2;  // Ideal Hamming distance (50% bit flip)

    for (size_t i = 0; i < avalancheTestSize; ++i) {
        const auto& originalData = testSamples[i];
        // Create modified data (flip 1 bit in the first uint64_t element)
        std::vector<uint64_t> modifiedData = originalData;
        if (!modifiedData.empty()) {
            modifiedData[0] ^= 1ULL;  // Flip the least significant bit

            // Calculate original and modified hash values
            uint64_t originalHash = hashFunc(const_cast<uint64_t*>(originalData.data()), originalData.size(), seed);
            uint64_t modifiedHash = hashFunc(const_cast<uint64_t*>(modifiedData.data()), modifiedData.size(), seed);

            totalHd += hammingDistance(originalHash, modifiedHash);
        }
    }

    double avgHd = static_cast<double>(totalHd) / avalancheTestSize;
    double avalancheRatio = (avgHd / idealHd) * 100;

    // ---------------------- Output Test Results ----------------------
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "==================== Hash Function Statistical Performance Test ====================" << std::endl;

    std::cout << "\n1. Distribution Uniformity:" << std::endl;
    std::cout << "   - Chi-square Value: " << chiSquare << std::endl;
    std::cout << "   - Degrees of Freedom: " << df << std::endl;
    std::cout << "   - Frequency Standard Deviation: " << std::setprecision(2) << stdFreq << std::endl;
    std::cout << "   - Expected Frequency per Bin: " << expectedFreq << std::endl;

    std::cout << "\n2. Collision Rate:" << std::endl;
    std::cout << "   - Number of Collisions: " << collisionNum << std::endl;
    std::cout << "   - Collision Rate (%): " << collisionRate << std::endl;

    std::cout << "\n3. Avalanche Effect:" << std::endl;
    std::cout << "   - Hash Value Bit Length: " << hashBitLength << std::endl;
    std::cout << "   - Average Hamming Distance: " << std::setprecision(2) << avgHd << std::endl;
    std::cout << "   - Ideal Hamming Distance: " << idealHd << std::endl;
    std::cout << "   - Avalanche Effect Compliance Rate (%): " << std::setprecision(2) << avalancheRatio << std::endl;

    std::cout << "==================================================================================" << std::endl;
}

// ---------------------- Example: Custom Hash Function ----------------------
/**
 * Simple custom hash function (for test only)
 * Conforms to the HashFunc interface: uint64_t (*)(uint64_t* ptr, size_t len, uint64_t seed)
 */
uint64_t customHashFunction1(uint64_t* ptr, size_t len, uint64_t seed) {
    return lmmp_siphash24_(ptr, len, key128_t{ 0x0706050403020100ULL, seed });
}

uint64_t customHashFunction2(uint64_t* ptr, size_t len, uint64_t seed) {
    return lmmp_xxhash_(ptr, len, seed);
}

void test_hash() {
    auto start = std::chrono::high_resolution_clock::now();
    testHashPerformance(customHashFunction1, 2000000, 100, 1234567890, 5);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Custom hash function 1 took " << duration << " microseconds" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    testHashPerformance(customHashFunction2, 2000000, 100, 1234567890, 5);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Custom hash function 2 took " << duration << " microseconds" << std::endl;
}