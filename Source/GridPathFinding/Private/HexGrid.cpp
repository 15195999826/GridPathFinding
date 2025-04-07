// Fill out your copyright notice in the Description page of Project Settings.


#include "HexGrid.h"
#include "EngineUtils.h"
#include "FastNoiseWrapper.h"
#include "GridPathFinding.h"
#include "LandDynamicMeshActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "HexGridModVolume.h"
#include "HexGridPlaceVolume.h"

// Sets default values
AHexGrid::AHexGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	HexGridWireframe = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("HexGridWireframe"));
	HexGridWireframe->SetupAttachment(SceneRoot);
	
	HexGridLand = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RegularHexTile"));
	HexGridLand->SetupAttachment(SceneRoot);
}

int AHexGrid::GetDistance(const FHCubeCoord& A, const FHCubeCoord& B)
{
	return (FMath::Abs(A.QRS.X - B.QRS.X) + FMath::Abs(A.QRS.Y - B.QRS.Y) + FMath::Abs(A.QRS.Z - B.QRS.Z)) / 2;
}

// Called when the game starts or when spawned
void AHexGrid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHexGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHexGrid::CreateGrid()
{
	SCOPE_CYCLE_COUNTER(STAT_CreateGrid);
	WireframeDefaultCustomData = {WireframeDefaultColor.R, WireframeDefaultColor.G, WireframeDefaultColor.B, WireframeDefaultColor.A};
	
	
	// 获取当前场景中所有的HexGridModVolume
	TArray<AHexGridModVolume*> ModVolumes;
	for (TActorIterator<AHexGridModVolume> It(GetWorld()); It; ++It)
	{
		ModVolumes.Add(*It);
	}

	auto MeshComs = SceneRoot->GetAttachChildren();
	TArray<UInstancedStaticMeshComponent*> MeshComsCache;
	for (auto MeshCom : MeshComs)
	{
		if (auto Mesh = Cast<UInstancedStaticMeshComponent>(MeshCom))
		{
			Mesh->ClearInstances();
			if (Mesh == HexGridLand || Mesh == HexGridWireframe)
			{
				continue;
			}
			MeshComsCache.Add(Mesh);
		}
	}
	
	EnvironmentMeshes.Empty();

	int CacheCursor = 0;
	// 初始化EnvironmentMeshes, 在蓝图中手动创建备用组件
	for (const auto& Pair:Environments)
	{
		if (!Pair.Value.DecorationMesh.IsValid())
		{
			continue;
		}

		if (!MeshComsCache.IsValidIndex(CacheCursor))
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("MeshComsCache is not enough"));
			continue;
		}

		auto GetCom =  MeshComsCache[CacheCursor];
		CacheCursor++;
		GetCom->SetStaticMesh(Pair.Value.DecorationMesh.LoadSynchronous());
		EnvironmentMeshes.Add(Pair.Key, GetCom);
	}

	FRandomStream RandomStream{ RandomSeed };

	// Todo: 先跑通网格-》地图-》移动的流程，再思考地图随机规则， 先用柏林噪声生成地图
	FastNoiseWrapper = NewObject<UFastNoiseWrapper>();
	FastNoiseWrapper->SetupFastNoise(
		FastNoiseSetting.NoiseType,
		FastNoiseSetting.TheOne,
		FastNoiseSetting.Frequency,
		FastNoiseSetting.Interp,
		FastNoiseSetting.FractalType,
		FastNoiseSetting.Octaves,
		FastNoiseSetting.Lacunarity,
		FastNoiseSetting.Gain,
		FastNoiseSetting.CellularJitter,
		FastNoiseSetting.CellularDistanceFunction,
		FastNoiseSetting.CellularReturnType
		);
	
	CreateTile(ModVolumes, RandomStream);	
	
	if (CreateMesh)
	{
		ALandDynamicMeshActor* LandDynamicMeshActor = nullptr;
		for (TActorIterator<ALandDynamicMeshActor> It(GetWorld()); It; ++It)
		{
			It->GenerateLand(GridTiles, GetGridRotator(), TileConfig.TileSize);
			LandDynamicMeshActor = *It;
			break;
		}

		if (LandDynamicMeshActor)
		{
			auto Location = LandDynamicMeshActor->GetActorLocation();
			LandDynamicMeshActor->SetActorLocation(FVector(Location.X, Location.Y, 5.f));
	
			// 需要改变一次位置， 才能使得NavMesh被更新， 官方项目也是这么做的
			LandDynamicMeshActor->SetActorLocation(FVector(Location.X, Location.Y, 1.f));
		}else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("LandDynamicMeshActor is nullptr, 在场景中放入LandDynamicMeshActor"));
		}
	}
}

