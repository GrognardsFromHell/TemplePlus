
#pragma once

#include "math.h"

struct Ray3d {
	XMFLOAT3 origin;
	XMFLOAT3 direction;
};

struct Cylinder3d {
	XMFLOAT3 baseCenter;
	float radius;
	float height;

	bool HitTest(const Ray3d &ray, float &distance) const;
};

inline bool Cylinder3d::HitTest(const Ray3d & ray, float &distance) const
{
	using namespace DirectX;

	float t;
	// Intersect segment S(t)=sa+t(sb-sa), 0<=t<=1 against cylinder specifiedby p, q and r
	// int IntersectSegmentCylinder(Point sa, Point sb, Point p, Point q, float r, float &t)

	auto sa = XMLoadFloat3(&ray.origin);
	auto sb = sa + XMLoadFloat3(&ray.direction);
	auto rayLength = XMVectorGetX(XMVector3Length(XMLoadFloat3(&ray.direction)));

	auto p = XMLoadFloat3(&baseCenter);
	
	// Fix issues with degenerated cylinders (height=0)
	auto effectiveHeight = 1.0f;
	if (height > effectiveHeight)
	{
		effectiveHeight = height;
	}
	auto q = p + XMVectorSet(0, effectiveHeight, 0, 1);

	auto d = q - p;
	auto m = sa - p;
	auto n = sb - sa;
	float md = XMVectorGetX(XMVector3Dot(m, d));
	float nd = XMVectorGetX(XMVector3Dot(n, d));
	float dd = XMVectorGetX(XMVector3Dot(d, d));
	// Test if segment fully outside either endcap of cylinder
	if (md < 0.0f && md + nd < 0.0f) return 0; // Segment outside ’p’ side of cylinder
	if (md > dd && md + nd > dd) return 0; // Segment outside ’q’ side of cylinder
	float nn = XMVectorGetX(XMVector3Dot(n, n));
	float mn = XMVectorGetX(XMVector3Dot(m, n));
	float a = dd * nn - nd * nd;
	float k = XMVectorGetX(XMVector3Dot(m, m)) - radius * radius;
	float c = dd * k - md * md;
	if (fabsf(a) < 0.0001f) {
		// Segment runs parallel to cylinder axis
		if (c > 0.0f) return 0; // ’a’ and thus the segment lie outside cylinder
								// Now known that segment intersects cylinder; figure out how it intersects
		if (md < 0.0f) t = -mn / nn; // Intersect segment against ’p’ endcap
		else if (md > dd) t = (nd - mn) / nn; // Intersect segment against ’q’ endcap
		else t = 0.0f; // ’a’ lies inside cylinder
		distance = t * rayLength;
		return 1;
	}
	float b = dd * mn - nd * md;
	float discr = b * b - a * c;
	if (discr < 0.0f) return 0; // No real roots; no intersection
	t = (-b - sqrtf(discr)) / a;
	if (t < 0.0f || t > 1.0f) return 0; // Intersection lies outside segment
	if (md + t * nd < 0.0f) {
		// Intersection outside cylinder on ’p’ side
		if (nd <= 0.0f) return 0; // Segment pointing away from endcap
		t = -md / nd;
		distance = t * rayLength;
		// Keep intersection if Dot(S(t) - p, S(t) - p) <= r ? 2
		return k + 2 * t * (mn + t * nn) <= 0.0f;
	}
	else if (md + t * nd > dd) {
		// Intersection outside cylinder on ’q’ side
		if (nd >= 0.0f) return 0; // Segment pointing away from endcap
		t = (dd - md) / nd;
		distance = t * rayLength;
		// Keep intersection if Dot(S(t) - q, S(t) - q) <= r ? 2
		return k + dd - 2 * md + t * (2 * (mn - nd) + t * nn) <= 0.0f;
	}
	distance = t * rayLength;
	// Segment intersects cylinder between the endcaps; t is correct
	return true;
}
