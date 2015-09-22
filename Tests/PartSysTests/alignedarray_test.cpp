#include "stdafx.h"

#include <particles/alignedarray.h>

TEST(AlignedArrayTest, TestCapacityRounding) {
	for (auto i = 1; i <= 4; ++i) {
		AlignedFloatArray a(i);
		ASSERT_EQ(4, a.GetCapacity());
	}
	for (auto i = 5; i <= 8; ++i) {
		AlignedFloatArray a(i);
		ASSERT_EQ(8, a.GetCapacity());
	}
}

TEST(AlignedArrayTest, TestSettingAndGetting) {
	AlignedFloatArray a(10);
	a[0] = 123;
	ASSERT_EQ(123, a[0]);
}

TEST(AlignedArrayTest, TestAdd) {
	AlignedFloatArray a(10000);
	AlignedFloatArray b(10000);
	a.Fill(0, 10000, 1234);
	b.Fill(0, 10000, 4321);
	
	a.Add(0, 5000, b);
	b.Add(5000, 10000, a);

	for (int i = 0; i < 5000; ++i) {
		ASSERT_EQ(5555, a[i]);		
	}
	for (int i = 5000; i < 10000; ++i) {
		ASSERT_EQ(5555, b[i]);
	}
	
}