void AHexGrid::CreateTile(const TArray<AHexGridModVolume*>& ModVolumes, const FRandomStream& RandomStream)
{
	RdHeightAreaCoords.Empty();
	if (TileConfig.HeightRandomType == EHTileHeightRandomType::RDHeightArea)
	{
		const TArray<FHCubeCoord>& AreaCorePossibleIndexes = GetRangeCoords(FHCubeCoord{FIntVector::ZeroValue}, RDHeightAreaConfig.AreaRadius);
	
		TArray<FHCubeCoord> AreaCorePositions;
		for (int i = 0; i < RDHeightAreaConfig.CoreCount; ++i)
		{
			// 保证随机结果与AreaCorePositions中的点不重复，且距离大于等于CoreMinDistance
			bool bIsValid = false;
			int MaxTry = 100;
			FHCubeCoord NewCore;

			while (!bIsValid && MaxTry-- > 0)
			{
				// 随机选择一个可能的索引
				int RandomIndex = RandomStream.RandRange(0, AreaCorePossibleIndexes.Num() - 1);
				NewCore = AreaCorePossibleIndexes[RandomIndex];

				bIsValid = true;
				for (const auto& ExistingCore : AreaCorePositions)
				{
					if (GetDistance(NewCore, ExistingCore) < RDHeightAreaConfig.CoreMinDistance)
					{
						bIsValid = false;
						break;
					}
				}
			}

			if (bIsValid)
			{
				AreaCorePositions.Add(NewCore);
			}
		}
		
		for (const auto& Core : AreaCorePositions)
		{
			// 随机半径
			int Radius = RandomStream.RandRange(RDHeightAreaConfig.MinRadius, RDHeightAreaConfig.MaxRadius);
			auto AreaCoords = GetRangeCoords(Core, Radius);
			for (const auto& Coord : AreaCoords)
			{
				// 随机是否空地
				if (RandomStream.FRand() < RDHeightAreaConfig.EmptyWeight)
				{
					continue;
				}

				RdHeightAreaCoords.AddUnique(Coord);
			}
		}
	}
	
	// https://www.unrealengine.com/en-US/blog/optimizing-tarray-usage-for-performance
	// preallocate array memory
	// R1 = 1 + 6*1
	// R2 = 1 + 6*1 + 6*2
	// R3 = 1 + 6*1 + 6*2 + 6*3
	// R4 = 1 + 6*1 + 6*2 + 6*3 + 6*4
	// R5 = .......
	GridTiles.Empty();
	
	// int32 T = 1.1f; 不会报错 int32 T{1.1f} Ide就会报错
	int32 Size{ 1 };
	auto Radius = TileConfig.Radius;
	for (int32 i{ 1 }; i <= Radius; ++i)
	{
		Size += 6 * i;
	}
	// Reverse 预分配指定数量的元素内存空间
	// GridCoordinates.Reserve(Size);
	GridTiles.Reserve(Size);

	auto Offset = GetGridRotator();

	switch (TileConfig.DrawMode) {
		case EHTileDrawMode::BaseOnRadius:
			{
				for (int32 Q{-Radius}; Q <= Radius; ++Q)
				{
					// Calculate R1
					int32 R1{FMath::Max(-Radius, -Q - Radius)};

					// Calculate R2
					int32 R2{FMath::Min(Radius, -Q + Radius)};

					for (int32 R{R1}; R <= R2; ++R)
					{
						AddTile(ModVolumes, Offset, RandomStream, Q, R);
					}
				}
			}
			
			break;
		case EHTileDrawMode::BaseOnRowColumn:
			{
				const auto RowStart = -FMath::FloorToInt(TileConfig.Row / 2.f);
				const auto RowEnd = FMath::CeilToInt(TileConfig.Row / 2.f);
				const auto ColumnStart = -FMath::FloorToInt(TileConfig.Column / 2.f);
				const auto ColumnEnd = FMath::CeilToInt(TileConfig.Column / 2.f);
				CreateTileByRowColumn(ModVolumes, RandomStream, Offset, RowStart, RowEnd, ColumnStart, ColumnEnd);
			}
			
			break;
		case EHTileDrawMode::BaseOnVolume:
			{
				TArray<AHexGridPlaceVolume*> PlaceVolumes;
				for (TActorIterator<AHexGridPlaceVolume> It(GetWorld()); It; ++It)
				{
					PlaceVolumes.Add(*It);
				}
				// 检查各个Volume之间是否存在重叠，重叠报错
				for (int32 i = 0; i < PlaceVolumes.Num(); ++i)
				{
					for (int32 j = i + 1; j < PlaceVolumes.Num(); ++j)
					{
						if (PlaceVolumes[i]->GetComponentsBoundingBox().Intersect(PlaceVolumes[j]->GetComponentsBoundingBox()))
						{
							UE_LOG(LogGridPathFinding, Error, TEXT("PlaceVolumes[%s] and PlaceVolumes[%s] are intersected"), *PlaceVolumes[i]->GetName(), *PlaceVolumes[j]->GetName());
							return;
						}
					}
				}

				RowColumnRanges.Empty();
				
				// 在每一个PlaceVolume中生成Tile
				for (auto PlaceVolume : PlaceVolumes)
				{
					// 获取区域左上和右下的坐标
					FVector Min, Max;
					auto Center = PlaceVolume->GetActorLocation();
					auto VolumeSize = PlaceVolume->GetComponentsBoundingBox().GetSize();
					Max = Center - VolumeSize / 2;
					Min = Center + VolumeSize / 2;
					// 交换Min和Max的Y值
					Swap(Min.Y, Max.Y);
					
					// Z轴设置到TileConfig.Origin.Z
					Min.Z = TileConfig.Origin.Z;
					Max.Z = TileConfig.Origin.Z;
					// Y值取反, 并且向正Y轴偏移TileConfig.TileSize / 2
					Min.Y += TileConfig.TileSize / 2;
					Max.Y += TileConfig.TileSize / 2;

					// 在坐标Min和Max位置绘制Debug球体
					
					DrawDebugSphere(GetWorld(), Min, 50.0f, 12, FColor::Red, false, 10.0f);
					DrawDebugSphere(GetWorld(), Max, 50.0f, 12, FColor::Blue, false, 10.0f);
					
					// SnapToGrid是将坐标转换为最近的Tile坐标
					auto MinHex = WorldToHex(SnapToGrid(Min));
					auto MaxHex = WorldToHex(SnapToGrid(Max));
					// 于是得到左上的行列和右下的行列
					int32 RowStart, RowEnd, ColumnStart, ColumnEnd;
					const auto& MinXY = CoordToXY(MinHex);
					const auto& MaxXY = CoordToXY(MaxHex);
					RowStart = MinXY.X;
					RowEnd = MaxXY.X;
					ColumnStart = MinXY.Y;
					ColumnEnd = MaxXY.Y;
					FHexGridVolumeData VolumeData;
					VolumeData.RowStart = RowStart;
					VolumeData.RowEnd = RowEnd;
					VolumeData.ColumnStart = ColumnStart;
					VolumeData.ColumnEnd = ColumnEnd;
					RowColumnRanges.Add(VolumeData);
				}

				// 记录行列范围， 当输入Coord时， 判断是否在此范围
				// 排列， Row越小， Column越小，Index越小
				RowColumnRanges.Sort([](const FHexGridVolumeData& A, const FHexGridVolumeData& B) -> bool
				{
					if (A.RowStart == B.RowStart)
					{
						return A.ColumnStart < B.ColumnStart;
					}
					return A.RowStart < B.RowStart;
				});

				for (auto& RCRange : RowColumnRanges)
				{
					// 生成Tile
					RCRange.Size = CreateTileByRowColumn(ModVolumes, RandomStream, Offset, RCRange.RowStart, RCRange.RowEnd, RCRange.ColumnStart, RCRange.ColumnEnd);
				}

				// 测试 读取当前所有GridTiles的Coord， 打印调用GetHexTileIndex
				// for (int32 index = 0; index < GridTiles.Num(); ++index)
				// {
				// 	auto Tile = GridTiles[index];
				// 	auto Coord = Tile.CubeCoord;
				// 	auto GetIndex = GetHexTileIndex(Coord);
				// 	bool bEqual = GetIndex == index;
				// 	UE_LOG(LogGridPathFinding, Log, TEXT("Index: %d, Coord: %s, GetIndex: %d, Equal: %d"), index, *Coord.ToString(), GetIndex, bEqual);
				// }
			}
			break;
	}
}

