# TireflyObjectPool 插件完全精通手册

## 概述

TireflyObjectPool 是一个专门用于管理UObject对象池的高性能系统插件。与TireflyActorPool专注于Actor不同，此插件针对通用UObject对象提供池化管理，适用于数据对象、组件、工具类等非Actor场景的性能优化。

## 核心特性

- **通用对象池**: 支持所有UObject及其子类的池化管理
- **跨关卡持续**: 基于GameInstance子系统，对象池跨关卡保持
- **双重池策略**: 支持按类型和按ID的对象池分类
- **线程安全**: 使用Critical Section确保多线程安全
- **结构化初始化**: 支持FInstancedStruct参数化对象初始化
- **蓝图友好**: 完整的蓝图接口支持

## 核心组件

1. **UTireflyObjectPoolGameInstanceSubsystem**: 游戏实例子系统，核心管理器
2. **ITireflyPoolingObjectInterface**: 对象池接口，定义池化行为
3. **UTireflyObjectPoolLibrary**: 蓝图函数库
4. **FTireflyObjectPool**: 对象池结构体

## 基本使用

### 1. 实现对象池接口

所有要使用对象池的UObject都必须实现ITireflyPoolingObjectInterface接口：

```cpp
UCLASS(BlueprintType)
class MYGAME_API UMyDataObject : public UObject, public ITireflyPoolingObjectInterface
{
    GENERATED_BODY()

public:
    // 实现池化接口
    virtual void PoolingInitialize_Implementation() override;
    virtual void PoolingResetProperty_Implementation(const FInstancedStruct& PropertyData) override;
    virtual void PoolingCleanup_Implementation() override;
    virtual FName PoolingGetObjectId_Implementation() const override;
    virtual void PoolingSetObjectId_Implementation(FName NewObjectId) override;

private:
    UPROPERTY()
    FName ObjectId = NAME_None;
    
    UPROPERTY()
    int32 Data = 0;
    
    UPROPERTY()
    TArray<FString> StringArray;
};
```

### 2. 接口实现示例

```cpp
void UMyDataObject::PoolingInitialize_Implementation()
{
    // 对象从池中取出时的初始化
    Data = 0;
    StringArray.Empty();
    bIsValid = true;
}

void UMyDataObject::PoolingResetProperty_Implementation(const FInstancedStruct& PropertyData)
{
    // 使用传入的结构化数据进行属性重置
    if (PropertyData.GetScriptStruct() == FMyObjectInitData::StaticStruct())
    {
        const FMyObjectInitData* InitData = PropertyData.GetPtr<FMyObjectInitData>();
        Data = InitData->InitialValue;
        StringArray = InitData->InitialStrings;
    }
}

void UMyDataObject::PoolingCleanup_Implementation()
{
    // 对象回收到池中前的清理
    Data = 0;
    StringArray.Empty();
    bIsValid = false;
}

FName UMyDataObject::PoolingGetObjectId_Implementation() const
{
    return ObjectId;
}

void UMyDataObject::PoolingSetObjectId_Implementation(FName NewObjectId)
{
    ObjectId = NewObjectId;
}
```

### 3. 从对象池获取对象

```cpp
// C++方式 - 基本获取
UObject* MyObject = UTireflyObjectPoolLibrary::GetObjectFromPool(
    this,                          // 世界上下文
    UMyDataObject::StaticClass(),  // 对象类型
    TEXT("DataProcessor")          // 对象ID（可选）
);

// 带参数的获取
FMyObjectInitData InitData;
InitData.InitialValue = 100;
InitData.InitialStrings = { TEXT("Test1"), TEXT("Test2") };

FInstancedStruct PropertyData = FInstancedStruct::Make(InitData);

UObject* ObjectWithData = UTireflyObjectPoolLibrary::GetObjectFromPoolWithParam(
    this,
    UMyDataObject::StaticClass(),
    TEXT("ConfiguredData"),
    PropertyData
);
```

### 4. 回收对象到池

```cpp
// 回收对象
UTireflyObjectPoolLibrary::RecycleObjectToPool(this, MyObject);

// 对象会根据其ObjectId自动分类到相应的池中
```

### 5. 对象池预热

