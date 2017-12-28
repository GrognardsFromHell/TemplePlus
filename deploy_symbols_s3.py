
import sys
import os.path
import lzma
import time
import os
import subprocess

if "APPVEYOR" in os.environ and "APPVEYOR_REPO_TAG" not in os.environ:
    print("Not uploading symbols since we're not building a tag")
    sys.exit(0)

symbol_filename = sys.argv[1]

with open(symbol_filename, "rt") as fh:
    first_line = next(fh).strip()

tokens = first_line.split()
expected_tokens = ['MODULE', 'windows', 'x86']
if tokens[0:3] != expected_tokens:
    raise RuntimeError("Expected first tokens to be " + str(expected_tokens) + ", but was: " + str(tokens[0:3]))

file_hash = tokens[3]
file_name = tokens[4]

basename = os.path.basename(symbol_filename)

target_path = "%s/%s/%s.xz" % (file_name, file_hash, basename)

# Compress symbols with LZMA to save bandwidth
print("Compressing symbol file...")
t_start = time.perf_counter()
with open(symbol_filename, "rb") as fh:
    symbol_data = fh.read()
    symbol_data_len = len(symbol_data)
    compressed_symbols = lzma.compress(symbol_data)
compression_ratio = len(compressed_symbols) * 100 / symbol_data_len
print("Compressed symbol data (ratio %d%%) in %fs" % (compression_ratio, time.perf_counter() - t_start))

print("Uploading symbols to ", target_path)

with open("TemplePlus.sym.xz", "wb") as fh:
    fh.write(compressed_symbols)

subprocess.run(["aws", "s3", "cp", "TemplePlus.sym.xz", "s3://templeplus-symbols/" + target_path], check=True, shell=True)

print("Uploaded symbols to S3.")
