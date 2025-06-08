// Copyright Tirefly. All Rights Reserved.


#include "TireflyObjectPoolLibrary.h"

#include "TireflyObjectPoolGameInstanceSubsystem.h"
#include "TireflyObjectPoolLogChannels.h"
#include "Kismet/GameplayStatics.h"



UObject* UTireflyObjectPoolLibrary::GetObjectFromPool(
	const UObject* WorldContext,
	TSubclassOf<UObject> ObjectClass,
	FName ObjectId)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No GameInstance found"), *FString(__FUNCTION__));
		return nullptr;
	}

	UTireflyObjectPoolGameInstanceSubsystem* GIS = GI->GetSubsystem<UTireflyObjectPoolGameInstanceSubsystem>();
	if (!GIS)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No ObjectPoolGameInstanceSubsystem found"), *FString(__FUNCTION__));
		return nullptr;
	}
	
	return GIS->GetObjectFromPool(ObjectClass, ObjectId);
}

UObject* UTireflyObjectPoolLibrary::GetObjectFromPoolWithParam(
	const UObject* WorldContext,
	TSubclassOf<UObject> ObjectClass,
	FName ObjectId,
	const FInstancedStruct& PropertyData)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No GameInstance found"), *FString(__FUNCTION__));
		return nullptr;
	}

	UTireflyObjectPoolGameInstanceSubsystem* GIS = GI->GetSubsystem<UTireflyObjectPoolGameInstanceSubsystem>();
	if (!GIS)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No ObjectPoolGameInstanceSubsystem found"), *FString(__FUNCTION__));
		return nullptr;
	}
	
	return GIS->GetObjectFromPool(ObjectClass, ObjectId, &PropertyData);
}

void UTireflyObjectPoolLibrary::RecycleObjectToPool(const UObject* WorldContext, UObject* Object)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No GameInstance found"), *FString(__FUNCTION__));
		return;
	}

	UTireflyObjectPoolGameInstanceSubsystem* GIS = GI->GetSubsystem<UTireflyObjectPoolGameInstanceSubsystem>();
	if (!GIS)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No ObjectPoolGameInstanceSubsystem found"), *FString(__FUNCTION__));
		return;
	}

	GIS->RecycleObjectToPool(Object);
}

void UTireflyObjectPoolLibrary::WarmUpObjectPool(
	const UObject* WorldContext,
	TSubclassOf<UObject> ObjectClass,
	FName ObjectId,
	int32 Count)
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(WorldContext);
	if (!GI)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No GameInstance found"), *FString(__FUNCTION__));
		return;
	}

	UTireflyObjectPoolGameInstanceSubsystem* GIS = GI->GetSubsystem<UTireflyObjectPoolGameInstanceSubsystem>();
	if (!GIS)
	{
		UE_LOG(LogTireflyObjectPool, Warning, TEXT("[%s] No ObjectPoolGameInstanceSubsystem found"), *FString(__FUNCTION__));
		return;
	}

	GIS->WarmUpObjectPool(ObjectClass, ObjectId, Count);
}
