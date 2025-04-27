// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TireflyObjectPoolGameInstanceSubsystem.generated.h"


USTRUCT()
struct FTireflyObjectPool
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<UObject*> ObjectPool;
};


/** 全局Object对象池子系统 */
/** Global object pool subsystem */
UCLASS()
class TIREFLYOBJECTPOOL_API UTireflyObjectPoolGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region GameInstanceSubsystem
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;

#pragma endregion


#pragma region ObjectPool_Getter

protected:
	UObject* GetObject_Internal(const TSubclassOf<UObject>& ObjectClass, FName ObjectID);

public:
	template<typename T>
	T* GetObject(TSubclassOf<T> ObjectClass, FName ObjectID = NAME_None);

	template<typename T>
	TArray<T*> GetObjects(TSubclassOf<T> ObjectClass, FName ObjectID = NAME_None, int32 Count = 16);

	UFUNCTION(BlueprintCallable, Category = "Object Pool", Meta = (DeterminesOutputType = "ObjectClass", DisplayName = "Object Pool - Get Object"))
	UObject* K2_GetObject(const TSubclassOf<UObject>& ObjectClass, FName ObjectID);

	UFUNCTION(BlueprintCallable, Category = "Object Pool", Meta = (DeterminesOutputType = "ObjectClass", DisplayName = "Object Pool - Get Objects"))
	TArray<UObject*> K2_GetObjects(const TSubclassOf<UObject>& ObjectClass, FName ObjectID, int32 Count = 16);

#pragma endregion


#pragma region ObjectPool_Recycle

public:
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ReturnObject(UObject* Object);

#pragma endregion


#pragma region ObjectPool_WarmUp

public:
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void WarmUpObjects(TSubclassOf<UObject> ObjectClass, FName ObjectID, int32 Count);

#pragma endregion


#pragma region ObjectPool_Clear

public:
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ClearObjectPoolByClass(const TSubclassOf<UObject>& ObjectClass);

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ClearObjectPoolByID(FName ObjectID);

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	void ClearAllObjectPools();

#pragma endregion


#pragma region ObjectPool_Debug

public:
	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	TArray<TSubclassOf<UObject>> DebugObjectClasses() const;

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	TArray<FName> DebugObjectIDs() const;

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	int32 DebugObjectNumberOfClass(const TSubclassOf<UObject>& ObjectClass) const;

	UFUNCTION(BlueprintCallable, Category = "Object Pool")
	int32 DebugObjectNumberOfID(FName ObjectID) const;

#pragma endregion


#pragma region ObjectPool_Declaration

private:
	UPROPERTY()
	TMap<TSubclassOf<UObject>, FTireflyObjectPool> ObjectPoolOfClass;

	UPROPERTY()
	TMap<FName, FTireflyObjectPool> ObjectPoolOfID;

#pragma endregion
};


#pragma region ObjectPool_FuncitonTemplate

template <typename T>
T* UTireflyObjectPoolGameInstanceSubsystem::GetObject(TSubclassOf<T> ObjectClass, FName ObjectID)
{
	return Cast<T>(GetObject_Internal(ObjectClass, ObjectID));
}

template <typename T>
TArray<T*> UTireflyObjectPoolGameInstanceSubsystem::GetObjects(TSubclassOf<T> ObjectClass, FName ObjectID, int32 Count)
{
	TArray<T*> Objects;
	for (int32 i = 0; i < Count; ++i)
	{
		Objects.Add(Cast<T>(GetObject_Internal(ObjectClass, ObjectID)));
	}

	return Objects;
}

#pragma endregion