int AHexGrid::CreateTileByRowColumn(const TArray<AHexGridModVolume*>& ModVolumes, const FRandomStream& RandomStream, const FRotator& InOffset, const int RowStart, const int32 RowEnd, const int ColumnStart, const int32 ColumnEnd)
{
	int CreateCount = 0;
	if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	{
		for (int32 Column{ColumnStart}; Column < ColumnEnd; ++Column)
		{
			for (int32 Row{RowStart}; Row < RowEnd; ++Row)
			{
				auto Q = Column;
				auto R = Row - (Column - (Column & 1)) / 2;
				AddTile(ModVolumes, InOffset, RandomStream, Q, R);
				CreateCount++;
			}
		}
	}
	else if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
	{
		for (int32 Row{ RowStart }; Row < RowEnd; ++Row)
		{
			for (int32 Column{ ColumnStart }; Column < ColumnEnd; ++Column)
			{
				auto Q = Column - (Row - (Row & 1)) / 2;
				auto R = Row;
				AddTile(ModVolumes, InOffset, RandomStream, Q, R);
				CreateCount++;
			}
		}
	}

	return CreateCount;
}

void AHexGrid::AddTile(const TArray<AHexGridModVolume*>& ModVolumes, const FRotator& Offset,const FRandomStream& RandomStream, int32 Q, int32 R)
{
	FHCubeCoord CCoord{FIntVector(Q, R, -Q - R)};
	// Todo: 线框模式， 只绘制线框
	// 世界坐标
	auto TileLocation = HexToWorld(CCoord);

	// 检查是否在ModVolume中
	bool bInModVolume = false;
	for (auto ModVolume : ModVolumes)
	{
		if (ModVolume->EncompassesPoint(TileLocation))
		{
			bInModVolume = true;
			break;
		}
	}
	
	float Cost = TileConfig.MinCost;
	bool IsBlocking = false;
	float Height = TileConfig.MinHeight;
	// Todo: 根据Mod功能执行不一样的效果，目前只有剔除这一种功能，简写
	if (bInModVolume)
	{
		IsBlocking = true;
	}
	else
	{
		// 随机block (Weight表示成为障碍物的概率)
		IsBlocking = TileConfig.BlockWeight <= 0.f
			                  ? false
			                  : TileConfig.BlockWeight >= RandomStream.FRandRange(0.0f, 1.0f);
		switch (TileConfig.HeightRandomType)
		{
			case EHTileHeightRandomType::NONE:
				break;
			case EHTileHeightRandomType::RDHeightArea:
				if (RdHeightAreaCoords.Contains(CCoord))
				{
					// 这里是高地
					Height = TileConfig.MaxHeight;
				}
				break;
		}
		
		switch (TileConfig.CostRandomType)
		{
			case EHTileCostRandomType::NONE:
				break;
			case EHTileCostRandomType::NOISE:
				{
					auto Noise = FastNoiseWrapper->GetNoise2D(TileLocation.X, TileLocation.Y);

					if (Noise > 1.f || Noise < -1.f)
					{
						UE_LOG(LogGridPathFinding, Error, TEXT("Noise is out of range: %f"), Noise);
						Noise = 1.f;
					}
					else if (Noise < -1.f)
					{
						UE_LOG(LogGridPathFinding, Error, TEXT("Noise is out of range: %f"), Noise);
						Noise = -1.f;
					}

					// 将Noise值映射到TileConfig.MinCost, TileConfig.MaxCost之间
					// 将噪声值映射到0到1之间
					float normalizedNoise = (Noise + 1) / 2;
					// 使用线性插值将噪声值映射到TileConfig.MinCost和TileConfig.MaxCost之间
					if (TileConfig.bDiscreteCost)
					{
						Cost = FMath::RoundToInt(FMath::Lerp(TileConfig.MinCost, TileConfig.MaxCost, normalizedNoise));
					}
					else
					{
						Cost = FMath::Lerp(TileConfig.MinCost, TileConfig.MaxCost, normalizedNoise);
					}

					// // 目前，噪声算法中，高度和Cost使用了一个噪声值，Todo: 考虑是否应该分开
					//
					// float NoiseHeight = FMath::Lerp(TileConfig.MinHeight, TileConfig.MaxHeight, normalizedNoise);
					// Height = IsBlocking ? TileConfig.BlockHeight : NoiseHeight * NoiseScale;
				}
				break;
			case EHTileCostRandomType::SIMPLE:
				{
					if (TileConfig.bDiscreteCost)
					{
						Cost = FMath::RoundToInt(RandomStream.FRandRange(TileConfig.MinCost, TileConfig.MaxCost));
					}
					else
					{
						Cost = RandomStream.FRandRange(TileConfig.MinCost, TileConfig.MaxCost);
					}
				}
				break;
		}
	}
	
	
	check(Cost >= 1.f);
	// 创建HexTile
	FHexTile Tile;
	Tile.CubeCoord = CCoord;
	Tile.WorldPosition = TileLocation;
	Tile.Cost = Cost;
	Tile.bIsBlocking = IsBlocking;
	Tile.Height = Height;
	
	// UE_LOG(LogGridPathFinding, Log, TEXT("Cost: %f, CCoord: %s"), Cost, *CCoord.ToString());
	GridTiles.Add(Tile);

	if (bInModVolume)
	{
		return;
	}
	
	auto ScaleXY = TileConfig.TileSize / DefaultHexMeshSize;
	auto Transform = FTransform(Offset, TileLocation, FVector(ScaleXY, ScaleXY, 1));
	float ScaleZ =  (Height - 1) * TileConfig.RenderHeightScale;
	ScaleZ = ScaleZ<=0.1f? 1.0f : ScaleZ;
	
	if (DrawWireframe)
	{
		FTransform WireFrameTransform = FTransform(Offset, TileLocation + FVector(0.f, 0.f, ScaleZ * TileConfig.BaseRenderHeight + WireframeOffsetZ), FVector(ScaleXY, ScaleXY, 1));
		auto Index = HexGridWireframe->AddInstance(WireFrameTransform, true);
		HexGridWireframe->SetCustomData(Index, WireframeDefaultCustomData);
	}

	if (DrawLand)
	{
		// 目前默认的Tile的Mesh是以100为标准制作的， 因此这里按照100进行缩放
		// Block的设置Z的Scale为BlockHeight， 普通区块设置为Cost * 2.f
		Tile.EnvironmentType = IsBlocking?EHTileEnvironmentType::OBSTACLE : Cost2Environment[FMath::RoundToInt(Cost)];
		Transform.SetScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));
		auto Index = HexGridLand->AddInstance(Transform, true);

		const auto& Environment = Environments[Tile.EnvironmentType];
		TArray<float> CustomData {Environment.CustomData.R, Environment.CustomData.G, Environment.CustomData.B, Environment.CustomData.A};
		HexGridLand->SetCustomData(Index, CustomData);
		if (Environment.DecorationMesh.IsValid())
		{
			auto EnvironmentMesh = EnvironmentMeshes[Tile.EnvironmentType];
			auto RDCount = RandomStream.RandRange(1, Environment.DecorationMaxCount);
			for (int32 i = 0; i < RDCount; ++i)
			{
				FTransform ItemTransform;
				if (Environment.bRandomDecorationRotation)
				{
					ItemTransform.SetRotation(FQuat(FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f)));
				}

				if (Environment.bRandomDecorationLocation)
				{
					// 随机XY平面的位置， 范围是TileSize的一半
					auto RandomX = RandomStream.FRandRange(-TileConfig.TileSize / 2.f, TileConfig.TileSize / 2.f);
					auto RandomY = RandomStream.FRandRange(-TileConfig.TileSize / 2.f, TileConfig.TileSize / 2.f);
					auto RandomLocation = FVector(RandomX, RandomY, 0.f);
					ItemTransform.SetLocation(TileLocation + RandomLocation + FVector(0.f, 0.f, ScaleZ * TileConfig.BaseRenderHeight));
				}
				else
				{
					ItemTransform.SetLocation(TileLocation+ FVector(0.f, 0.f, ScaleZ * TileConfig.BaseRenderHeight));
				}

				ItemTransform.SetScale3D(FVector(Environment.DecorationScale));
				EnvironmentMesh->AddInstance(ItemTransform, true);
			}
		}
	}
}

