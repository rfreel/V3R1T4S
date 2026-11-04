#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <chrono>
#include <omp.h>

using namespace std;

struct GF27 {
    int c0, c1, c2;
    int to_int() const { return c0 + 3 * c1 + 9 * c2; }
    static GF27 from_int(int v) { return {v % 3, (v / 3) % 3, (v / 9) % 3}; }
};

GF27 add(GF27 a, GF27 b) {
    return {(a.c0 + b.c0) % 3, (a.c1 + b.c1) % 3, (a.c2 + b.c2) % 3};
}

GF27 sub_gf(GF27 a, GF27 b) {
    return {(a.c0 - b.c0 + 3) % 3, (a.c1 - b.c1 + 3) % 3, (a.c2 - b.c2 + 3) % 3};
}

GF27 mul(GF27 a, GF27 b) {
    int d[5] = {0, 0, 0, 0, 0};
    int ac[3] = {a.c0, a.c1, a.c2};
    int bc[3] = {b.c0, b.c1, b.c2};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            d[i + j] = (d[i + j] + ac[i] * bc[j]) % 3;
    d[2] = (d[2] + d[4]) % 3;
    d[1] = (d[1] + 2 * d[4]) % 3;
    d[1] = (d[1] + d[3]) % 3;
    d[0] = (d[0] + 2 * d[3]) % 3;
    return {d[0], d[1], d[2]};
}

GF27 inv(GF27 a) {
    GF27 res = {1, 0, 0};
    for (int i = 0; i < 25; ++i) res = mul(res, a);
    return res;
}

GF27 frob(GF27 z, int s) {
    GF27 res = z;
    for (int i = 0; i < s; ++i) res = mul(mul(res, res), res);
    return res;
}

