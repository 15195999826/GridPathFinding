// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastNoiseWrapper.h"
#include "HGTypes.h"
#include "GameFramework/Actor.h"
#include "Types/HCubeCoord.h"
#include "HexGrid.generated.h"

class AHexGridModVolume;
class ASeedAIController;

// 用于性能检查
DECLARE_STATS_GROUP(TEXT("HEXGRID_STATS"), STATGROUP_HEXGRID, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("CreateGrid(..)"), STAT_CreateGrid, STATGROUP_HEXGRID);

/*
	Just a rough implementation of a tile.
*/
USTRUCT(BlueprintType)
struct FHexTile
{
	GENERATED_USTRUCT_BODY()

	FHexTile() {};

	/* Hex坐标， RQS表示 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	FHCubeCoord CubeCoord;

	/* 世界坐标 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	FVector WorldPosition {
		FVector::ZeroVector
	};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	EHTileEnvironmentType EnvironmentType{EHTileEnvironmentType::PLAIN};
	
	/* Cost of the tile, for a well execution of the GraphAStar pathfinder it need to have a value of at least 1 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid", meta =(ClampMin = 1))
	float Cost{};

	/* Is this tile a blocking tile? For example a static obstacle. */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	bool bIsBlocking{};

	/**
	 * Cost映射为地形， 而Height表示当前地块的高度
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	float Height{0.f};
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	bool HasAIBooked{false};

	// 只存Stand在当前Tile上的AI
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HexGrid")
	TWeakObjectPtr<AActor> StandingActor;

	bool HasActor() const
	{
		return StandingActor.IsValid();
	}

	friend bool operator==(const FHexTile &A, const FHexTile &B)
	{
		return (A.CubeCoord == B.CubeCoord) && (A.Cost == B.Cost) && (A.bIsBlocking == B.bIsBlocking);
	}

	friend bool operator!=(const FHexTile &A, const FHexTile &B)
	{
		return !(A == B);
	}
};

USTRUCT(BlueprintType)
struct FTheOneFastNoiseSetting
{
	GENERATED_USTRUCT_BODY()

	FTheOneFastNoiseSetting()
		: NoiseType(EFastNoise_NoiseType::Simplex), TheOne(1337), Frequency(0.01f), Interp(EFastNoise_Interp::Quintic), FractalType(EFastNoise_FractalType::FBM), Octaves(3), Lacunarity(2.f), Gain(0.5f), CellularJitter(0.45f), CellularDistanceFunction(EFastNoise_CellularDistanceFunction::Euclidean), CellularReturnType(EFastNoise_CellularReturnType::CellValue)
	{
	};

	UPROPERTY(EditAnywhere)
	EFastNoise_NoiseType NoiseType;

	UPROPERTY(EditAnywhere)
	int32 TheOne;

	UPROPERTY(EditAnywhere)
	float Frequency;

	UPROPERTY(EditAnywhere)
	EFastNoise_Interp Interp;

	UPROPERTY(EditAnywhere)
	EFastNoise_FractalType FractalType;

	UPROPERTY(EditAnywhere)
	int32 Octaves;

	UPROPERTY(EditAnywhere)
	float Lacunarity;

	UPROPERTY(EditAnywhere)
	float Gain;

	UPROPERTY(EditAnywhere)
	float CellularJitter;

	UPROPERTY(EditAnywhere)
	EFastNoise_CellularDistanceFunction CellularDistanceFunction;

	UPROPERTY(EditAnywhere)
	EFastNoise_CellularReturnType CellularReturnType;
};

USTRUCT()
struct FHexGridVolumeData
{
	GENERATED_BODY()

	FHexGridVolumeData(): Size(0), RowStart(0), RowEnd(0), ColumnStart(0), ColumnEnd(0)
	{
	}

	int Size;
	int RowStart;
	int RowEnd;
	int ColumnStart;
	int ColumnEnd;
};

/* Delegate used in the CreateGrid function, executed if bound on each inner loop step. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRightClickHexGrid, int, Row, int, Column, FVector, WorldPosition);

/**
 * Todo: 在超大地图上, 格子内存占用会很大, 思考是否存在优化方案（但寻路效率并不会受到影响）
 */
UCLASS()
class GRIDPATHFINDING_API AHexGrid : public AActor
{
	GENERATED_BODY()

public:
	inline static FRotator FlatRotator = FRotator(0.f, 0.f, 0.f);
	inline static FRotator PointyRotator = FRotator(0.f, 30.f, 0.f);

	static int GetDistance(const FHCubeCoord& A, const FHCubeCoord& B);

	// Sets default values for this actor's properties
	AHexGrid();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UInstancedStaticMeshComponent> HexGridWireframe;
	
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UInstancedStaticMeshComponent> HexGridLand;

	UPROPERTY(VisibleDefaultsOnly)
	TMap<EHTileEnvironmentType, TObjectPtr<UInstancedStaticMeshComponent>> EnvironmentMeshes;

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnRightClickHexGrid OnRightClickHexGrid;
	
	/** Array of HexTiles, in our example we fill it in blueprint with the CreationStepDelegate. */
	UPROPERTY(BlueprintReadWrite)
	TArray<FHexTile> GridTiles;

	/** Array of Cube coordinates that compose the grid. */
	// UPROPERTY(BlueprintReadWrite)
	// TArray<FHCubeCoord> GridCoordinates{};

