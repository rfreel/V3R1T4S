# A(28, 6, 11): a 29,484-word construction

**Status: research material, not a new V3R1T4S result.** The supplied words form a valid binary constant-weight code with 29,484 words. This equals the lower bound in [Brouwer's table](https://aeb.win.tue.nl/codes/Andw.html), attributed there to Nurmela, Kaikkonen and Östergård. The table also links an [earlier construction](https://aeb.win.tue.nl/codes/cwc/d6/i28.11). The submitted words have not been checked for equivalence to that construction.

The benchmark's [constant-weight-code criteria](https://github.com/elder-plinius/V3R1T4S/blob/cf68067f5251de248b538749e1ccc1138da48841/known-unknowns/record-tables/constant-weight-codes.md) require an improvement over the live lower bound, or an equal-size first explicit certificate for a code without a surviving construction. This package establishes neither. It does not improve the known lower bound, provide a new upper bound, or establish the exact value of A(28, 6, 11). The table gives the upper bound 104,230.

## Checks performed

- The included C++ verifier passed all 434,638,386 word pairs, reporting minimum distance 6.
- `verify_independent.py` checked the length, weight and uniqueness of every word; it separately tested all 1,621,620 nine-coordinate subsets for collisions. Two weight-11 words have distance below 6 exactly when they share at least nine coordinates.
- The independent script also counted every coordinate triple and found each in exactly 1,485 words. This verifies the supplied code's 3-(28, 11, 1485) design property.
- These checks do **not** establish the group-theoretic, optimality, uniqueness, maximality, or novelty assertions in the supplied report. The file `supplied/how-we-built-it/audit_psl27_w11.cpp` did not compile as supplied because `<array>` is missing. It also contains concurrent writes to shared entries in its maximality scan. Two other programs cited by the report are absent.

## Reproduce

Requires Python 3.10+, a C++ compiler and OpenMP. From this directory:

```sh
python3 verify_independent.py supplied/object/A28_6_11_29484_hex.txt
(cd supplied && OMP_NUM_THREADS=8 sh verify-it-yourself/run_checks.sh)
sha256sum -c SHA256SUMS
```

The independent checker ends with `PASS`. The supplied verifier also ends with `PASS`. The additional group and maximality claims have not been independently reproduced here.

## Supplied material

`supplied/` preserves the object, two C++ sources, contributor report, index and supplied panel notes. The notes are historical assertions from the archive, not newly conducted or authenticated independent reviews; the gate's `CONFIRMED_SOLUTION` assertion is not adopted here. The supplied report includes statements of equality for A(28, 6, 11) that are not supported by the checks above.

The archive's purported table snapshot concerns A(27, 6, 10), not this code. It and the copied third-party HTML have been excluded. The archive's original checksum manifest is consequently not applicable; `SHA256SUMS` covers the files provided here. Original ZIP SHA-256: `4ccdf6acbf35eb8afb1f1ffa9d445c218ffa5ecfb678070c73a321cde2539723`.

Credits: Nurmela, Kaikkonen and Östergård for the earlier lower bound, and Andries E. Brouwer for maintaining the table and hosting the earlier construction. Discovery provenance for the supplied object has not been independently established.