```cpp
// 预热对象池
UTireflyObjectPoolLibrary::WarmUpObjectPool(
    this,
    UMyDataObject::StaticClass(),
    TEXT("DataProcessor"),
    20  // 预热20个实例
);
```

## 高级应用场景

### 1. 数据处理对象池

```cpp
// 数据处理器对象
UCLASS()
class MYGAME_API UDataProcessor : public UObject, public ITireflyPoolingObjectInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    TArray<FString> ProcessData(const TArray<int32>& InputData);

    virtual void PoolingInitialize_Implementation() override
    {
        ProcessedCount = 0;
        LastProcessTime = 0.0f;
    }

private:
    int32 ProcessedCount;
    float LastProcessTime;
};

// 使用示例
void AMyGameMode::ProcessLargeDataSet()
{
    TArray<int32> DataChunks = SplitDataIntoChunks(LargeDataSet);
    
    for (int32 i = 0; i < DataChunks.Num(); ++i)
    {
        // 从池中获取处理器
        UDataProcessor* Processor = Cast<UDataProcessor>(
            UTireflyObjectPoolLibrary::GetObjectFromPool(
                this, UDataProcessor::StaticClass(), TEXT("DataProcessor")
            )
        );
        
        // 处理数据
        TArray<FString> Result = Processor->ProcessData(DataChunks);
        
        // 使用结果...
        
        // 回收处理器
        UTireflyObjectPoolLibrary::RecycleObjectToPool(this, Processor);
    }
}
```

### 2. 配置对象池

```cpp
// 配置数据对象
UCLASS(BlueprintType)
class MYGAME_API UGameConfig : public UObject, public ITireflyPoolingObjectInterface
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    float Difficulty = 1.0f;
    
    UPROPERTY(BlueprintReadWrite)
    TMap<FString, float> Settings;

    virtual void PoolingResetProperty_Implementation(const FInstancedStruct& PropertyData) override
    {
        if (PropertyData.GetScriptStruct() == FConfigInitData::StaticStruct())
        {
            const FConfigInitData* InitData = PropertyData.GetPtr<FConfigInitData>();
            Difficulty = InitData->DifficultyLevel;
            Settings = InitData->GameSettings;
        }
    }
};

// 配置管理器
UCLASS()
class MYGAME_API UConfigManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    UGameConfig* GetTempConfig(const FString& ConfigType);

    UFUNCTION(BlueprintCallable)
    void ReleaseTempConfig(UGameConfig* Config);
};
```

### 3. 临时计算对象池

```cpp
// 数学计算工具对象
UCLASS()
class MYGAME_API UMathCalculator : public UObject, public ITireflyPoolingObjectInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    float CalculateComplexFormula(const TArray<float>& Inputs);

    UFUNCTION(BlueprintCallable)
    FVector CalculateTrajectory(FVector Start, FVector Target, float Gravity);

    virtual void PoolingInitialize_Implementation() override
    {
        // 重置缓存
        CalculationCache.Empty();
        LastCalculationTime = 0.0f;
    }

private:
    TMap<uint32, float> CalculationCache;
    float LastCalculationTime;
};

// 使用示例
class MYGAME_API AProjectileWeapon : public AActor
{
public:
    void CalculateProjectilePath()
    {
        // 获取计算器
        UMathCalculator* Calculator = Cast<UMathCalculator>(
            UTireflyObjectPoolLibrary::GetObjectFromPool(
                this, UMathCalculator::StaticClass(), TEXT("TrajectoryCalc")
            )
        );
        
        // 执行计算
        FVector TrajectoryPoint = Calculator->CalculateTrajectory(
            GetActorLocation(), TargetLocation, ProjectileGravity
        );
        
        // 使用结果...
        
        // 回收计算器
        UTireflyObjectPoolLibrary::RecycleObjectToPool(this, Calculator);
    }
};
```

## 与TireflyActorPool的对比

| 特性 | TireflyObjectPool | TireflyActorPool |
|------|------------------|------------------|
| **目标类型** | UObject及其子类 | AActor及其子类 |
| **子系统类型** | GameInstanceSubsystem | WorldSubsystem |
| **生命周期** | 跨关卡持续 | 限于当前World |
| **主要用途** | 数据对象、工具类、组件 | 游戏实体、特效、AI |
| **初始化方式** | PoolingResetProperty | PoolingInitialized |
| **适用场景** | 计算、配置、临时数据 | 可见对象、游戏逻辑 |

