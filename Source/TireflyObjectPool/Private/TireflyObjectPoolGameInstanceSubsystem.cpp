// Copyright Tirefly. All Rights Reserved.


#include "TireflyObjectPoolGameInstanceSubsystem.h"

#include "TireflyObjectPoolLogChannels.h"
#include "TireflyPoolingObjectInterface.h"


void UTireflyObjectPoolGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTireflyObjectPoolGameInstanceSubsystem::Deinitialize()
{
	ClearAllObjectPools();
	Super::Deinitialize();
}

UObject* UTireflyObjectPoolGameInstanceSubsystem::FetchObjectFromPool(
	const TSubclassOf<UObject>& ObjectClass,
	FName ObjectId)
{
	if (!ObjectClass)
	{
		UE_LOG(LogTireflyObjectPool, Error, TEXT("[%s] Invalid ObjectClass"), *FString(__FUNCTION__));
		return nullptr;
	}

	if (FTireflyObjectPool* Pool = (ObjectId != NAME_None) ? ObjectPoolOfId.Find(ObjectId) : ObjectPoolOfClass.Find(ObjectClass))
	{
		return Pool->ObjectPool.IsEmpty() ? nullptr : Pool->ObjectPool.Pop();
	}

	return nullptr;
}

UObject* UTireflyObjectPoolGameInstanceSubsystem::GetObjectFromPool_Internal(
	const TSubclassOf<UObject>& ObjectClass,
	FName ObjectId,
	const FInstancedStruct* PropertyData)
{
	if (!ObjectClass)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] Invalid ObjectClass"), *FString(__FUNCTION__));
		return nullptr;
	}

	if (!ObjectClass->ImplementsInterface(UTireflyPoolingObjectInterface::StaticClass()))
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] ObjectClass %s does not implements UTireflyPoolingObjectInterface"),
			*FString(__FUNCTION__),
			*ObjectClass->GetName());
		return nullptr;
	}

	FScopeLock Lock(&PoolLock);

	UObject* Object = FetchObjectFromPool(ObjectClass, ObjectId);
	if (!Object)
	{
		Object = NewObject<UObject>(this, ObjectClass);
		if (!Object)
		{
			UE_LOG(LogTireflyObjectPool, Error, TEXT("[%s] Failed to create object %s"),
				*FString(__FUNCTION__),
				*ObjectClass->GetName());
			return nullptr;
		}

		if (ObjectId != NAME_None)
		{
			ITireflyPoolingObjectInterface::Execute_PoolingSetObjectId(Object, ObjectId);
		}
	}
	
	ITireflyPoolingObjectInterface::Execute_PoolingInitialize(Object);
	if (PropertyData)
	{
		ITireflyPoolingObjectInterface::Execute_PoolingResetProperty(Object, *PropertyData);
	}

	return Object;
}

void UTireflyObjectPoolGameInstanceSubsystem::RecycleObjectToPool(UObject* Object)
{
	if (!IsValid(Object))
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] Invalid Object"), *FString(__FUNCTION__));
		return;
	}

	FScopeLock Lock(&PoolLock);

	FName ObjectId = NAME_None;
	if (Object->Implements<UTireflyPoolingObjectInterface>())
	{
		ObjectId = ITireflyPoolingObjectInterface::Execute_PoolingGetObjectId(Object);
		ITireflyPoolingObjectInterface::Execute_PoolingCleanup(Object);
	}

	if (ObjectId != NAME_None)
	{
		FTireflyObjectPool& Pool = ObjectPoolOfId.FindOrAdd(ObjectId);
		Pool.ObjectPool.Push(Object);

		return;
	}
	
	FTireflyObjectPool& Pool = ObjectPoolOfClass.FindOrAdd(Object->GetClass());
	Pool.ObjectPool.Push(Object);
}