FRotator AHexGrid::GetGridRotator() const
{
	if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	{
		return FlatRotator;
	}

	if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
	{
		return  PointyRotator;
	}

	return FRotator::ZeroRotator;
}

void AHexGrid::UpdateHitLocation(const FVector& InHitLocation)
{
	MouseHitLocation = InHitLocation;

	if (TileConfig.DrawMode == EHTileDrawMode::BaseOnRadius)
	{
		// Todo: 未实现
		MouseHitColumn = 0;
		MouseHitRow = 0;
	}
	else
	{
		auto HexCoord = WorldToHex(InHitLocation);
		const FIntVector2& XY = CoordToXY(HexCoord);
		MouseHitRow = XY.X;
		MouseHitColumn = XY.Y;
		if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
		{
			MouseHitColumn = HexCoord.QRS.X;
			MouseHitRow = HexCoord.QRS.Y + (HexCoord.QRS.X - (HexCoord.QRS.X & 1)) / 2;
		}
		else if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
		{
			MouseHitColumn = HexCoord.QRS.X + (HexCoord.QRS.Y - (HexCoord.QRS.Y & 1)) / 2;
			MouseHitRow = HexCoord.QRS.Y;
		}
	}
}

bool AHexGrid::TryGetHexXYToWorld(int Row, int Column, FVector& OutWorldPosition)
{
	auto Key = GetHexCoordByXY(Row, Column);
	if (GetHexTileIndex(Key) != INDEX_NONE)
	{
		OutWorldPosition = HexToWorld(Key);
		return true;
	}

	return false;
}