## 性能优化建议

### 1. 合理的对象设计

```cpp
// 好的做法：轻量级对象
UCLASS()
class ULightweightData : public UObject, public ITireflyPoolingObjectInterface
{
    GENERATED_BODY()

public:
    // 最小化成员变量
    UPROPERTY()
    int32 SimpleValue;
    
    // 避免复杂的嵌套结构
    UPROPERTY()
    TArray<float> SimpleArray;
};

// 避免的做法：重量级对象
UCLASS()
class UHeavyweightData : public UObject, public ITireflyPoolingObjectInterface
{
    GENERATED_BODY()

public:
    // 避免大量复杂数据
    UPROPERTY()
    TMap<FString, TArray<TMap<int32, FComplexStruct>>> ComplexNestedData;
    
    // 避免昂贵的组件引用
    UPROPERTY()
    TArray<UActorComponent*> ComponentReferences;
};
```

### 2. 预热策略

```cpp
void AMyGameMode::PrewarmObjectPools()
{
    // 根据使用频率预热
    UTireflyObjectPoolLibrary::WarmUpObjectPool(this, UDataProcessor::StaticClass(), TEXT("HighFreq"), 50);
    UTireflyObjectPoolLibrary::WarmUpObjectPool(this, UMathCalculator::StaticClass(), TEXT("MediumFreq"), 20);
    UTireflyObjectPoolLibrary::WarmUpObjectPool(this, UGameConfig::StaticClass(), TEXT("LowFreq"), 5);
}
```

### 3. 内存管理

```cpp
class MYGAME_API UPoolManager : public UGameInstanceSubsystem
{
public:
    UFUNCTION(BlueprintCallable)
    void CleanupUnusedPools()
    {
        if (UTireflyObjectPoolGameInstanceSubsystem* PoolSubsystem = 
            GetGameInstance()->GetSubsystem<UTireflyObjectPoolGameInstanceSubsystem>())
        {
            // 清理特定类型的池
            for (auto& UnusedClass : GetUnusedObjectClasses())
            {
                PoolSubsystem->ClearObjectPoolsOfClass(UnusedClass);
            }
        }
    }
};
```

## 调试功能

```cpp
// 获取对象池状态信息
UFUNCTION(BlueprintCallable, Category = "Debug")
void DebugObjectPoolStatus()
{
    if (UTireflyObjectPoolGameInstanceSubsystem* PoolSubsystem = 
        GetGameInstance()->GetSubsystem<UTireflyObjectPoolGameInstanceSubsystem>())
    {
        TArray<TSubclassOf<UObject>> AllClasses = PoolSubsystem->Debug_GetAllObjectPoolClasses();
        TArray<FName> AllIds = PoolSubsystem->Debug_GetAllObjectPoolIds();
        
        for (auto ObjectClass : AllClasses)
        {
            int32 Count = PoolSubsystem->Debug_GetObjectNumberOfClassPool(ObjectClass);
            UE_LOG(LogTemp, Log, TEXT("Class Pool [%s]: %d objects"), *ObjectClass->GetName(), Count);
        }
        
        for (FName Id : AllIds)
        {
            int32 Count = PoolSubsystem->Debug_GetObjectNumberOfIdPool(Id);
            UE_LOG(LogTemp, Log, TEXT("ID Pool [%s]: %d objects"), *Id.ToString(), Count);
        }
    }
}
```

## 最佳实践

1. **接口实现**: 正确实现ITireflyPoolingObjectInterface的所有方法
2. **对象设计**: 保持对象轻量级，避免复杂的依赖关系
3. **生命周期管理**: 确保对象在使用完毕后及时回收
4. **预热策略**: 根据实际使用频率合理预热对象池
5. **内存监控**: 定期检查对象池状态，避免内存泄漏

## 总结

TireflyObjectPool插件为UObject对象提供了高效的池化管理方案，特别适用于需要频繁创建/销毁数据对象、工具类、计算对象的场景。通过合理使用此插件，可以显著减少内存分配开销，提升游戏性能，特别是在数据密集型和计算密集型的游戏场景中效果显著。