void UTireflyObjectPoolGameInstanceSubsystem::WarmUpObjectPool(
	TSubclassOf<UObject> ObjectClass,
	FName ObjectId,
	int32 Count)
{
	if (!ObjectClass)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] Invalid ObjectClass"), *FString(__FUNCTION__));
		return;
	}

	if (!ObjectClass->ImplementsInterface(UTireflyPoolingObjectInterface::StaticClass()))
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] ObjectClass %s does not implements UTireflyPoolingObjectInterface"),
			*FString(__FUNCTION__),
			*ObjectClass->GetName());
		return;
	}

	if (Count <= 0)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] Count must be greater than 0"), *FString(__FUNCTION__));
		return;
	}

	FScopeLock Lock(&PoolLock);

	FTireflyObjectPool& Pool = ObjectId != NAME_None ? ObjectPoolOfId.FindOrAdd(ObjectId) :	ObjectPoolOfClass.FindOrAdd(ObjectClass);
	Pool.ObjectPool.Reserve(Pool.ObjectPool.Num() + Count);
    
	for (int32 i = 0; i < Count; i++)
	{
		UObject* Object = NewObject<UObject>(this, ObjectClass);
		if (!IsValid(Object))
		{
			UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] Failed to create object %s"),
				*FString(__FUNCTION__),
				*Object->GetName());
			continue;
		}
        
		if (Object->Implements<UTireflyPoolingObjectInterface>())
		{
			if (ObjectId != NAME_None)
			{
				ITireflyPoolingObjectInterface::Execute_PoolingSetObjectId(Object, ObjectId);
			}
			ITireflyPoolingObjectInterface::Execute_PoolingCleanup(Object);
		}
        
		Pool.ObjectPool.Push(Object);
	}
}

void UTireflyObjectPoolGameInstanceSubsystem::ClearAllObjectPools()
{
	FScopeLock Lock(&PoolLock);
	
	for (auto& Pool : ObjectPoolOfClass)
	{
		for (auto Object : Pool.Value.ObjectPool)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
	}

	for (auto& Pool : ObjectPoolOfId)
	{
		for (auto Object : Pool.Value.ObjectPool)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
	}

	ObjectPoolOfClass.Empty();
	ObjectPoolOfId.Empty();
}

void UTireflyObjectPoolGameInstanceSubsystem::ClearObjectPoolByClass(const TSubclassOf<UObject>& ObjectClass)
{
	if (FTireflyObjectPool* Pool = ObjectPoolOfClass.Find(ObjectClass))
	{
		for (auto Object : Pool->ObjectPool)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
		Pool->ObjectPool.Empty();
		ObjectPoolOfClass.Remove(ObjectClass);
	}
}

void UTireflyObjectPoolGameInstanceSubsystem::ClearObjectPoolById(FName ObjectId)
{
	if (FTireflyObjectPool* Pool = ObjectPoolOfId.Find(ObjectId))
	{
		for (auto Object : Pool->ObjectPool)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
		Pool->ObjectPool.Empty();
		ObjectPoolOfId.Remove(ObjectId);
	}
}

TArray<TSubclassOf<UObject>> UTireflyObjectPoolGameInstanceSubsystem::Debug_GetAllObjectPoolClasses() const
{
	TArray<TSubclassOf<UObject>> ObjectClasses;
	ObjectPoolOfClass.GetKeys(ObjectClasses);
	return ObjectClasses;
}

TArray<FName> UTireflyObjectPoolGameInstanceSubsystem::DebugGetObjectPoolIds() const
{
	TArray<FName> ObjectIds;
	ObjectPoolOfId.GetKeys(ObjectIds);
	return ObjectIds;
}

int32 UTireflyObjectPoolGameInstanceSubsystem::DebugObjectNumberOfClassPool(const TSubclassOf<UObject>& ObjectClass) const
{
	if (!ObjectPoolOfClass.Contains(ObjectClass))
	{
		return -1;
	}
	return ObjectPoolOfClass[ObjectClass].ObjectPool.Num();
}

int32 UTireflyObjectPoolGameInstanceSubsystem::DebugObjectNumberOfIdPool(FName ObjectId) const
{
	if (!ObjectPoolOfId.Contains(ObjectId))
	{
		return -1;
	}
	return ObjectPoolOfId[ObjectId].ObjectPool.Num();
}
