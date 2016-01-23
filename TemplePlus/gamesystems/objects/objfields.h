
#pragma once

#include <cstdint>
#include <functional>

#include "../../temple_enums.h"

enum class ObjectFieldType : uint32_t {
	None = 0,
	BeginSection = 1,
	EndSection = 2,
	Int32 = 3,
	Int64 = 4,
	AbilityArray = 5,
	UnkArray = 6, // Not used by anything in ToEE
	Int32Array = 7,
	Int64Array = 8,
	ScriptArray = 9,
	Unk2Array = 10, // Not used by anything in ToEE
	String = 11,
	Obj = 12,
	ObjArray = 13,
	SpellArray = 14,
	Float32 = 15
};

struct ObjectFieldDef {
	// Name of this field
	const char* name = nullptr;

	// The index of this field in the property 
	// collection of prototype objects
	int protoPropIdx = -1;

	// The idx of this array field (starting at 0 for the first array) or -1 if this is 
	// not an array field
	int arrayIdx = -1;

	// The idx of the bitmap 32-bit block that contains the bit indicating whether
	// an object instance has this field or not
	int bitmapBlockIdx = -1;

	// The bit within the bitmap block identified by bitmapBlockIdx
	int bitmapBitIdx = 0;

	// A bitmask to easily test for the bit identified by bitmapBitIdx
	uint32_t bitmapMask = 0;
	
	// Number of entries in the properties collection for this field
	size_t storedInPropColl = 0;

	ObjectFieldType type = ObjectFieldType::None;

};

class ObjectFields {
public:
	ObjectFields();

	const ObjectFieldDef &GetFieldDef(obj_f field) const {
		return mFieldDefs[field];
	}

	const std::array<ObjectFieldDef, 430> &GetFieldDefs() const {
		return mFieldDefs;
	}

	bool DoesTypeSupportField(ObjectType type, obj_f field);

	const char *GetFieldName(obj_f field) const;

	// Gets the type of the given field
	ObjectFieldType GetType(obj_f field) const {
		return mFieldDefs[field].type;
	}

	bool IsTransient(obj_f field) const {
		return field > obj_f_transient_begin && field < obj_f_transient_end;
	}

	/**
	 * Returns the number of bitmap blocks needed for the given type.
	 */
	size_t GetBitmapBlockCount(ObjectType type) const {
		return mBitmapBlocksPerType[type];
	}

	/**
	 * Returns the number of properties supported by the given type.
	 */
	size_t GetSupportedFieldCount(ObjectType type) const {
		return mPropCollSizePerType[type];
	}

	bool IterateTypeFields(ObjectType type, std::function<bool(obj_f)> callback);
	
	bool IsArrayType(ObjectFieldType type);

private:
	std::array<ObjectFieldDef, 430> mFieldDefs;

	// The number of bitmap blocks needed to be allocated by type
	std::array<size_t, ObjectTypeCount> mBitmapBlocksPerType;

	// The number of possible property storage locations per type
	std::array<size_t, ObjectTypeCount> mPropCollSizePerType;

	const char *GetTypeName(ObjectFieldType type);

	static size_t GetPropCollSize(obj_f field, ObjectFieldType type);
			
	bool IterateFieldRange(obj_f rangeStart, obj_f rangeEnd, std::function<bool(obj_f)> callback);
};

extern ObjectFields objectFields;
