
// EASTL needs these

void* operator new[](size_t size, const char* /*name*/, int /*flags*/,
	unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return new char[size];
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* /*name*/,
	int flags, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return new char[size];
}
