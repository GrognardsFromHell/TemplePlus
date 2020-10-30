#pragma once

#include <algorithm>
#include <DirectXMath.h>

#include <cmath>

namespace aas {

	namespace DX = DirectX;

	struct Matrix4x4 {
		// Indexing here is col-major
		float m00;
		float m01;
		float m02;
		float m03;
		float m10;
		float m11;
		float m12;
		float m13;
		float m20;
		float m21;
		float m22;
		float m23;
		float m30;
		float m31;
		float m32;
		float m33;

		static constexpr int rows = 4;
		static constexpr int cols = 4;

		static const Matrix4x4 &identity();

		const float *data() const {
			return &m00;
		}

		float *data() {
			return &m00;
		}

		Matrix4x4 inverse() const;

		float operator()(int row, int col) const {
			assert(row >= 0 && row < rows && col >= 0 && col < cols);
			return data()[col * rows + row];
		}

		float& operator()(int row, int col) {
			assert(row >= 0 && row < rows && col >= 0 && col < cols);
			return data()[col * rows + row];
		}

	};

	struct Matrix3x4 {
		// Indexing here is col-major
		float m00;
		float m01;
		float m02;
		float m03;
		float m10;
		float m11;
		float m12;
		float m13;
		float m20;
		float m21;
		float m22;
		float m23;

		static constexpr int rows = 4;
		static constexpr int cols = 3;

		static const Matrix3x4 &identity();

		operator Matrix4x4() const;

		const float *data() const {
			return &m00;
		}

		float *data() {
			return &m00;
		}

		float operator()(int row, int col) const {
			assert(row >= 0 && row < 4 && col >= 0 && col < 3);
			return data()[col * 4 + row];
		}

		float& operator()(int row, int col) {
			assert(row >= 0 && row < 4 && col >= 0 && col < 3);
			return data()[col * 4 + row];
		}

		Matrix3x4& operator =(const Matrix3x4& ref) {
			memcpy(this, &ref, sizeof(Matrix3x4));
			return *this;
		}

		bool operator==(const Matrix3x4 &o) const {
			return m00 == o.m00
				&& m01 == o.m01
				&& m02 == o.m02
				&& m03 == o.m03
				&& m10 == o.m10
				&& m11 == o.m11
				&& m12 == o.m12
				&& m13 == o.m13
				&& m20 == o.m20
				&& m21 == o.m21
				&& m22 == o.m22
				&& m23 == o.m23;
		}

		DX::XMFLOAT4X4 ToFloat4x4() const {
			DX::XMFLOAT4X4 result;
			for (int col = 0; col < cols; col++) {
				for (int row = 0; row < rows; row++) {
					result(row, col) = (*this)(row, col);
				}
			}
			result(0, 3) = 0;
			result(1, 3) = 0;
			result(2, 3) = 0;
			result(3, 3) = 1;
			return result;
		}

		static Matrix3x4 From4x4(const DX::XMFLOAT4X4& ref);
	};

	struct Quaternion {
		float x;
		float y;
		float z;
		float w;

		Quaternion() {
		}

		Quaternion(DX::XMFLOAT4 vector) {
			x = vector.x;
			y = vector.y;
			z = vector.z;
			w = vector.w;
		}
	};

	// Originally @ 102647F0
	inline Quaternion slerpQuaternion(const Quaternion &from, const Quaternion &to, float fraction) {

		auto fromV = DX::XMLoadFloat4((DX::XMFLOAT4*) &from);
		auto toV = DX::XMLoadFloat4((DX::XMFLOAT4*) &to);

		Quaternion out;
		auto qi = DX::XMQuaternionSlerp(fromV, toV, fraction);
		DX::XMStoreFloat4((DX::XMFLOAT4*)&out, qi);
		return out;
	}

	inline Quaternion quaternionAxisAngle(float x, float y, float z, float angle) {
		Quaternion q;
		angle /= 2.0f;
		float cos_angle = cos(angle);
		float sin_angle = sin(angle);

		q.x = x * sin_angle;
		q.y = y * sin_angle;
		q.z = z * sin_angle;
		q.w = cos_angle;
		return q;
	}

