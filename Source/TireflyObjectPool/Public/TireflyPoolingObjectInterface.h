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


// 对象池生成的对象需要实现的接口
class TIREFLYOBJECTPOOL_API ITireflyPoolingObjectInterface
{
	GENERATED_BODY()

public:
	// 对象从对象池中取出后执行的初始化
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Object Pool")
	void PoolingInitialize();
	virtual void PoolingInitialize_Implementation() {}

	// 对象从对象池中取出后执行的属性重置
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Object Pool")
	void PoolingResetProperty(const FInstancedStruct& PropertyData);
	virtual void PoolingResetProperty_Implementation(const FInstancedStruct& PropertyData) {}

	// 对象被放回对象池中后执行的清理
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Object Pool")
	void PoolingCleanup();
	virtual void PoolingCleanup_Implementation() {}

	// 获取对象的Id
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Object Pool")
	FName PoolingGetObjectId() const;
	virtual FName PoolingGetObjectId_Implementation() const { return NAME_None; }

	// 设置对象的Id
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tirefly Object Pool")
	void PoolingSetObjectId(FName NewObjectId);
	virtual void PoolingSetObjectId_Implementation(FName NewObjectId) {}
};
