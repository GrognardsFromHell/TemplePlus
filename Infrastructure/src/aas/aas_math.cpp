
#include "aas/aas_math.h"

namespace aas {

	static Matrix4x4 sIdentity4x4 = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	const Matrix4x4 & Matrix4x4::identity()
	{
		return sIdentity4x4;
	}

	Matrix4x4 Matrix4x4::inverse() const
	{
		auto m = DX::XMMatrixTranspose(DX::XMLoadFloat4x4((DX::XMFLOAT4X4*)&m00));
		m = DX::XMMatrixTranspose(DX::XMMatrixInverse(nullptr, m));

		Matrix4x4 result = *this;
		DX::XMStoreFloat4x4((DX::XMFLOAT4X4*)&result.m00, m);
		return result;
	}

	static Matrix3x4 sIdentity3x4 = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	const Matrix3x4 & Matrix3x4::identity()
	{
		return sIdentity3x4;
	}

	Matrix3x4::operator Matrix4x4() const
	{
		Matrix4x4 result;
		for (int col = 0; col < Matrix3x4::cols; col++) {
			for (int row = 0; row < Matrix3x4::rows; row++) {
				result(row, col) = (*this)(row, col);
			}
		}
		result(0, 3) = 0;
		result(1, 3) = 0;
		result(2, 3) = 0;
		result(3, 3) = 1.0f;
		return result;
	}

}
