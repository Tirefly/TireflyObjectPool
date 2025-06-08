// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "TireflyObjectPoolGameInstanceSubsystem.generated.h"



// Object对象池
USTRUCT()
struct FTireflyObjectPool
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<UObject*> ObjectPool;
};



// Object对象池游戏实例子系统，所有基于该子系统生成的Object实例的Outer都会被默认设置为该子系统实例
UCLASS()
class TIREFLYOBJECTPOOL_API UTireflyObjectPoolGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

#pragma region GameInstanceSubsystem
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void Deinitialize() override;

private:
	// 对象池操作的线程安全锁
	FCriticalSection PoolLock;

#pragma endregion


#pragma region ObjectPool_Getter

protected:
	UObject* FetchObjectFromPool(const TSubclassOf<UObject>& ObjectClass, FName ObjectId);
	
	UObject* GetObjectFromPool_Internal(
		const TSubclassOf<UObject>& ObjectClass,
		FName ObjectId,
		const FInstancedStruct* PropertyData = nullptr);

public:
	template<typename T>
	T* GetObjectFromPool(TSubclassOf<T> ObjectClass, FName ObjectId = NAME_None, const FInstancedStruct* PropertyData = nullptr);

#pragma endregion


#pragma region ObjectPool_Recycle

public:
	// 回收Object实例到对象池中
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	void RecycleObjectToPool(UObject* Object);

#pragma endregion


#pragma region ObjectPool_WarmUp

public:
	/**
	 * 预热特定类型（或特定Id）的对象池，生成指定数量的Object实例并使其在池中待命
	 * 
	 * @param ObjectClass 对象池的目标类型
	 * @param ObjectId 对象的目标Id
	 * @param Count 
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	void WarmUpObjectPool(
		TSubclassOf<UObject> ObjectClass,
		FName ObjectId,
		int32 Count = 16);

#pragma endregion


#pragma region ObjectPool_Clear

public:
	// 清空所有对象池
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	void ClearAllObjectPools();
	
	// 清空指定类型的对象池
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	void ClearObjectPoolByClass(const TSubclassOf<UObject>& ObjectClass);

	// 清空指定Id的对象池
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	void ClearObjectPoolById(FName ObjectId);

#pragma endregion


#pragma region ObjectPool_Debug

public:
	// 获取在Object类对象池中所有的Object类型
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	TArray<TSubclassOf<UObject>> Debug_GetAllObjectPoolClasses() const;

	// 获取在ObjectId对象池中所有的ObjectId
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	TArray<FName> DebugGetObjectPoolIds() const;

	// 获取特定类型的对象池中剩余Object的数量，如果不存在指定类型的对象池或者池中没有Object实例，则返回-1
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	int32 DebugObjectNumberOfClassPool(const TSubclassOf<UObject>& ObjectClass) const;

	// 获取特定Id的对象池中剩余Object的数量，如果不存在指定类型的对象池或者池中没有Object实例，则返回-1
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool")
	int32 DebugObjectNumberOfIdPool(FName ObjectId) const;

#pragma endregion


#pragma region ObjectPool_Declaration

private:
	UPROPERTY()
	TMap<TSubclassOf<UObject>, FTireflyObjectPool> ObjectPoolOfClass;

	UPROPERTY()
	TMap<FName, FTireflyObjectPool> ObjectPoolOfId;

#pragma endregion
};


#pragma region ObjectPool_FuncitonTemplate

template <typename T>
T* UTireflyObjectPoolGameInstanceSubsystem::GetObjectFromPool(
	TSubclassOf<T> ObjectClass,
	FName ObjectId,
	const FInstancedStruct* PropertyData)
{
	return Cast<T>(GetObjectFromPool_Internal(ObjectClass, ObjectId, PropertyData));
}

#pragma endregion