	// UPROPERTY(BlueprintReadWrite)
	// TMap<FHCubeCoord, int> GridCoordinates{};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	int32 RandomSeed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	float NoiseScale = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	bool CreateMesh{true};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	bool DrawLand{true};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	bool DrawWireframe{true};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	float WireframeOffsetZ{0.5f};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	FLinearColor WireframeDefaultColor{};
	/**
	 * 通过在编辑器中配置它来创建区块
	 * Layout of the tile (i know is very misleading, please read the article)
	 * @see  https://www.redblobgames.com/grids/hexagons/implementation.html#layout
	 */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	FHTileConfig TileConfig{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	FRDHeightAreaConfig RDHeightAreaConfig{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	FTheOneFastNoiseSetting FastNoiseSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	TMap<int, EHTileEnvironmentType> Cost2Environment;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	TMap<EHTileEnvironmentType, FHTileEnvironment> Environments;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "_HexGrid")
	float DefaultHexMeshSize{100.f};
	
protected:
	UPROPERTY()
	UFastNoiseWrapper* FastNoiseWrapper;

	// 仅用于保存高度核心随机结果
	TArray<FHCubeCoord> RdHeightAreaCoords;

	UPROPERTY(BlueprintReadOnly)
	FVector MouseHitLocation;

	UPROPERTY(BlueprintReadOnly)
	int MouseHitRow;

	UPROPERTY(BlueprintReadOnly)
	int MouseHitColumn;

	FHexTile EmptyHexTile;

public:
	void AddTile(const TArray<AHexGridModVolume*>& ModVolumes, const FRotator& Offset,const FRandomStream& RandomStream, int32 Q, int32 R);
	FRotator GetGridRotator() const;

	void UpdateHitLocation(const FVector& InHitLocation);

	/**
	* Create a new grid and fill the CubeCoordinates array.
	* 将Actor放到关卡内，才能看到该按钮
	* @see https://www.redblobgames.com/grids/hexagons/implementation.html#map-shapes
	*/	
	UFUNCTION(CallInEditor, Category = "_HexGrid")
	void CreateGrid();
	
	/**
	 * Convert coordinates from Cube space to World space.
	 * 转换Cube坐标到世界坐标
	 * @see https://www.redblobgames.com/grids/hexagons/#hex-to-pixel
	 */ 
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FVector HexToWorld(const FHCubeCoord &H);

	/**
	 * Convert coordinates from World space to Cube space.
	 * 转换世界坐标到Cube坐标
	 * @see https://www.redblobgames.com/grids/hexagons/#pixel-to-hex
	 */ 
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FHCubeCoord WorldToHex(const FVector &Location);

	/**
	 * Snap a World coordinate to the Grid space.
	 * 将世界坐标转换为最近的Hex坐标
	 */
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FVector SnapToGrid(const FVector &Location);

	/** 
	 * Round from floating-point cube coordinate to integer cube coordinate. 
	 * @see https://www.redblobgames.com/grids/hexagons/#rounding
	 */
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FHCubeCoord HexRound(const FHFractional &F);

	/**
	 * Compare two Cube coordinate. 
	 * @see https://www.redblobgames.com/grids/hexagons/implementation.html#hex-equality
	 */
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	bool HexEqual(const FHCubeCoord &A, const FHCubeCoord &B);

	/**
	 * Return one of the six cube directions.
	 * @see https://www.redblobgames.com/grids/hexagons/#neighbors
	 */
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FHCubeCoord GetDirection(int32 Dir);

	/**
	 * Return the neighbor Cube coordinate in the provided direction. 
	 * @see https://www.redblobgames.com/grids/hexagons/#neighbors
	 */
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FHCubeCoord GetNeighbor(const FHCubeCoord &H, const FHCubeCoord &Dir);

	TArray<FHCubeCoord> GetRangeCoords(const FHCubeCoord& Center, int32 Radius) const;
	
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	TArray<FHexTile> GetRange(const FHCubeCoord& Center, int32 Radius);

	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	bool InRange(const FHCubeCoord& Center, const FHCubeCoord& Target, int32 Radius);

	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	bool IsTileReachable(const FVector &Location);
	
	int GetHexTileIndex(const FHCubeCoord& InCoord) const;
	FHCubeCoord GetHexCoordByIndex(int Index) const;

	const FHexTile& GetHexTile(const FVector& InLocation, bool ErrorIfNotFound = true);
	const FHexTile& GetHexTile(const FHCubeCoord& InCoord, bool ErrorIfNotFound = true);

	FHexTile& GetMutableHexTile(const FHCubeCoord& InCoord);

	int GetDistanceByIndex(int A, int B) const;

	UFUNCTION(BlueprintCallable)
	void SetWireFrameColor(int Index, const FLinearColor& InColor, float NewHeight = 0.f);

private:
	FSixDirections HDirections{};

	TArray<float> WireframeDefaultCustomData;

	TArray<FHexGridVolumeData> RowColumnRanges;

	void CreateTile(const TArray<AHexGridModVolume*>& ModVolumes, const FRandomStream& RandomStream);
	int CreateTileByRowColumn(const TArray<AHexGridModVolume*>& ModVolumes, const FRandomStream& RandomStream, const FRotator& InOffset, const int RowStart, const int32 RowEnd, const int ColumnStart, const int32 ColumnEnd);

	//////// Row Column Functions Start///////
public:
	/**
	 * 只有位置存在Hex才会返回true
	 */
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	bool TryGetHexXYToWorld(int Row, int Column, FVector& OutWorldPosition);
	/**
	 * 总是返回一个有效的位置
	 */ 
	UFUNCTION(BlueprintCallable, Category = "HexGrid")
	FVector HexXYToWorld(int Row, int Column);

	FIntVector2 CoordToXY(const FHCubeCoord& InCoord) const;

protected:
	FHCubeCoord GetHexCoordByXY(int32 Row, int32 Column) const;
	//////// Row Column Functions End///////
};
