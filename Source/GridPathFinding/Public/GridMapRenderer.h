// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridEnvironmentType.h"
#include "GameFramework/Actor.h"
#include "Types/HCubeCoord.h"
#include "Types/MapConfig.h"
#include "Types/TileInfo.h"
#include "GridMapRenderer.generated.h"

enum class ETileTokenModifyType;
class UGridMapModel;


USTRUCT(BlueprintType)
struct FGridMapRenderConfig
{
	GENERATED_BODY()

	FGridMapRenderConfig()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="绘制Cursor"))
	bool bDrawTileCursor{false};

	// UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="使用默认地图绘制方式"))
	// bool bDrawDefaultMap{true};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="绘制背景Wireframe"))
	bool bDrawBackgroundWireframe{true};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="背景Wireframe默认颜色", EditCondition="bDrawBackgroundWireframe", EditConditionHides))
	FLinearColor BackgroundWireframeColor{FLinearColor::White};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="背景Wireframe高亮颜色", EditCondition="bDrawBackgroundWireframe", EditConditionHides))
	FLinearColor BackgroundWireframeHighlightColor{FLinearColor::Red};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="背景Wireframe默认绘制偏移", EditCondition="bDrawBackgroundWireframe", EditConditionHides))
	FVector BackgroundDrawLocationOffset = FVector(0.f, 0.f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="高亮Mask绘制偏移"))
	FVector HighlightMaskLocationOffset = FVector(0.f, 0.f, 0.3f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="高亮Mask每层高度偏移"))
	FVector HighlightMaskHeightOffset = FVector(0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="高亮Mask默认颜色"))
	FLinearColor DefaultHighlightColor{FLinearColor::Red};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="高亮Mask基础尺寸"))
	float MaskBaseSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="高度比例"))
	float HeightScale = 1.0f;
};

/**
 * 需要更换绘制用的mesh时， 直接在关卡中自行修改
 */
UCLASS()
class GRIDPATHFINDING_API AGridMapRenderer : public AActor
{
	GENERATED_BODY()

	inline static FRotator FlatRotator = FRotator(0.f, 0.f, 0.f);
	inline static FRotator HexPointyRotator = FRotator(0.f, 30.f, 0.f);
	inline static FRotator RectPointyRotator = FRotator(0.f, 90.f, 0.f);

public:
	// Sets default values for this actor's properties
	AGridMapRenderer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	FSimpleMulticastDelegate OnRenderOver;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Config)
	FGridMapRenderConfig RenderConfig;

	// 数据模型
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UGridMapModel> GridModel;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInstancedStaticMeshComponent> HighLightMask;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInstancedStaticMeshComponent> BackgroundWireframe;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UInstancedStaticMeshComponent> TileCursorRenderer;

	FDelegateHandle TilesDataBuildOverHandle;

public:
	void SetModel(UGridMapModel* InModel);

	virtual void RenderGridMap();

	virtual void ClearGridMap();

	// 使用WireFrame高亮
	void HighLightBackground(const FHCubeCoord& InCoord, bool bHighLight);
	
	// Mask高亮相关方法
	void AddHighlightMask(const FHCubeCoord& InCoord, const FLinearColor& HighlightColor);

	void RemoveHighlightMask(const FHCubeCoord& InCoord);

	// 清理所有Mask高亮
	void ClearAllHighlightMasks();

	// 设置当前光标位置
	void SetCurrentTileCursor(const FVector& InLocation, float InScale, const FLinearColor& InColor);

protected:
	virtual void RenderTiles();

	UFUNCTION(BlueprintImplementableEvent)
	void SetBackgroundWireframeMesh(EGridMapType InMapType);

	void DrawBackgroundWireframe();

	// ------绘制地图的辅助函数-------
	FRotator GetGridRotator() const;

	void SetWireFrameColor(TObjectPtr<UInstancedStaticMeshComponent> InWireFrame, int Index, const FLinearColor& InColor, float DefaultHeight, float NewHeight = 0.f);

	virtual void OnTileEnvUpdate(const FHCubeCoord& InCoord, const FTileEnvData& OldTileInfo, const FTileEnvData& NewTileInfo);

	void OnTileHeightUpdate(const FHCubeCoord& InCoord, float oldHeight, float NewHeight);
	
private:
	void OnTilesDataBuildCancel();
	void OnTilesDataBuildComplete();

	// ------- 默认环境Mesh绘制功能 Start ----------

	// 根据不同环境类型， 运行时创建多个InstancedStaticMeshComponent
protected:
	UPROPERTY(VisibleAnywhere)
	TArray<UInstancedStaticMeshComponent*> DynamicEnvironmentComponents;

	// 按照Mesh分组存储组件，而非按环境类型
	UPROPERTY()
	TMap<UStaticMesh*, int32> Mesh2ISMCIndexMap;

	// 记录环境类型与其对应的Mesh关系
	UPROPERTY()
	TMap<FName, UStaticMesh*> EnvTypeToMeshMap;
	UPROPERTY()
	TMap<FName, FGridEnvironmentMaterialCustomData> EnvType2DefaultCustomDataMap;

	// Coord to ISM Index
	UPROPERTY()
	TMap<FHCubeCoord, int32> EnvISMCIndexMap;

	UPROPERTY(EditAnywhere, Category=Config, meta=(DisplayName="默认Lit"))
	float DefaultTint = 1.0f;
	
	// 为每种环境类型创建对应的ISM组件
	virtual void InitializeEnvironmentComponents();

	// 获取指定环境类型的ISM组件
	UFUNCTION(BlueprintCallable)
	UInstancedStaticMeshComponent* GetEnvironmentComponent(FName TypeID);

	virtual void UpdateTileEnvRenderer(FHCubeCoord Coord, const FTileEnvData& InOldEnvData, const FTileEnvData& InNewEnvData);
	// ------ 默认环境Mesh绘制功能 End ----------

	// ------- HighLightMask 功能 Start ----------
	// 坐标到 HighLightMask 实例索引的映射
	UPROPERTY()
	TMap<FHCubeCoord, int32> HighlightMaskIndexMap;
	// ------- HighLightMask 功能 End ----------

private:
	// 调试Chunk功能分区是否正确
	void DebugDrawChunks();
};
