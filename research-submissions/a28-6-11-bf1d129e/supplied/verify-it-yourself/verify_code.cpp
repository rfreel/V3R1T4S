#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <omp.h>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 5) {
        cerr << "Usage: verify_code FILE N D W [EXPECTED_M]\n";
        return 1;
    }
    string path = argv[1];
    int n = stoi(argv[2]);
    int d = stoi(argv[3]);
    int w = stoi(argv[4]);
    int exp_m = (argc >= 6) ? stoi(argv[5]) : -1;

    ifstream f(path);
    if (!f) { cerr << "Cannot open " << path << "\n"; return 1; }
    vector<uint32_t> words;
    string s;
    while (f >> s) {
        if ((int)s.size() == n && (s[0] == '0' || s[0] == '1')) {
            uint32_t v = 0;
            for (int i = 0; i < n; ++i) {
                if (s[i] == '1') v |= (1u << i);
                else if (s[i] != '0') { cerr << "Invalid char\n"; return 1; }
            }
            if (__builtin_popcount(v) != w) { cerr << "Weight mismatch\n"; return 1; }
            words.push_back(v);
        } else {
            uint32_t v = stoul(s, nullptr, 16);
            if (__builtin_popcount(v) != w) { cerr << "Hex weight mismatch\n"; return 1; }
            words.push_back(v);
        }
    }
    int M = words.size();
    if (exp_m > 0 && M != exp_m) {
        cerr << "Count mismatch: got " << M << ", expected " << exp_m << "\n";
        return 1;
    }
    vector<uint32_t> sorted_w = words;
    sort(sorted_w.begin(), sorted_w.end());
    for (int i = 1; i < M; ++i) {
        if (sorted_w[i] == sorted_w[i - 1]) {
            cerr << "Duplicate word detected!\n";
            return 1;
        }
    }
    int min_d = 999;
    #pragma omp parallel for reduction(min:min_d) schedule(dynamic, 256)
    for (int i = 0; i < M; ++i) {
        uint32_t u = words[i];
        int local_min = 999;
        for (int j = i + 1; j < M; ++j) {
            int dist = __builtin_popcount(u ^ words[j]);
            if (dist < local_min) local_min = dist;
        }
        if (local_min < min_d) min_d = local_min;
    }
    if (min_d < d) {
        cerr << "FAIL: min_d = " << min_d << " < " << d << "\n";
        return 1;
    }
    cout << "VALID: " << path << " | A(" << n << "," << d << "," << w << ") >= " << M
         << " | pairs=" << (1LL * M * (M - 1) / 2) << " | min_dist=" << min_d << " | PASS\n";
    return 0;
}