	inline Matrix3x4 identityMatrix() {
		Matrix3x4 r{};
		r.m00 = 1.0f;
		r.m11 = 1.0f;
		r.m22 = 1.0f;
		return r;
	}

	inline Matrix3x4 scaleMatrix(float x, float y, float z) {
		Matrix3x4 result{};
		result.m00 = x;
		result.m11 = y;
		result.m22 = z;
		return result;
	}

	inline Matrix3x4 translationMatrix(float x, float y, float z) {
		Matrix3x4 result = identityMatrix();
		result.m03 = x;
		result.m13 = y;
		result.m23 = z;
		return result;
	}

	inline Matrix3x4 multiplyMatrix3x3(const Matrix3x4 &a, const Matrix3x4 &b) {

		Matrix3x4 r;
		r.m00 = a.m00 * b.m00 + b.m02 * a.m20 + b.m01 * a.m10;
		r.m01 = a.m21 * b.m02 + b.m00 * a.m01 + a.m11 * b.m01;
		r.m02 = a.m12 * b.m01 + b.m02 * a.m22 + a.m02 * b.m00;
		r.m10 = b.m12 * a.m20 + a.m10 * b.m11 + b.m10 * a.m00;
		r.m11 = a.m11 * b.m11 + b.m10 * a.m01 + b.m12 * a.m21;
		r.m12 = b.m12 * a.m22 + a.m02 * b.m10 + a.m12 * b.m11;
		r.m20 = b.m21 * a.m10 + b.m20 * a.m00 + b.m22 * a.m20;
		r.m21 = b.m21 * a.m11 + b.m20 * a.m01 + b.m22 * a.m21;
		r.m22 = b.m22 * a.m22 + b.m21 * a.m12 + a.m02 * b.m20;
		return r;

	}

	inline Matrix3x4 multiplyMatrix3x3_3x4(const Matrix3x4 &a, const Matrix3x4 &b) {

		Matrix3x4 r;
		r.m00 = a.m00 * b.m00 + b.m02 * a.m20 + b.m01 * a.m10;
		r.m01 = a.m21 * b.m02 + b.m00 * a.m01 + a.m11 * b.m01;
		r.m02 = a.m12 * b.m01 + b.m02 * a.m22 + a.m02 * b.m00;
		r.m03 = b.m03;
		r.m10 = b.m11 * a.m10 + b.m10 * a.m00 + a.m20 * b.m12;
		r.m11 = b.m10 * a.m01 + a.m21 * b.m12 + a.m11 * b.m11;
		r.m12 = a.m12 * b.m11 + a.m02 * b.m10 + a.m22 * b.m12;
		r.m13 = b.m13;
		r.m20 = b.m21 * a.m10 + b.m20 * a.m00 + a.m20 * b.m22;
		r.m21 = b.m20 * a.m01 + a.m21 * b.m22 + a.m11 * b.m21;
		r.m22 = a.m12 * b.m21 + b.m20 * a.m02 + a.m22 * b.m22;
		r.m23 = b.m23;
		return r;

	}

	inline Matrix3x4 multiplyMatrix3x4_3x3(const Matrix3x4 &left, const Matrix3x4 &right) {

		Matrix3x4 r;
		r.m00 = left.m00 * right.m00 + right.m02 * left.m20 + right.m01 * left.m10;
		r.m01 = left.m21 * right.m02 + right.m00 * left.m01 + left.m11 * right.m01;
		r.m02 = left.m12 * right.m01 + right.m02 * left.m22 + left.m02 * right.m00;
		r.m03 = left.m03 * right.m00 + left.m23 * right.m02 + right.m01 * left.m13;
		r.m10 = right.m11 * left.m10 + right.m10 * left.m00 + right.m12 * left.m20;
		r.m11 = right.m11 * left.m11 + right.m10 * left.m01 + right.m12 * left.m21;
		r.m12 = right.m12 * left.m22 + right.m11 * left.m12 + left.m02 * right.m10;
		r.m13 = left.m23 * right.m12 + right.m11 * left.m13 + left.m03 * right.m10;
		r.m20 = right.m22 * left.m20 + right.m21 * left.m10 + right.m20 * left.m00;
		r.m21 = left.m11 * right.m21 + right.m20 * left.m01 + right.m22 * left.m21;
		r.m22 = right.m22 * left.m22 + left.m02 * right.m20 + left.m12 * right.m21;
		r.m23 = right.m20 * left.m03 + left.m23 * right.m22 + left.m13 * right.m21;
		return r;

	}