FVector AHexGrid::HexXYToWorld(int Row, int Column)
{
	FHCubeCoord CCoord = GetHexCoordByXY(Row, Column);
	return HexToWorld(CCoord);
}

FIntVector2 AHexGrid::CoordToXY(const FHCubeCoord& InCoord) const
{
	if (TileConfig.DrawMode == EHTileDrawMode::BaseOnRadius)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("CoordToXY BaseOnRadius mode, 尚未实现"));
	}
	
	if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	{
		return FIntVector2{InCoord.QRS.Y + (InCoord.QRS.X - (InCoord.QRS.X & 1)) / 2, InCoord.QRS.X};
	}

	if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
	{
		return FIntVector2{InCoord.QRS.Y, InCoord.QRS.X + (InCoord.QRS.Y - (InCoord.QRS.Y & 1)) / 2};
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("CoordToXY Error"));
	return FIntVector2::ZeroValue;
}

FVector AHexGrid::HexToWorld(const FHCubeCoord &H)
{
	// Set the layout orientation
	FHTileOrientation TileOrientation;
	if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	{
		TileOrientation = HFlatTopLayout;
	}
	else
	{
		TileOrientation = HPointyLayout;
	}

	float x = ((TileOrientation.f0 * H.QRS.X) + (TileOrientation.f1 * H.QRS.Y)) * TileConfig.TileSize;
	float y = ((TileOrientation.f2 * H.QRS.X) + (TileOrientation.f3 * H.QRS.Y)) * TileConfig.TileSize;

	return FVector(x + TileConfig.Origin.X, y + TileConfig.Origin.Y, TileConfig.Origin.Z);
}

FHCubeCoord AHexGrid::WorldToHex(const FVector &Location)
{
	// Set the layout orientation
	FHTileOrientation TileOrientation;
	if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	{
		TileOrientation = HFlatTopLayout;
	}
	else
	{
		TileOrientation = HPointyLayout;
	}

	FVector InternalLocation{ FVector((Location.X - TileConfig.Origin.X) / TileConfig.TileSize,
									  (Location.Y - TileConfig.Origin.Y) / TileConfig.TileSize,
									  (Location.Z - TileConfig.Origin.Z))	// Z is useless here.
	};

	float q = ((TileOrientation.b0 * InternalLocation.X) + (TileOrientation.b1 * InternalLocation.Y));
	float r = ((TileOrientation.b2 * InternalLocation.X) + (TileOrientation.b3 * InternalLocation.Y));
	
	FVector v{ (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT) ? FVector(q, -q - r, r) : FVector(q, r, -q - r) };

	return HexRound(FHFractional(v));
}

