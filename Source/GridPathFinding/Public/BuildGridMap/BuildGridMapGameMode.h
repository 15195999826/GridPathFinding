// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridEnvironmentType.h"
#include "GameFramework/GameModeBase.h"
#include "Service/MapModelProvider.h"
#include "Types/GridMapSave.h"
#include "Types/HCubeCoord.h"
#include "UI/BuildGridMapTokenActorPanel.h"
#include "BuildGridMapGameMode.generated.h"

class ABuildGridMapRenderer;
class UBuildGridMapCommandManager;
class UGridMapModel;
class UBuildGridMapWindow;

UENUM(BlueprintType)
enum class EBuildGridMapSaveMode : uint8
{
	FullSave UMETA(DisplayName = "全量保存"),
	IncrementalSave UMETA(DisplayName = "增量保存")
};

DECLARE_MULTICAST_DELEGATE_OneParam(FBuildGridMapSaveStartDelegate, EBuildGridMapSaveMode InSaveMode);

/**
 * 设计上， 编辑地图时， 使用InstancedStaticMeshComponent来渲染地图, 因为此时不需要携带游戏玩法
 * 在引用当前插件的项目中， 一般每一个格子的内容都会具有独立的行为逻辑，大概率是需要使用Actor来表示的，因此GamePlay中处理大地图，可以使用Mass
 * 本插件旨在提供一个格子地图编辑器 + 格子地图寻路的功能
 */
UCLASS()
class GRIDPATHFINDING_API ABuildGridMapGameMode : public AGameModeBase, public IMapModelProvider
{
	GENERATED_BODY()

	inline static FGridMapSave EmptyMapSave{};
	
	virtual UGridMapModel* GetGridMapModel() const override;
	
public:
	inline static FString NoneString = TEXT("None");
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category=Config)
	bool DebugSave{false};

	// 实际保存的地图
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UGridMapModel> GridMapModel;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ABuildGridMapRenderer> BuildGridMapRenderer;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UBuildGridMapCommandManager> CommandManager;


protected:
	// 在蓝图中创建和赋值
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UBuildGridMapWindow> BuildGridMapWindow;

	UPROPERTY(BlueprintReadOnly)
	TArray<TSubclassOf<ATokenActor>> TokenActorTypes;
	
	UPROPERTY()
	TMap<FString, int32> TokenActorTypeStringToIndexMap;

	// StaticMesh映射表，Key为路径简写，Value为软引用路径
	UPROPERTY(BlueprintReadOnly)
	TMap<FName, FSoftObjectPath> AvailableStaticMeshes;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FName> MeshPathStrToShortNameMap;
public:
	// 编辑地图相关事件
	FSimpleMulticastDelegate OnCreateGridMapSave;
	FBuildGridMapSaveStartDelegate OnSaveStart;
	FSimpleMulticastDelegate OnSaveOver; // 实现异步保存
	FSimpleMulticastDelegate OnEmptyEditingMapSave;
	FSimpleMulticastDelegate OnSwitchEditingMapSave;
	FSimpleMulticastDelegate OnDeleteGridMapSave;

private:
	UPROPERTY()
	FGridMapSave EditingMapSave;
	
	TMap<FHCubeCoord, FSerializableTile> EditingTiles;

	// --------- 数据保存相关 Start ---------
public:
	// 保存进度更新委托
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSaveProgressDelegate, float, Progress, FString, StatusMessage);

	// 进度更新委托
	UPROPERTY(BlueprintAssignable, Category = "GridPathFinding|Save")
	FSaveProgressDelegate OnSaveProgressUpdated;

	/**
	 * Save不使用命令模式， 不允许撤销和重做
	 * Todo: 每隔60秒自动保存一次， 自动保存为异步保存(保存到那一帧的状态， 保存过程的改变不生效)， 手动按下Ctrl+S时UI出现Mask， 禁止其它操作
	 */
	UFUNCTION(BlueprintCallable)
	void SaveEditingMapSave(EBuildGridMapSaveMode InSaveMode);

	void IntervalMapSaveToFile(const FGridMapSave& InMapSave);

	void MarkEditingTilesDirty(const FHCubeCoord& InDirtyCoord);

	void MarkEditingTilesDirty(const TArray<FHCubeCoord>& InDirtyCoords);

	const TArray<TSubclassOf<ATokenActor>>& GetTokenActorTypes() const
	{
		return TokenActorTypes;
	}

	const TMap<FString, int32>& GetTokenActorTypeStringToIndexMap() const
	{
		return TokenActorTypeStringToIndexMap;
	}

	/**
	 * 获取可用的StaticMesh映射表
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TMap<FName, FSoftObjectPath>& GetAvailableStaticMeshes() const
	{
		return AvailableStaticMeshes;
	}
	
	/**
	 * 根据简写名称获取StaticMesh软引用
	 */
	FSoftObjectPath GetStaticMeshPath(FName ShortName) const;

	FName GetStaticMeshShortName(const FString& MeshPathStr) const;
	
	/**
	 * 设置可用的StaticMesh映射表（由Editor模块调用）
	 */
	void SetAvailableStaticMeshes(const TMap<FName, FSoftObjectPath>& InStaticMeshes);

