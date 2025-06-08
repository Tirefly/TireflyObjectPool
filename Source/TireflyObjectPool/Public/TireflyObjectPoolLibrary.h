// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "TireflyObjectPoolLibrary.generated.h"



// 用于Object对象池的函数库
UCLASS()
class TIREFLYOBJECTPOOL_API UTireflyObjectPoolLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 从对象池中取用Object实例
	 * 
	 * @param WorldContext 世界上下文对象，需要通过该对象获取游戏实例
	 * @param ObjectClass 对象实例的指定类型
	 * @param ObjectId 对象的指定Id
	 * @return 从对象池中取出的对象实例，也有可能是全新创建好的
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool", Meta = (WorldContext = "WorldContext", DeterminesOutputType = "ObjectClass"))
	static UObject* GetObjectFromPool(const UObject* WorldContext, TSubclassOf<UObject> ObjectClass, FName ObjectId);

	/**
	 * 从对象池中取用Object实例并附带初始化参数
	 * 
	 * @param WorldContext 世界上下文对象，需要通过该对象获取游戏实例
	 * @param ObjectClass 对象实例的指定类型
	 * @param ObjectId 对象的指定Id
	 * @param PropertyData 初始化参数
	 * @return 从对象池中取出的对象实例，也有可能是全新创建好的
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool", Meta = (WorldContext = "WorldContext", DeterminesOutputType = "ObjectClass"))
	static UObject* GetObjectFromPoolWithParam(
		const UObject* WorldContext,
		TSubclassOf<UObject> ObjectClass,
		FName ObjectId,
		const FInstancedStruct& PropertyData);

	// 回收Object实例
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool", Meta = (WorldContext = "WorldContext"))
	static void RecycleObjectToPool(const UObject* WorldContext, UObject* Object);

	/**
	 * 预热特定类型（或特定Id）的对象池，生成指定数量的Object实例并使其在池中待命
	 *
	 * @param WorldContext 世界上下文对象，需要通过该对象获取游戏实例
	 * @param ObjectClass 对象池的目标类型
	 * @param ObjectId 对象池的目标Id
	 * @param Count 预热的Object实例数量
	 */
	UFUNCTION(BlueprintCallable, Category = "Tirefly Object Pool", Meta = (WorldContext = "WorldContext"))
	static void WarmUpObjectPool(
		const UObject* WorldContext,
		TSubclassOf<UObject> ObjectClass,
		FName ObjectId,
		int32 Count = 16);
};
