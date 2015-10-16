
#include <vector>

#include "platform/windows.h"

#include <Bcrypt.h>
#pragma comment(lib, "bcrypt")

#include "infrastructure/crypto.h"
#include "infrastructure/exception.h"

namespace crypto {

	namespace {

		/*
			Returns a cached handle to a BCRYPT provider for MD5
		*/
		BCRYPT_ALG_HANDLE GetMd5Alg() {
			static BCRYPT_ALG_HANDLE hAlg = nullptr;

			if (!hAlg) {
				auto status = BCryptOpenAlgorithmProvider(
					&hAlg,
					BCRYPT_MD5_ALGORITHM,
					NULL,
					0);

				if (status) {
					throw TempleException("Unable to create a MD5 hashing object. Returned error: {:x}", hAlg);
				}

				// Close the provider when we shut down
				atexit([]() {
					BCryptCloseAlgorithmProvider(hAlg, 0);
				});
			}

			return hAlg;
		}

		std::string RawHashToStr(const std::vector<uint8_t> &hash) {
			// Convert vector to lower case hex string
			fmt::MemoryWriter result;
			for (auto octet : hash) {
				result << fmt::pad(fmt::hex(octet), 2, '0');
			}
			return result.str();
		}

	}

	std::string MD5AsString(gsl::array_view<uint8_t> data) {

		auto alg = GetMd5Alg();

		BCRYPT_HASH_HANDLE hash;
		NTSTATUS status;

		//calculate the length of the hash
		DWORD hashLength, cbData;
		if ((status = BCryptGetProperty(alg, BCRYPT_HASH_LENGTH, (PBYTE)&hashLength, sizeof(DWORD), &cbData, 0))) {
			throw TempleException("Unable to determine how long the hash value of MD5 hashes is");
		}

		status = BCryptCreateHash(alg, &hash, nullptr, 0, nullptr, 0, 0);
		if (status) {
			throw TempleException("Unable to create a MD5 hasher: {:x}", status);
		}

		status = BCryptHashData(hash, data.data(), data.bytes(), 0);
		if (status) {
			BCryptDestroyHash(hash);
			throw TempleException("Unable to create MD5 hash: {:x}", status);
		}

		std::vector<uint8_t> hashValue(hashLength, 0);
		status = BCryptFinishHash(hash, &hashValue[0], hashValue.size(), 0);
		if (status) {
			BCryptDestroyHash(hash);
			throw TempleException("Unable to compute the final hash value: {:x}", status);
		}

		BCryptDestroyHash(hash); // We can't really do anything about errors here...

		return RawHashToStr(hashValue);

	}

}
