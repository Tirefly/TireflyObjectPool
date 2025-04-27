// Copyright Tirefly. All Rights Reserved.


#include "TireflyObjectPoolGameInstanceSubsystem.h"

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

UObject* UTireflyObjectPoolGameInstanceSubsystem::GetObject_Internal(
	const TSubclassOf<UObject>& ObjectClass,
	FName ObjectID)
{
	FTireflyObjectPool* Pool = ObjectPoolOfID.Find(ObjectID);
	if (!Pool)
	{
		Pool = ObjectPoolOfClass.Find(ObjectClass);
	}

	if (Pool && Pool->ObjectPool.Num() > 0)
	{
		UObject* Object = Pool->ObjectPool.Pop();
        
		if (Object->Implements<UTireflyPoolingObjectInterface>())
		{
			ITireflyPoolingObjectInterface::Execute_PoolingInitialize(Object);
		}
        
		return Object;
	}

	// Create new instance if pool is empty
	if (ObjectClass)
	{
		UObject* NewObjectInst = NewObject<UObject>(GetTransientPackage(), ObjectClass);
        
		if (NewObjectInst->Implements<UTireflyPoolingObjectInterface>())
		{
			if (ObjectID != NAME_None)
			{
				ITireflyPoolingObjectInterface::Execute_PoolingSetObjectID(NewObjectInst, ObjectID);
			}
			ITireflyPoolingObjectInterface::Execute_PoolingInitialize(NewObjectInst);
		}
        
		return NewObjectInst;
	}

	return nullptr;
}

UObject* UTireflyObjectPoolGameInstanceSubsystem::K2_GetObject(const TSubclassOf<UObject>& ObjectClass, FName ObjectID)
{
	return GetObject_Internal(ObjectClass, ObjectID);
}

TArray<UObject*> UTireflyObjectPoolGameInstanceSubsystem::K2_GetObjects(const TSubclassOf<UObject>& ObjectClass, FName ObjectID,
	int32 Count)
{
	return GetObjects(ObjectClass, ObjectID, Count);
}

void UTireflyObjectPoolGameInstanceSubsystem::ReturnObject(UObject* Object)
{
	if (!IsValid(Object))
	{
		return;
	}

	FName ObjectID = NAME_None;
	if (Object->Implements<UTireflyPoolingObjectInterface>())
	{
		ObjectID = ITireflyPoolingObjectInterface::Execute_PoolingGetObjectID(Object);
		ITireflyPoolingObjectInterface::Execute_PoolingCleanup(Object);
	}

	if (ObjectID != NAME_None)
	{
		FTireflyObjectPool& Pool = ObjectPoolOfID.FindOrAdd(ObjectID);
		Pool.ObjectPool.Push(Object);
	}
	else
	{
		FTireflyObjectPool& Pool = ObjectPoolOfClass.FindOrAdd(Object->GetClass());
		Pool.ObjectPool.Push(Object);
	}
}

void UTireflyObjectPoolGameInstanceSubsystem::WarmUpObjects(TSubclassOf<UObject> ObjectClass, FName ObjectID,
	int32 Count)
{
	if (!ObjectClass || Count <= 0)
	{
		return;
	}

	FTireflyObjectPool& Pool = ObjectID != NAME_None ? 
		ObjectPoolOfID.FindOrAdd(ObjectID) : 
		ObjectPoolOfClass.FindOrAdd(ObjectClass);
        
	Pool.ObjectPool.Reserve(Pool.ObjectPool.Num() + Count);
    
	for (int32 i = 0; i < Count; i++)
	{
		UObject* Object = NewObject<UObject>(GetTransientPackage(), ObjectClass);
        
		if (Object->Implements<UTireflyPoolingObjectInterface>())
		{
			if (ObjectID != NAME_None)
			{
				ITireflyPoolingObjectInterface::Execute_PoolingSetObjectID(Object, ObjectID);
			}
			ITireflyPoolingObjectInterface::Execute_PoolingCleanup(Object);
		}
        
		Pool.ObjectPool.Push(Object);
	}
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

void UTireflyObjectPoolGameInstanceSubsystem::ClearObjectPoolByID(FName ObjectID)
{
	if (FTireflyObjectPool* Pool = ObjectPoolOfID.Find(ObjectID))
	{
		for (auto Object : Pool->ObjectPool)
		{
			if (IsValid(Object))
			{
				Object->ConditionalBeginDestroy();
			}
		}
		Pool->ObjectPool.Empty();
		ObjectPoolOfID.Remove(ObjectID);
	}
}

void UTireflyObjectPoolGameInstanceSubsystem::ClearAllObjectPools()
{
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

	for (auto& Pool : ObjectPoolOfID)
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
	ObjectPoolOfID.Empty();
}

TArray<TSubclassOf<UObject>> UTireflyObjectPoolGameInstanceSubsystem::DebugObjectClasses() const
{
	TArray<TSubclassOf<UObject>> ObjectClasses;
	ObjectPoolOfClass.GetKeys(ObjectClasses);
	return ObjectClasses;
}

TArray<FName> UTireflyObjectPoolGameInstanceSubsystem::DebugObjectIDs() const
{
	TArray<FName> ObjectIDs;
	ObjectPoolOfID.GetKeys(ObjectIDs);
	return ObjectIDs;
}

int32 UTireflyObjectPoolGameInstanceSubsystem::DebugObjectNumberOfClass(const TSubclassOf<UObject>& ObjectClass) const
{
	if (!ObjectPoolOfClass.Contains(ObjectClass))
	{
		return -1;
	}
	return ObjectPoolOfClass[ObjectClass].ObjectPool.Num();
}

int32 UTireflyObjectPoolGameInstanceSubsystem::DebugObjectNumberOfID(FName ObjectID) const
{
	if (!ObjectPoolOfID.Contains(ObjectID))
	{
		return -1;
	}
	return ObjectPoolOfID[ObjectID].ObjectPool.Num();
}
