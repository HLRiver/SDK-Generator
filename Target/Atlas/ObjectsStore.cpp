#include "ObjectsStore.hpp"
#include "PatternFinder.hpp"
#include "EngineClasses.hpp"

class FUObjectItem
{
public:
	UObject* Object;
	int32_t SerialNumber;
};

class FChunkedFixedUObjectArray
{
public:
	inline int32_t Num() const
	{
		return NumElements;
	}

	enum
	{
		NumElementsPerChunk = 64 * 1024,
	};

	inline FUObjectItem const* GetObjectPtr(int32_t Index) const
	{
		const int32_t ChunkIndex = Index / NumElementsPerChunk;
		const int32_t WithinChunkIndex = Index % NumElementsPerChunk;
		const auto Chunk = Objects[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	inline FUObjectItem const& GetByIndex(int32_t Index) const
	{
		return *GetObjectPtr(Index);
	}

private:
	FUObjectItem** Objects;
	FUObjectItem* PreAllocatedObjects;
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;
};

class FUObjectArray
{
public:
	int32_t ObjFirstGCIndex;
	int32_t ObjLastNonGCIndex;
	int32_t MaxObjectsNotConsideredByGC;
	int32_t OpenForDisregardForGC;
	FChunkedFixedUObjectArray ObjObjects;
};

FUObjectArray* GlobalObjects = nullptr;

bool ObjectsStore::Initialize()
{
	const auto address = FindPattern(GetModuleHandleW(nullptr), reinterpret_cast<const unsigned char*>("\x48\x8D\x05\x00\x00\x00\x00\x8B\x40"), "xxx????xx");
	GlobalObjects = reinterpret_cast<decltype(GlobalObjects)>(address + 7 + *reinterpret_cast<uint32_t*>(address + 3));

	return true;
}

void* ObjectsStore::GetAddress()
{
	return GlobalObjects;
}

size_t ObjectsStore::GetObjectsNum() const
{
	return GlobalObjects->ObjObjects.Num();
}

UEObject ObjectsStore::GetById(size_t id) const
{
	return GlobalObjects->ObjObjects.GetByIndex(id).Object;
}