	inline Matrix3x4 multiplyMatrix3x4(const Matrix3x4 &left, const Matrix3x4 &right) {

		Matrix3x4 result;

		result(0, 0) = left(0, 0) * right(0, 0) + left(0, 1) * right(1, 0) + left(0, 2) * right(2, 0);
		result(1, 0) = left(1, 0) * right(0, 0) + left(1, 1) * right(1, 0) + left(1, 2) * right(2, 0);
		result(2, 0) = left(2, 0) * right(0, 0) + left(2, 1) * right(1, 0) + left(2, 2) * right(2, 0);
		result(3, 0) = left(3, 0) * right(0, 0) + left(3, 1) * right(1, 0) + left(3, 2) * right(2, 0) + right(3, 0);

		result(0, 1) = left(0, 0) * right(0, 1) + left(0, 1) * right(1, 1) + left(0, 2) * right(2, 1);
		result(1, 1) = left(1, 0) * right(0, 1) + left(1, 1) * right(1, 1) + left(1, 2) * right(2, 1);
		result(2, 1) = left(2, 0) * right(0, 1) + left(2, 1) * right(1, 1) + left(2, 2) * right(2, 1);
		result(3, 1) = left(3, 0) * right(0, 1) + left(3, 1) * right(1, 1) + left(3, 2) * right(2, 1) + right(3, 1);

		result(0, 2) = left(0, 0) * right(0, 2) + left(0, 1) * right(1, 2) + left(0, 2) * right(2, 2);
		result(1, 2) = left(1, 0) * right(0, 2) + left(1, 1) * right(1, 2) + left(1, 2) * right(2, 2);
		result(2, 2) = left(2, 0) * right(0, 2) + left(2, 1) * right(1, 2) + left(2, 2) * right(2, 2);
		result(3, 2) = left(3, 0) * right(0, 2) + left(3, 1) * right(1, 2) + left(3, 2) * right(2, 2) + right(3, 2);

		return result;

	}

	inline Matrix3x4 rotationMatrix(Quaternion q) {

		auto q_dx = DX::XMLoadFloat4((DX::XMFLOAT4*)&q);
		auto m_dx = DX::XMMatrixRotationQuaternion(q_dx);
		DX::XMFLOAT3X3 mat_dx;
		DX::XMStoreFloat3x3(&mat_dx, DX::XMMatrixTranspose(m_dx));
		Matrix3x4 matrix_dx;
		matrix_dx(0, 0) = mat_dx._11;
		matrix_dx(1, 0) = mat_dx._12;
		matrix_dx(2, 0) = mat_dx._13;
		matrix_dx(0, 1) = mat_dx._21;
		matrix_dx(1, 1) = mat_dx._22;
		matrix_dx(2, 1) = mat_dx._23;
		matrix_dx(0, 2) = mat_dx._31;
		matrix_dx(1, 2) = mat_dx._32;
		matrix_dx(2, 2) = mat_dx._33;

		for (int row = 0; row < 3; row++) {
			for (int col = 0; col < 3; col++) {
				auto &value = matrix_dx(row, col);
				if (fabs(value) < 0.0001) {
					value = 0.0f;
				}
				else if (fabs(value - 1.0) < 0.0001) {
					value = 1.0f;
				}
				else if (fabs(value + 1.0) < 0.0001) {
					value = -1.0f;
				}
			}
		}

		return matrix_dx;

	}

