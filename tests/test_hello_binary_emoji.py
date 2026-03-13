#!/usr/bin/env python3
"""
Test: transcode "Hello, World!" (raw bytes) → binary using emoji digits.

src alphabet: raw bytes (default)
dst alphabet: base2-alphabet.txt  (0️⃣ = 0, 1️⃣ = 1)
"""

import os
import subprocess
import tempfile

ROOT      = os.path.join(os.path.dirname(__file__), "..")
TRANSCODE = os.path.join(ROOT, "build", "transcode")
BASE2     = os.path.join(ROOT, "base2-alphabet.txt")

EMOJI = ["0️⃣", "1️⃣"]

def expected(data: bytes) -> str:
    n = int.from_bytes(data, "big")
    return "".join(EMOJI[int(b)] for b in bin(n)[2:])  # bin() strips leading zeros


def test():
    input_data = b"Hello, World!"

    with tempfile.NamedTemporaryFile(delete=False) as f:
        f.write(input_data)
        in_path = f.name
    out_path = in_path + ".out"

    try:
        result = subprocess.run(
            [TRANSCODE, "-d", BASE2, in_path, out_path],
            capture_output=True,
        )
        assert result.returncode == 0, f"transcode exited {result.returncode}: {result.stderr.decode()}"

        with open(out_path, "r", encoding="utf-8") as f:
            actual = f.read()

        want = expected(input_data)
        assert actual == want, (
            f"mismatch\n"
            f"  want ({len(want)} chars): {want[:80]}...\n"
            f"  got  ({len(actual)} chars): {actual[:80]}..."
        )

        print("PASS")

    finally:
        os.unlink(in_path)
        if os.path.exists(out_path):
            os.unlink(out_path)


if __name__ == "__main__":
    test()