FVector AHexGrid::SnapToGrid(const FVector &Location)
{
	const double TempZ{Location.Z};
	FVector Result{ HexToWorld(WorldToHex(Location)) };
	Result.Z = TempZ;
	return Result;
}

FHCubeCoord AHexGrid::HexRound(const FHFractional &F)
{
	int32 q{ int32(FMath::RoundToDouble(F.QRS.X)) };
	int32 r{ int32(FMath::RoundToDouble(F.QRS.Y)) };
	int32 s{ int32(FMath::RoundToDouble(F.QRS.Z)) };

	const double q_diff{ FMath::Abs(q - F.QRS.X) };
	const double r_diff{ FMath::Abs(r - F.QRS.Y) };
	const double s_diff{ FMath::Abs(s - F.QRS.Z) };

	if ((q_diff > r_diff) && (q_diff > s_diff))
	{
		q = -r - s;
	}
	else if (r_diff > s_diff)
	{
		r = -q - s;
	}
	else
	{
		s = -q - r;
	}

	return FHCubeCoord{ FIntVector(q, r, s) };
}

bool AHexGrid::HexEqual(const FHCubeCoord &A, const FHCubeCoord &B)
{
	return A == B;
}

FHCubeCoord AHexGrid::GetDirection(int32 Dir)
{
	check(Dir < HDirections.Directions.Num());
	return HDirections.Directions[Dir];
}

FHCubeCoord AHexGrid::GetNeighbor(const FHCubeCoord &H, const FHCubeCoord &Dir)
{
	return H + Dir;
}

TArray<FHCubeCoord> AHexGrid::GetRangeCoords(const FHCubeCoord& Center, int32 Radius) const
{
	TArray<FHCubeCoord> Result;

	for (int32 dq = -Radius; dq <= Radius; ++dq)
	{
		for (int32 dr = FMath::Max(-Radius, -dq - Radius); dr <= FMath::Min(Radius, -dq + Radius); ++dr)
		{
			int32 ds = -dq - dr;
			Result.Add(FHCubeCoord{FIntVector(Center.QRS.X + dq, Center.QRS.Y + dr, Center.QRS.Z + ds)});
		}
	}
	return Result;
}

TArray<FHexTile> AHexGrid::GetRange(const FHCubeCoord& Center, int32 Radius)
{
	TArray<FHexTile> Result;

	const TArray<FHCubeCoord>& Coords = GetRangeCoords(Center, Radius);
	for (const auto& Coord : Coords)
	{
		auto Index = GetHexTileIndex(Coord);
		if (Index != INDEX_NONE)
		{
			Result.Add(GridTiles[Index]);
		}
	}
	
	return Result;
}


bool AHexGrid::InRange(const FHCubeCoord& Center, const FHCubeCoord& Target, int32 Radius)
{
	return FMath::Abs(Center.QRS.X - Target.QRS.X) <= Radius &&
		FMath::Abs(Center.QRS.Y - Target.QRS.Y) <= Radius &&
		FMath::Abs(Center.QRS.Z - Target.QRS.Z) <= Radius;
}

bool AHexGrid::IsTileReachable(const FVector& Location)
{
	auto HexCoord = WorldToHex(Location);
	auto Index = GetHexTileIndex(HexCoord);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	
	auto Tile = GridTiles[Index];
	return !Tile.bIsBlocking;
}

