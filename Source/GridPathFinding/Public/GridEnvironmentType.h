// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GridEnvironmentType.generated.h"

UENUM(BlueprintType)
enum class EDecorationObjectType : uint8
{
	StaticMesh,
	Actor
};

// USTRUCT(BlueprintType)
// struct FDecorationDescriptor
// {
// 	GENERATED_BODY()
//
// 	// 装饰物类型
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoration")
// 	EDecorationObjectType ObjectType = EDecorationObjectType::StaticMesh;
//
// 	// 静态网格体引用
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoration", meta = (EditCondition = "ObjectType == EDecorationObjectType::StaticMesh", EditConditionHides))
// 	TSoftObjectPtr<UStaticMesh> StaticMesh;
//
// 	// Actor类引用
// 	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoration", meta = (EditCondition = "ObjectType == EDecorationObjectType::Actor", EditConditionHides))
// 	TSoftClassPtr<AActor> ActorClass;
// };

/**
 * Todo: 存放用于设置到材质球的各种参数， 但是只能用float参数， 因为只会用于InstancedStaticMeshComponent
 */
USTRUCT(BlueprintType)
struct FGridEnvironmentMaterialCustomData
{
	GENERATED_BODY()

	/**
	 * 使用Texture2DArray, 为了防止贴图数组阅读难度过大， 分层， 这里仅确定贴图的Category， 具体使用哪个Index， 相同Env会有不同的配置，因此放在序列化数据中
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="贴图Category")
	float TextureArrayCategory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="贴图Tint")
	float DefaultTint = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Roughness")
	float Roughness = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="NormalIntensity")
	float NormalIntensity = 1.f;
};

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UGridEnvironmentType : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	inline static FName EmptyEnvTypeID = FName("Empty");
	
	// 环境类型的唯一标识符
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	FName TypeID;
    
	// 环境类型的显示名称
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	FText DisplayName;
    
	// 环境类型的描述
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Default")
	FText Description;
    
	// 环境的视觉属性
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	FLinearColor CustomData;
    
	// 装饰物模型
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals", meta=(DisplayName="装饰物模型"))
	TSoftObjectPtr<UStaticMesh> DecorationMesh;

	// 装饰物材质球参数
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals", meta=(DisplayName="装饰物材质球参数"))
	FGridEnvironmentMaterialCustomData DecorationMaterialCustomData;
    
	// 装饰物数量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals", meta = (ClampMin = "1", ClampMax = "10"))
	int32 DecorationMaxCount = 1;
    
	// 是否随机旋转装饰物
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	bool bRandomDecorationRotation = false;
    
	// 是否随机放置装饰物
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	bool bRandomDecorationLocation = false;
    
	// 装饰物缩放
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	float DecorationScale = 1.0f;
    
	// 游戏玩法相关属性
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	float MovementCost = 1.0f;
    
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	bool bIsBlocking = false;

	// ---------- 用于地图编辑器的资源 / 运行时默认的地图绘制方式 Start----------
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BuildGridMap")
	TSoftObjectPtr<UStaticMesh> BuildGridMapMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BuildGridMap")
	TSoftObjectPtr<UMaterial> BuildGridMapMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BuildGridMap")
	FGridEnvironmentMaterialCustomData BuildGridMapMaterialCustomData;
	// ---------- 用于地图编辑器的资源 / 运行时默认的地图绘制方式  End----------
	
	// 获取资产的标识符
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("HexEnvironmentType", TypeID);
	}

	virtual float GetCost() const
	{
		return MovementCost;
	}
};