private:
	// 当前正在编辑的地图是否为有效的地图
	UPROPERTY()
	bool HasValidMapSave{false};

	UPROPERTY()
	bool bIsSaving{false};

	// 只记录需要更新的Chunk
	TSet<int32> DirtyChunks;

	// 用于在异步任务间共享的进度信息
	struct FSaveProgress
	{
		FThreadSafeBool bIsDirty = false;
		float Progress = 0.0f; 
		FString StatusMessage;

		FThreadSafeCounter CompletedTasks{0};
		int32 TotalTaskCount{0};

		FCriticalSection ProgressLock;
	};

	TSharedPtr<FSaveProgress> CurrentSaveProgress;
	FTimerHandle ProgressUpdateTimerHandle;

	void UpdateSaveProgressUI();

	// 启动时检查是否存在Temp或者Backup文件夹， 如果有，则进行数据恢复; 目前看起来UE似乎自己处理了相关事务，但是不知道原理=。=
	void DataRecover();

	void CreateStandingActors();

	// --------- 数据保存相关 End ---------

public:
	TObjectPtr<UBuildGridMapWindow> GetBuildGridMapWindow() const
	{
		return BuildGridMapWindow;
	}

	TObjectPtr<UBuildGridMapCommandManager> GetCommandManager() const
	{
		return CommandManager;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FGridMapSave& GetEmptyMapSave()
	{
		return EmptyMapSave;
	}

	const FGridMapSave* GetEditingMapSave() const
	{
		return &EditingMapSave;
	}

	FGridMapSave* GetMutEditingMapSave()
	{
		return &EditingMapSave;
	}
	
	const TMap<FHCubeCoord, FSerializableTile>& GetEditingTiles() const
	{
		return EditingTiles;
	}

	TMap<FHCubeCoord, FSerializableTile>& GetMutEditingTiles()
	{
		return EditingTiles;
	}

	const FSerializableTile& GetEditingTile(const FHCubeCoord& InCoord) const
	{
		if (EditingTiles.Contains(InCoord))
		{
			return EditingTiles[InCoord];
		}

		return FSerializableTile::Invalid;
	}

	FSerializableTile* GetMutEditingTile(const FHCubeCoord& InCoord)
	{
		if (EditingTiles.Contains(InCoord))
		{
			return &EditingTiles[InCoord];
		}

		return nullptr;
	}

	void SetEditingTiles(const TMap<FHCubeCoord, FSerializableTile>& InTiles)
	{
		EditingTiles = InTiles;
	}

	void SetEditingTiles(TMap<FHCubeCoord, FSerializableTile>&& InTiles)
	{
		EditingTiles = MoveTemp(InTiles);
	}

	UFUNCTION(BlueprintCallable)
	void CreateGridMapSave(FName InMapName);

	UFUNCTION(BlueprintCallable)
	void SwitchEditingMapSave(const FGridMapSave& InMapSave);

	UFUNCTION(BlueprintCallable)
	void DeleteGridMapSave(/*const FName& InMapName*/);

	/**
	 * 打印当前地图的命令
	 */
	UFUNCTION(BlueprintCallable)
	void DebugCommandHistory();
	
	void ListenToTokenActorChange(UBuildGridMapTokenActorPanel* NewActorPanel);
	
private:
	// Map UI事件
	UFUNCTION()
	void OnMapNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void OnMapTypeSelectionChanged(EGridMapType InMapType);
	void OnMapDrawModeSelectionChanged(EGridMapDrawMode InDrawMode);
	void OnHexTileOrientationSelectionChanged(ETileOrientationFlag InTileOrientation);

	UFUNCTION()
	void OnMapRowTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnMapColumnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnMapRadiusTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnGridSizeXTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnGridSizeYTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnGridRadiusTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// Tile UI事件
	void OnTileEnvSelectionChanged(TObjectPtr<UGridEnvironmentType> NewGridEnvironment);
	UFUNCTION()
	void OnEnvTextureIndexTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void OnAddTokenButtonClick();
	UFUNCTION()
	void OnTokenDeleteClicked(int32 SerializedTokenIndex);
	
	void OnTokenActorTypeChanged(int InActorIndex, const FString& NewTypeString);
	void OnTokenFeaturePropertyChanged(int InActorIndex, int InFeatureIndex, FName InPropertyName, FString NewValue);
	// UFUNCTION()
	// void OnEnvTextureIndexTextChanged(const FText& InNewText);

	// 一些合并处理的指令
	void IntervalChangeTileSize(double InSizeX, double InSizeY);

	// 辅助函数
	FHCubeCoord GetSelectedCoord();
	FString GetChunksRootDir();
	void DeleteFile(const FString& FilePath);
};
