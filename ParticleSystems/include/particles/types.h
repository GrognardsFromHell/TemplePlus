
#pragma once

#include <stdint.h>
#include <DirectXMath.h>

typedef uint64_t ObjHndl;
typedef DirectX::XMFLOAT2 Vec2;
typedef DirectX::XMFLOAT3 Vec3;
typedef DirectX::XMFLOAT4 Vec4;
typedef DirectX::XMFLOAT4X4 Matrix4x4;

struct Box2d {
	float left = 0;
	float top = 0;
	float right = 0;
	float bottom = 0;

	Vec2 GetTopLeft() const {
		return Vec2(left, top);
	}

	Vec2 GetTopRight() const {
		return Vec2(right, top);
	}

	Vec2 GetBottomLeft() const {
		return Vec2(left, bottom);
	}

	Vec2 GetBottomRight() const {
		return Vec2(right, bottom);
	}

	Vec2 GetCenter() const {
		return Vec2((left + right) / 2.0f,
			(top + bottom) / 2.0f);
	}
};


