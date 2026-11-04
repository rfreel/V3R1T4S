#!/usr/bin/env python3
"""Verify a fixed A(28,6,11) construction by subset collisions."""
import itertools
import re
import sys
from collections import Counter
from pathlib import Path


def verify(path):
    words = []
    for lineno, line in enumerate(path.read_text(encoding="ascii").splitlines(), 1):
        if not re.fullmatch(r"[0-9a-fA-F]{1,7}", line):
            raise ValueError(f"line {lineno}: expected a 28-bit hexadecimal word")
        word = int(line, 16)
        if word.bit_count() != 11:
            raise ValueError(f"line {lineno}: expected weight 11")
        words.append(word)
    if len(words) != 29484:
        raise ValueError(f"expected 29484 words; got {len(words)}")
    if len(set(words)) != len(words):
        raise ValueError("duplicate codeword")

    subsets = set()
    triples = Counter()
    for word in words:
        points = [i for i in range(28) if word & (1 << i)]
        triples.update(itertools.combinations(points, 3))
        for nine in itertools.combinations(points, 9):
            mask = sum(1 << i for i in nine)
            if mask in subsets:
                raise ValueError("two codewords share at least nine coordinates")
            subsets.add(mask)
    if len(subsets) != 29484 * 55:
        raise ValueError("unexpected number of 9-subsets")
    if len(triples) != 3276 or set(triples.values()) != {1485}:
        raise ValueError("triple incidences do not form the claimed 3-design")
    print(f"words={len(words)} unique_9_subsets={len(subsets)}")
    print(f"coordinate_triples={len(triples)} incidence=1485")
    print("PASS")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: python3 verify_independent.py PATH")
    try:
        verify(Path(sys.argv[1]))
    except (OSError, UnicodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