int AHexGrid::GetHexTileIndex(const FHCubeCoord& InCoord) const
{
	auto Q = InCoord.QRS.X;
	auto R = InCoord.QRS.Y;
	int32 PassedCount;

	switch (TileConfig.DrawMode) {
		case EHTileDrawMode::BaseOnRadius:
			{
				// Q,R,S中的最大值就是距离中心的距离
				if (FMath::Abs(Q) > TileConfig.Radius || FMath::Abs(R) > TileConfig.Radius || FMath::Abs(-Q - R) > TileConfig.Radius)
				{
					return INDEX_NONE;
				}
		
				// 根据AddTile的添加顺序，逆推
				auto Radius = TileConfig.Radius;
		
				int32 Start{FMath::Max(-Radius, -Q - Radius)};
		
				auto DeltaQ = Q + Radius;
		
				// 实际是等差数列，优化为下列的数学公式
				if (DeltaQ <= Radius + 1)
				{
					PassedCount = ( Radius + 1 + Radius + DeltaQ) * (DeltaQ) / 2;
				}
				else
				{
					PassedCount = (Radius + 1 + 2 * Radius + 1) * (Radius + 1) / 2;
					PassedCount += (5 * Radius - DeltaQ + 2) * (DeltaQ - Radius - 1) / 2;
				}
		
				return PassedCount + R - Start;
			}
		case EHTileDrawMode::BaseOnRowColumn:
			{
				const auto RowStart = -FMath::FloorToInt(TileConfig.Row / 2.f);
				const auto RowEnd = FMath::CeilToInt(TileConfig.Row / 2.f);
				const auto ColumnStart = -FMath::FloorToInt(TileConfig.Column / 2.f);
				const auto ColumnEnd = FMath::CeilToInt(TileConfig.Column / 2.f);
				if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
				{
					auto Row = R + (Q - (Q & 1)) / 2;
					if (Q < ColumnStart || Q >= ColumnEnd || Row < RowStart || Row >= RowEnd)
					{
						return INDEX_NONE;
					}
		
					auto DeltaCol = Q - ColumnStart;
					PassedCount = DeltaCol * TileConfig.Row;
		
					return PassedCount + Row - RowStart;
				}
	
				if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
				{
					auto Col = Q + (R - (R&1)) / 2;
					if (Col < ColumnStart || Col >= ColumnEnd || R < RowStart || R >= RowEnd)
					{
						return INDEX_NONE;
					}
		
					auto DeltaRow = R - RowStart;
					PassedCount = DeltaRow * TileConfig.Column;
		
					return PassedCount + Col - ColumnStart;
				}
			}
		case EHTileDrawMode::BaseOnVolume:
			{
				int DesiredRow = INDEX_NONE;
				int DesiredColumn = INDEX_NONE;
				if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
				{
					DesiredColumn = InCoord.QRS.X;
					DesiredRow = InCoord.QRS.Y + (InCoord.QRS.X - (InCoord.QRS.X & 1)) / 2;
				}
				else if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
				{
					DesiredColumn = InCoord.QRS.X + (InCoord.QRS.Y - (InCoord.QRS.Y & 1)) / 2;
					DesiredRow = InCoord.QRS.Y;
				}
				PassedCount = 0;
				// 查找这个DesiredRow, DesiredColumn落在哪个Volume中
				for (int32 i = 0; i < RowColumnRanges.Num(); ++i)
				{
					const auto& Range = RowColumnRanges[i];
					if (DesiredRow >= Range.RowStart && DesiredRow < Range.RowEnd && DesiredColumn >= Range.ColumnStart && DesiredColumn < Range.ColumnEnd)
					{
						auto DeltaRow = DesiredRow - Range.RowStart;
						auto DeltaColumn = DesiredColumn - Range.ColumnStart;
						// auto Ret = PassedCount + DeltaRow * (Range.ColumnEnd - Range.ColumnStart) + DeltaColumn;
						// if (Ret >= GridTiles.Num())
						// {
						// 	// 打印Range数据
						// 	UE_LOG(LogGridPathFinding, Error, TEXT("RowStart: %d, RowEnd: %d, ColumnStart: %d, ColumnEnd: %d, Size: %d"), Range.RowStart, Range.RowEnd, Range.ColumnStart, Range.ColumnEnd, Range.Size);
						// 	UE_LOG(LogGridPathFinding, Error, TEXT("Index is out of range, DesiredRow: %d, DesiredColumn: %d, PassedCount: %d, DeltaRow: %d, DeltaColumn: %d"), DesiredRow, DesiredColumn, PassedCount, DeltaRow, DeltaColumn);
						// }
						
						return PassedCount + DeltaRow * (Range.ColumnEnd - Range.ColumnStart) + DeltaColumn;
					}
					
					PassedCount += Range.Size;
				}

				return INDEX_NONE;
			}
	}
	
	
	UE_LOG(LogGridPathFinding, Error, TEXT("[AHexGrid] GetHexTileIndex Failed, Return -1"));
	return INDEX_NONE;
}

