
#pragma once

#include <string>
#include <memory>
#include <type_traits>
#include <functional>

namespace temple {

	/*
	Utility to validate the size of structures.
	*/
	template <typename Type, size_t ExpectedSize, size_t ActualSize = 0>
	struct validate_size : std::true_type {
		static_assert(ActualSize == ExpectedSize, "Structure does not have required size!");
	};
	template <typename Type, size_t ExpectedSize>
	struct validate_size<Type, ExpectedSize, 0> : validate_size<Type, ExpectedSize, sizeof(Type)> {
	};
		
	class Dll {
	friend class DllImpl;
	public:
		Dll(Dll&) = delete;
		Dll(Dll&&) = delete;
		Dll& operator=(const Dll&) = delete;
		Dll& operator=(const Dll&&) = delete;

		~Dll();

		void* GetAddress(uint32_t vanillaAddress) const;

		void Load(const std::wstring &installationPath);
		void Unload();

		bool HasBeenRebased();
		std::wstring FindConflictingModule();
		void ReserveMemoryRange();

		static Dll& GetInstance();

		static void RegisterAddressPtr(void** ref);

		void SetDebugOutputCallback(std::function<void(const std::string &text)> callback);
				
	private:
		std::shared_ptr<class DllImpl> mImpl;
		void *mReservedMem;

		Dll() {}
	};

	struct AddressTable
	{
		AddressTable() {}
		
	protected:
		template<typename T>
		void rebase(T &ref, uint32_t offset) {
			ref = reinterpret_cast<T>(offset);
			Dll::RegisterAddressPtr(reinterpret_cast<void**>(&ref));
		}

		AddressTable(const AddressTable &) = delete;
		AddressTable(const AddressTable &&) = delete;
		AddressTable &operator =(const AddressTable &) = delete;
		AddressTable &operator =(AddressTable &&) = delete;
	};

	template <typename T, uint32_t offsetPreset>
	struct GlobalStruct
	{
		GlobalStruct() : mPtr(reinterpret_cast<T*>(offsetPreset))
		{
			static_assert(offsetPreset != 0, "This constructor should only be used with a template argument offset");
			Dll::RegisterAddressPtr(reinterpret_cast<void**>(&mPtr));
		}

		operator T*()
		{
			return mPtr;
		}

		T* operator ->()
		{
			return mPtr;
		}

		T* ptr()
		{
			return mPtr;
		}

	private:
		T* mPtr;
	};

	template <typename T, uint32_t offsetPreset>
	struct GlobalPrimitive
	{
		GlobalPrimitive() : mPtr(reinterpret_cast<T*>(offsetPreset))
		{
			assert(mPtr != nullptr);
			Dll::RegisterAddressPtr(reinterpret_cast<void**>(&mPtr));
		}

		T operator =(T value)
		{
			return (*mPtr = value);
		}

		operator T()
		{
			return *mPtr;
		}

		T* ptr()
		{
			return mPtr;
		}

		GlobalPrimitive & operator =(const GlobalPrimitive &) = delete;
		GlobalPrimitive(const GlobalPrimitive &) = delete;
	private:
		T* mPtr;
	};

	template<uint32_t offset> using GlobalBool = GlobalPrimitive<bool, offset>;
	
	// As long as we're replacing malloc/free in temple.dll, this struct 
	// does not need to override operator new/delete
	struct TempleAlloc {
	};


	// Get's a pointer into DLL memory accounting for any offset from vanilla addresses 
	// that might've occured thanks to DLL rebasing
	template <uint32_t address, typename T = void>
	inline T* GetPointer()
	{
		static_assert(address > 0x10000000 && address < 0x20000000,
			"address is not within temple.dll address space");
		return Dll::GetInstance().GetAddress(address);
	}

	template <typename T = void>
	inline T* GetPointer(uint32_t address)
	{
		assert(address > 0x10000000 && address < 0x20000000);
		return (T*) Dll::GetInstance().GetAddress(address);
	}

	// Get's a pointer into DLL memory accounting for any offset from vanilla addresses 
	// that might've occured thanks to DLL rebasing
	template <uint32_t address, typename T>
	inline T& GetRef()
	{
		static_assert(address > 0x10000000 && address < 0x20000000,
			"address is not within temple.dll address space");
		return *(T*)Dll::GetInstance().GetAddress(address);
	}

	template <typename T>
	inline T& GetRef(uint32_t address)
	{
		assert(address > 0x10000000 && address < 0x20000000);
		return *(T*)Dll::GetInstance().GetAddress(address);
	}

	template<typename T>
	inline void WriteMem(uint32_t address, T value) {
		GetRef<T>(address) = value;
	}
	
	template<uint32_t address, typename T>
	inline void WriteMem(T value) {
		GetRef<address, T>() = value;
	}

}
