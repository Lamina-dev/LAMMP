#include <fstream>  
#include "include/benchmark.hpp"


int main() {
    std::ofstream outFile("output.txt");  

    if (!outFile.is_open()) {
        std::cerr << "无法打开文件 output.txt 以写入！\n";
        return 1;
    }
    for (int i = 0; i < 100; i++) {
        int a = 25 * i + 100;
        for (int j = 0; j < 100; j++) {
            int exp = (5 * j + 20);
            outFile << a << "," << exp << ",";
            outFile << bench_pow_win2(a, exp) << ",";
            outFile << bench_pow_basecase(a, exp) << ";";
        }
        outFile << "\n"; 
    }
    outFile.close();
    return 0;
}