FHCubeCoord AHexGrid::GetHexCoordByIndex(int Index) const
{
	if (Index < 0 || Index >= GridTiles.Num())
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[AHexGrid] GetHexCoordByIndex 查询越界的Index, Index: %d"), Index);
		return FHCubeCoord{FIntVector::ZeroValue};;
	}

	return GridTiles[Index].CubeCoord;
	
	// if (TileConfig.DrawMode == EHTileDrawMode::BaseOnRadius)
	// {
	// 	// 从Index逆推出Q,R
	// 	auto Radius = TileConfig.Radius;
	// 	int32 DeltaQ = 0;
	// 	int32 PassedCount = 0;
	//
	// 	// Find the maximum DeltaQ that satisfies PassedCount <= Index
	// 	while (true)
	// 	{
	// 		int32 NextPassedCount;
	// 		if (DeltaQ <= Radius + 1)
	// 		{
	// 			NextPassedCount = (Radius + 1 + Radius + DeltaQ) * (DeltaQ) / 2;
	// 		}
	// 		else
	// 		{
	// 			NextPassedCount = (Radius + 1 + 2 * Radius + 1) * (Radius + 1) / 2;
	// 			NextPassedCount += (5 * Radius - DeltaQ + 2) * (DeltaQ - Radius - 1) / 2;
	// 		}
	//
	// 		if (NextPassedCount > Index)
	// 		{
	// 			break;
	// 		}
	//
	// 		PassedCount = NextPassedCount;
	// 		DeltaQ++;
	// 	}
	// 	// Calculate Q
	// 	int32 Q = DeltaQ - Radius - 1;
	// 	int32 Start{FMath::Max(-Radius, -Q - Radius)};
	//
	// 	// Calculate R
	// 	int32 R = Index - PassedCount + Start;
	// 	return FHCubeCoord{FIntVector(Q, R, -Q - R)};
	// }
	//
	// const auto RowStart = -FMath::FloorToInt(TileConfig.Row / 2.f);
	// const auto ColumnStart = -FMath::FloorToInt(TileConfig.Column / 2.f);
	// if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	// {
	// 	int32 DeltaCol = Index / TileConfig.Row;
	// 	int32 Col = DeltaCol + ColumnStart;
	// 	int32 Row = Index % TileConfig.Row + RowStart;
	// 	// UE_LOG(LogGridPathFinding, Log, TEXT("Row: %d, Col: %d"), Row, Col);
	// 	int32 R = Row - (Col - (Col & 1)) / 2;
	// 	return FHCubeCoord{FIntVector(Col, R, -Col - R)};
	// }
	//
	// if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
	// {
	// 	int32 DeltaRow = Index / TileConfig.Column;
	// 	int32 Row = DeltaRow + RowStart;
	// 	int32 Col = Index % TileConfig.Column + ColumnStart;
	// 	// UE_LOG(LogGridPathFinding, Log, TEXT("Row: %d, Col: %d"), Row, Col);
	// 	int32 Q = Col - (Row - (Row & 1)) / 2;
	// 	return FHCubeCoord{FIntVector(Q, Row, -Q - Row)};
	// }
	//
	// UE_LOG(LogGridPathFinding, Error, TEXT("[AHexGrid] GetHexCoordByIndex Failed, HexGrid配置错误"));
	// return FHCubeCoord{FIntVector::ZeroValue};
}

const FHexTile& AHexGrid::GetHexTile(const FVector& InLocation, bool ErrorIfNotFound)
{
	return GetHexTile(WorldToHex(InLocation), ErrorIfNotFound);
}

const FHexTile& AHexGrid::GetHexTile(const FHCubeCoord& InCoord, bool ErrorIfNotFound)
{
	auto Index = GetHexTileIndex(InCoord);
	if (Index != INDEX_NONE)
	{
		return GridTiles[Index];
	}

	if (ErrorIfNotFound)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[AHexGrid] GetHexTile Failed, Return EmptyHexTile"));
	}
	
	return EmptyHexTile;
}

FHexTile& AHexGrid::GetMutableHexTile(const FHCubeCoord& InCoord)
{
	auto Index = GetHexTileIndex(InCoord);
	if (Index != INDEX_NONE)
	{
		return GridTiles[Index];
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[AHexGrid] GetMutableHexTile Failed, Return EmptyHexTile"));
	return EmptyHexTile;
}

int AHexGrid::GetDistanceByIndex(int A, int B) const
{
	if (GridTiles.IsValidIndex(A) && GridTiles.IsValidIndex(B))
	{
		return GetDistance(GridTiles[A].CubeCoord, GridTiles[B].CubeCoord);
	}

	return INT_MAX;
}

void AHexGrid::SetWireFrameColor(int Index, const FLinearColor& InColor, float NewHeight)
{
	TArray<float> CustomData {InColor.R, InColor.G, InColor.B, InColor.A};
	HexGridWireframe->SetCustomData(Index, CustomData);
	const auto& Tile = GridTiles[Index];
	float ScaleZ =  (Tile.Height - 1) * TileConfig.RenderHeightScale;
	ScaleZ = ScaleZ<=0.1f? 1.0f : ScaleZ;
	FTransform Transform ;
	HexGridWireframe->GetInstanceTransform(Index, Transform, true);
	Transform.SetLocation(Tile.WorldPosition + FVector(0.f, 0.f, ScaleZ * TileConfig.BaseRenderHeight + WireframeOffsetZ + NewHeight));
	HexGridWireframe->UpdateInstanceTransform(Index,Transform, true);
}

FHCubeCoord AHexGrid::GetHexCoordByXY(int32 Row, int32 Column) const
{
	if (TileConfig.DrawMode == EHTileDrawMode::BaseOnRadius)
	{
		return FHCubeCoord{FIntVector(Column, Row, -Column - Row)};
	}

	if (TileConfig.TileOrientation == EHTileOrientationFlag::FLAT)
	{
		return FHCubeCoord{FIntVector(Column, Row - (Column - (Column & 1)) / 2, Row)};
	}
	
	if (TileConfig.TileOrientation == EHTileOrientationFlag::POINTY)
	{
		return FHCubeCoord{FIntVector(Column - (Row - (Row & 1)) / 2, Row, Column)};
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[AHexGrid] GetHexCoordByXY Failed, Return ZeroValue"));
	return FHCubeCoord{FIntVector::ZeroValue};
}

