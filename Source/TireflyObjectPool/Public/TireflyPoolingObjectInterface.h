// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "StructUtils/InstancedStruct.h"
#include "TireflyPoolingObjectInterface.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UTireflyPoolingObjectInterface : public UInterface
{
	GENERATED_BODY()
};


/** 对象池生成的对象需要实现的接口 */
/** Interface that objects spawned from object pool should implement */
class TIREFLYOBJECTPOOL_API ITireflyPoolingObjectInterface
{
	GENERATED_BODY()

public:
	// 对象从对象池中取出后执行的初始化
	// Initialization executed after the object is taken from the object pool
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void PoolingInitialize();
	virtual void PoolingInitialize_Implementation() {}

	// 对象从对象池中取出后执行的属性重置
	// Reset properties after the object is taken from the object pool
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void PoolingResetProperty(const FInstancedStruct& PropertyData);
	virtual void PoolingResetProperty_Implementation(const FInstancedStruct& PropertyData) {}

	// 对象被放回对象池中后执行的清理
	// Cleanup executed after the object is returned to the object pool
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void PoolingCleanup();
	virtual void PoolingCleanup_Implementation() {}

	// 获取对象的ID
	// Get the ID of the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	FName PoolingGetObjectID() const;
	virtual FName PoolingGetObjectID_Implementation() const { return NAME_None; }

	// 设置对象的ID
	// Set the ID of the object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Object Pool")
	void PoolingSetObjectID(FName NewObjectID);
	virtual void PoolingSetObjectID_Implementation(FName NewObjectID) {}
};