	inline DX::XMFLOAT4 transformPosition(const Matrix3x4 &matrix, const DX::XMFLOAT4 &vector) {
		DX::XMFLOAT4 result;
		result.x = matrix.m00 * vector.x + matrix.m01 * vector.y + matrix.m02 * vector.z + matrix.m03;
		result.y = matrix.m10 * vector.x + matrix.m11 * vector.y + matrix.m12 * vector.z + matrix.m13;
		result.z = matrix.m20 * vector.x + matrix.m21 * vector.y + matrix.m22 * vector.z + matrix.m23;
		result.w = vector.w;
		return result;
	}

	inline DX::XMFLOAT4 transformNormal(const Matrix3x4 &matrix, const DX::XMFLOAT4 &vector) {
		DX::XMFLOAT4 result;
		result.x = matrix.m02 * vector.z + matrix.m01 * vector.y + matrix.m00 * vector.x;
		result.y = matrix.m12 * vector.z + matrix.m11 * vector.y + matrix.m10 * vector.x;
		result.z = matrix.m22 * vector.z + matrix.m21 * vector.y + matrix.m20 * vector.x;
		result.w = vector.w;
		return result;
	}

	inline DX::XMFLOAT4 vectorMax(const DX::XMFLOAT4 &a, const DX::XMFLOAT4 &b) {
		DX::XMFLOAT4 result;
		result.x = std::max(a.x, b.x);
		result.y = std::max(a.y, b.y);
		result.z = std::max(a.z, b.z);
		result.w = std::max(a.w, b.w);
		return result;
	}

	inline DX::XMFLOAT4 vectorMin(const DX::XMFLOAT4 &a, const DX::XMFLOAT4 &b) {
		DX::XMFLOAT4 result;
		result.x = std::min(a.x, b.x);
		result.y = std::min(a.y, b.y);
		result.z = std::min(a.z, b.z);
		result.w = std::min(a.w, b.w);
		return result;
	}

	// Distance squared while disregarding w
	inline float distanceSquared3(const DX::XMFLOAT4 &a, const DX::XMFLOAT4 &b) {
		float deltaX = (a.x - b.x);
		float deltaY = (a.y - b.y);
		float deltaZ = (a.z - b.z);
		return deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
	}

	// This is a fast inverse if the src matrix is an orthogonal affine transformation matrix
	inline Matrix3x4 invertOrthogonalAffineTransform(const Matrix3x4 &src) {
		Matrix3x4 result;
		result.m00 = src.m00;
		result.m10 = src.m01;
		result.m20 = src.m02;
		result.m01 = src.m10;
		result.m11 = src.m11;
		result.m21 = src.m12;
		result.m02 = src.m20;
		result.m12 = src.m21;
		result.m22 = src.m22;
		result.m03 = -(src.m23 * src.m20 + src.m13 * src.m10 + src.m03 * src.m00);
		result.m13 = -(src.m23 * src.m21 + src.m13 * src.m11 + src.m03 * src.m01);
		result.m23 = -(src.m23 * src.m22 + src.m13 * src.m12 + src.m03 * src.m02);
		return result;
	}

	inline Matrix3x4 makeMatrixOrthogonal(Matrix3x4 &matrix) {
		for (auto i = 0; i < Matrix3x4::cols; i++) {
			for (auto j = 0; j < i; j++)
			{
				auto f = 0.0f;

				for (int k = 0; k < Matrix3x4::cols; k++) {
					f += matrix(k, i) * matrix(k, j);
				}

				for (int k = 0; k < Matrix3x4::cols; k++) {
					matrix(k, i) -= f * matrix(k, j);
				}
			}

			// Apparently this normalizes the columns
			auto lenSquared = 0.0f;
			for (auto j = 0; j < Matrix3x4::cols; j++) {
				auto v = matrix(j, i);
				lenSquared += v * v;
			}
			auto factor = 1.0f / sqrtf(lenSquared);

			for (auto j = 0; j < Matrix3x4::cols; j++) {
				matrix(j, i) *= factor;
			}
		}

		return matrix;
	}

}