int main() {
    vector<bool> is_sq(27, false);
    for (int a = 1; a < 27; ++a) {
        GF27 ga = GF27::from_int(a);
        is_sq[mul(ga, ga).to_int()] = true;
    }

    set<vector<int>> psl_set;
    for (int a = 0; a < 27; ++a) {
        GF27 ga = GF27::from_int(a);
        for (int b = 0; b < 27; ++b) {
            GF27 gb = GF27::from_int(b);
            for (int c = 0; c < 27; ++c) {
                GF27 gc = GF27::from_int(c);
                for (int d = 0; d < 27; ++d) {
                    GF27 gd = GF27::from_int(d);
                    GF27 det = sub_gf(mul(ga, gd), mul(gb, gc));
                    int det_i = det.to_int();
                    if (det_i == 0 || !is_sq[det_i]) continue;
                    vector<int> p(28);
                    for (int z = 0; z < 28; ++z) {
                        if (z == 27) {
                            p[z] = (c == 0) ? 27 : mul(ga, inv(gc)).to_int();
                        } else {
                            GF27 gz = GF27::from_int(z);
                            GF27 num = add(mul(ga, gz), gb);
                            GF27 den = add(mul(gc, gz), gd);
                            p[z] = (den.to_int() == 0) ? 27 : mul(num, inv(den)).to_int();
                        }
                    }
                    psl_set.insert(p);
                }
            }
        }
    }
    vector<vector<int>> psl_perms(psl_set.begin(), psl_set.end());
    int G = psl_perms.size(); // 9828

    vector<uint32_t> tbl0(G * 1024), tbl1(G * 512), tbl2(G * 512);
    for (int g = 0; g < G; ++g) {
        for (int m = 0; m < 1024; ++m) {
            uint32_t r0 = 0;
            for (int b = 0; b < 10; ++b) if (m & (1 << b)) r0 |= (1u << psl_perms[g][b]);
            tbl0[g * 1024 + m] = r0;
        }
        for (int m = 0; m < 512; ++m) {
            uint32_t r1 = 0, r2 = 0;
            for (int b = 0; b < 9; ++b) {
                if (m & (1 << b)) {
                    r1 |= (1u << psl_perms[g][b + 10]);
                    r2 |= (1u << psl_perms[g][b + 19]);
                }
            }
            tbl1[g * 512 + m] = r1;
            tbl2[g * 512 + m] = r2;
        }
    }

    vector<uint8_t> vis(1u << 28, 0);
    struct Orbit {
        uint32_t rep;
        int size;
        int min_d;
        vector<uint32_t> elems;
    };
    vector<Orbit> valid_orbits;
    vector<uint32_t> cur_elems;
    cur_elems.reserve(9828);

    uint32_t comb = (1u << 11) - 1;
    while (comb < (1u << 28)) {
        uint32_t w = comb;
        if (!vis[w]) {
            cur_elems.clear();
            uint32_t m0 = w & 1023, m1 = (w >> 10) & 511, m2 = (w >> 19) & 511;
            for (int g = 0; g < G; ++g) {
                uint32_t img = tbl0[g * 1024 + m0] | tbl1[g * 512 + m1] | tbl2[g * 512 + m2];
                if (!vis[img]) {
                    vis[img] = 1;
                    cur_elems.push_back(img);
                }
            }
            bool ok = true;
            int min_d = 28;
            for (uint32_t img : cur_elems) {
                if (img != w) {
                    int d = 22 - 2 * __builtin_popcount(w & img);
                    if (d < min_d) min_d = d;
                    if (d < 6) { ok = false; break; }
                }
            }
            if (ok) {
                sort(cur_elems.begin(), cur_elems.end());
                valid_orbits.push_back({cur_elems[0], (int)cur_elems.size(), min_d, cur_elems});
            }
        }
        uint32_t x = comb & -comb;
        uint32_t y = comb + x;
        comb = (((comb & ~y) / x) >> 1) | y;
    }

    int V = valid_orbits.size(); // 334
    vis.clear();
    vis.shrink_to_fit();
    vector<int16_t> word_to_orb(1u << 28, -1);
    for (int i = 0; i < V; ++i) {
        for (uint32_t u : valid_orbits[i].elems) word_to_orb[u] = (int16_t)i;
    }

    int W_WORDS = (V + 63) / 64;
    vector<uint64_t> adj((size_t)V * W_WORDS, 0);
    #pragma omp parallel for schedule(dynamic, 8)
    for (int i = 0; i < V; ++i) {
        uint32_t r = valid_orbits[i].rep;
        int ones[11], zeros[17], no = 0, nz = 0;
        for (int b = 0; b < 28; ++b) {
            if (r & (1u << b)) ones[no++] = b;
            else zeros[nz++] = b;
        }
        uint64_t* row = &adj[(size_t)i * W_WORDS];
        for (int a = 0; a < 11; ++a) {
            uint32_t r1 = r ^ (1u << ones[a]);
            for (int x = 0; x < 17; ++x) {
                int16_t j = word_to_orb[r1 | (1u << zeros[x])];
                if (j >= 0 && j != i) row[j >> 6] |= (1ULL << (j & 63));
            }
        }
        for (int a = 0; a < 11; ++a) {
            for (int b = a + 1; b < 11; ++b) {
                uint32_t r2 = r ^ (1u << ones[a]) ^ (1u << ones[b]);
                for (int x = 0; x < 17; ++x) {
                    uint32_t r3 = r2 | (1u << zeros[x]);
                    for (int y = x + 1; y < 17; ++y) {
                        int16_t j = word_to_orb[r3 | (1u << zeros[y])];
                        if (j >= 0 && j != i) row[j >> 6] |= (1ULL << (j & 63));
                    }
                }
            }
        }
    }
    auto has_edge = [&](int u, int v) -> bool {
        return (adj[(size_t)u * W_WORDS + (v >> 6)] >> (v & 63)) & 1ULL;
    };

    // Degree stats of V=334 conflict graph
    vector<int> degs(V, 0);
    for (int i = 0; i < V; ++i) {
        for (int j = 0; j < V; ++j) if (i != j && has_edge(i, j)) degs[i]++;
    }
    vector<int> s_degs = degs;
    sort(s_degs.begin(), s_degs.end());
    double sum_deg = 0;
    for (int d : s_degs) sum_deg += d;
    cout << "PSL(2, 27) Conflict Degree Stats (V=334): min = " << s_degs.front()
         << ", median = " << s_degs[V / 2] << ", mean = " << (sum_deg / V)
         << ", max = " << s_degs.back() << "\n";

    // Enumerate ALL 3-cliques (triangles in the complement graph) of weight 29,484!
    vector<array<int, 3>> triangles;
    for (int i = 0; i < V; ++i) {
        for (int j = i + 1; j < V; ++j) {
            if (has_edge(i, j)) continue;
            for (int k = j + 1; k < V; ++k) {
                if (!has_edge(i, k) && !has_edge(j, k)) {
                    triangles.push_back({i, j, k});
                }
            }
        }
    }
    cout << "\nTotal distinct 3-orbit cliques of size 29,484 in PSL(2, 27): " << triangles.size() << "\n";

    // Check how Frobenius (z -> z^3, fixing 27) and non-square scaling (z -> 2*z, fixing 27) permute the 334 valid orbits!
    vector<int> frob_perm(28), nsq_perm(28);
    for (int z = 0; z < 27; ++z) {
        frob_perm[z] = frob(GF27::from_int(z), 1).to_int();
        nsq_perm[z] = mul({2, 0, 0}, GF27::from_int(z)).to_int(); // 2 = -1 is a non-square in GF(27)!
    }
    frob_perm[27] = 27;
    nsq_perm[27] = 27;

    auto apply_p = [&](uint32_t w, const vector<int>& p) -> uint32_t {
        uint32_t res = 0;
        for (int b = 0; b < 28; ++b) if (w & (1u << b)) res |= (1u << p[b]);
        return res;
    };

    vector<int> frob_orb(V), nsq_orb(V);
    for (int i = 0; i < V; ++i) {
        frob_orb[i] = word_to_orb[apply_p(valid_orbits[i].rep, frob_perm)];
        nsq_orb[i] = word_to_orb[apply_p(valid_orbits[i].rep, nsq_perm)];
    }

    int psigmal_single_orbits = 0;
    for (size_t t = 0; t < triangles.size(); ++t) {
        int a = triangles[t][0], b = triangles[t][1], c = triangles[t][2];
        set<int> tri_set = {a, b, c};
        set<int> frob_set = {a, frob_orb[a], frob_orb[frob_orb[a]]};
        bool is_psigmal = (tri_set == frob_set);
        if (is_psigmal) psigmal_single_orbits++;
        cout << "  Triangle " << t << ": {" << a << ", " << b << ", " << c << "} "
             << "| reps = {" << valid_orbits[a].rep << ", " << valid_orbits[b].rep << ", " << valid_orbits[c].rep << "} "
             << "| degs = {" << degs[a] << ", " << degs[b] << ", " << degs[c] << "} "
             << "| Single PSigmaL(2,27) Orbit? " << (is_psigmal ? "YES" : "NO") << "\n";
    }
    cout << "Number of 29,484-word codes that are a SINGLE orbit of PSigmaL(2, 27): " << psigmal_single_orbits << "\n";

    // Free word_to_orb and test residual unblocked words in J(28, 11) for Triangle 0 (and check if any triangle has > 0 residual words)!
    word_to_orb.clear();
    word_to_orb.shrink_to_fit();
    vector<uint8_t> blocked(1u << 28, 0);

    for (size_t t = 0; t < triangles.size(); ++t) {
        fill(blocked.begin(), blocked.end(), 0);
        vector<uint32_t> code_words;
        code_words.reserve(29484);
        for (int u : triangles[t]) {
            for (uint32_t w : valid_orbits[u].elems) code_words.push_back(w);
        }

        #pragma omp parallel for schedule(dynamic, 256)
        for (size_t idx = 0; idx < code_words.size(); ++idx) {
            uint32_t r = code_words[idx];
            blocked[r] = 1;
            int ones[11], zeros[17], no = 0, nz = 0;
            for (int b = 0; b < 28; ++b) {
                if (r & (1u << b)) ones[no++] = b;
                else zeros[nz++] = b;
            }
            for (int a = 0; a < 11; ++a) {
                uint32_t r1 = r ^ (1u << ones[a]);
                for (int x = 0; x < 17; ++x) blocked[r1 | (1u << zeros[x])] = 1;
            }
            for (int a = 0; a < 11; ++a) {
                for (int b = a + 1; b < 11; ++b) {
                    uint32_t r2 = r ^ (1u << ones[a]) ^ (1u << ones[b]);
                    for (int x = 0; x < 17; ++x) {
                        uint32_t r3 = r2 | (1u << zeros[x]);
                        for (int y = x + 1; y < 17; ++y) blocked[r3 | (1u << zeros[y])] = 1;
                    }
                }
            }
        }

        long long res_cnt = 0;
        uint32_t c_iter = (1u << 11) - 1;
        while (c_iter < (1u << 28)) {
            if (!blocked[c_iter]) res_cnt++;
            uint32_t x = c_iter & -c_iter;
            uint32_t y = c_iter + x;
            c_iter = (((c_iter & ~y) / x) >> 1) | y;
        }
        cout << "  Triangle " << t << " residual unblocked words in J(28, 11) (out of 21,474,180): " << res_cnt << "\n";
    }

    // Save Triangle 0 (or best triangle) to artifacts/file_generation/ttl=63d/output/A28_6_11_29484.txt
    vector<uint32_t> code_words;
    for (int u : triangles[0]) {
        for (uint32_t w : valid_orbits[u].elems) code_words.push_back(w);
    }
    int M = code_words.size();
    vector<string> str_words;
    str_words.reserve(M);
    for (uint32_t w : code_words) {
        string s(28, '0');
        for (int b = 0; b < 28; ++b) if (w & (1u << b)) s[b] = '1';
        str_words.push_back(s);
    }
    sort(str_words.begin(), str_words.end());
    {
        ofstream fo("artifacts/file_generation/ttl=63d/output/A28_6_11_29484.txt");
        for (auto &s : str_words) fo << s << "\n";
    }
    cout << "\nSaved artifacts/file_generation/ttl=63d/output/A28_6_11_29484.txt (M = " << M << ")\n";

    // Compute exact pairwise Hamming distance distribution across all C(29484, 2) = 434,638,146 pairs!
    long long dist_hist[29] = {0};
    #pragma omp parallel
    {
        long long local_hist[29] = {0};
        #pragma omp for schedule(dynamic, 128)
        for (int i = 0; i < M; ++i) {
            uint32_t u = code_words[i];
            for (int j = i + 1; j < M; ++j) {
                int d = __builtin_popcount(u ^ code_words[j]);
                local_hist[d]++;
            }
        }
        #pragma omp critical
        {
            for (int d = 0; d < 29; ++d) dist_hist[d] += local_hist[d];
        }
    }
    long long total_pairs = 0;
    cout << "\nExact Pairwise Hamming Distance Spectrum of A(28,6,11) = 29,484:\n";
    for (int d = 0; d < 29; ++d) {
        if (dist_hist[d] > 0) {
            total_pairs += dist_hist[d];
            cout << "  d = " << d << ": " << dist_hist[d] << " pairs ("
                 << (100.0 * dist_hist[d] / (1LL * M * (M - 1) / 2)) << "%)\n";
        }
    }
    cout << "Total pairs verified: " << total_pairs << " (expected " << (1LL * M * (M - 1) / 2) << ")\n";

    vector<int> coord_freq(28, 0);
    for (uint32_t w : code_words) {
        for (int b = 0; b < 28; ++b) if (w & (1u << b)) coord_freq[b]++;
    }
    cout << "Coordinate 1-frequencies across 0..27: min = "
         << *min_element(coord_freq.begin(), coord_freq.end())
         << ", max = " << *max_element(coord_freq.begin(), coord_freq.end()) << "\n";

    return 0;
}
