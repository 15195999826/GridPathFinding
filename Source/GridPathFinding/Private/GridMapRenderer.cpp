// Fill out your copyright notice in the Description page of Project Settings.


#include "GridMapRenderer.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "Components/InstancedStaticMeshComponent.h"


// Sets default values
AGridMapRenderer::AGridMapRenderer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	GridWireframe = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridWireframe"));
	GridWireframe->SetupAttachment(SceneRoot);

	BackgroundWireframe = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BackgroundWireframe"));
	BackgroundWireframe->SetupAttachment(SceneRoot);
}

// Called when the game starts or when spawned
void AGridMapRenderer::BeginPlay()
{
	InitializeEnvironmentComponents();
	Super::BeginPlay();
}

// Called every frame
void AGridMapRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGridMapRenderer::SetModel(UGridMapModel* InModel)
{
	if (GridModel)
	{
		GridModel->OnTilesDataBuildCancel.RemoveAll(this);
		GridModel->OnTileModify.RemoveAll(this);
	}

	GridModel = InModel;
	GridModel->OnTilesDataBuildCancel.AddUObject(this, &AGridMapRenderer::OnTilesDataBuildCancel);
	GridModel->OnTileModify.AddUObject(this, &AGridMapRenderer::OnTileModify);
}

void AGridMapRenderer::RenderGridMap()
{
	if (GridModel == nullptr)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("GridModel is nullptr"));
		return;
	}

	// if (RenderConfig.bDrawDefaultMap)
	// {
	// 移除DynamicEnvironmentComponents上全部已经渲染的内容
	for (auto& ISMC : DynamicEnvironmentComponents)
	{
		if (ISMC)
		{
			ISMC->ClearInstances();
		}
	}
	// }

	if (RenderConfig.bDrawBackgroundWireframe)
	{
		DrawBackgroundWireframe();
		// DebugDrawChunks();
	}

	if (GridModel->IsBuildingTilesData())
	{
		TilesDataBuildOverHandle = GridModel->OnTilesDataBuildComplete.AddUObject(this, &AGridMapRenderer::OnTilesDataBuildComplete);
	}
	else
	{
		OnTilesDataBuildComplete();
	}
}

void AGridMapRenderer::ClearGridMap()
{
	BackgroundWireframe->ClearInstances();

	// if (RenderConfig.bDrawDefaultMap)
	// {
	for (auto& ISMC : DynamicEnvironmentComponents)
	{
		if (ISMC)
		{
			ISMC->ClearInstances();
		}
	}
	// }
}

void AGridMapRenderer::RenderTiles()
{
	// Todo: 渲染过程中不允许切换地图、修改数据等操作中断当前的异步任务
	AsyncTask(ENamedThreads::GameThread, [this]()
	{
		auto TilesPtr = GridModel->GetTilesArrayPtr();
		auto TileEnvDataPtr = GridModel->GetTileEnvDataMapPtr();

		int32 RenderCount = 0;
		for (const auto& Tile : *TilesPtr)
		{
			auto Coord = Tile.CubeCoord;
			auto EnvType = Tile.EnvironmentType;
			auto EnvData = TileEnvDataPtr->Find(Coord);

			check(EnvType != UGridEnvironmentType::EmptyEnvTypeID);

			UpdateTile(Coord, UGridEnvironmentType::EmptyEnvTypeID, EnvType, *EnvData);
			RenderCount++;

			if (RenderCount % 1000 == 0)
			{
				FPlatformProcess::Sleep(0.001f); // 每处理1000个格子休眠一小段时间
			}
		}

		OnRenderOver.Broadcast();
	});
}

void AGridMapRenderer::HighLightBackground(const FHCubeCoord& InCoord, bool bHighLight)
{
	int32 InstanceIndex = GridModel->StableGetFullMapGridIterIndex(InCoord);

	if (InstanceIndex == INDEX_NONE)
	{
		return;
	}

	if (bHighLight)
	{
		SetWireFrameColor(BackgroundWireframe, InstanceIndex, RenderConfig.BackgroundWireframeHighlightColor,
		                  RenderConfig.BackgroundDrawLocationOffset.Z, 0.2f);
	}
	else
	{
		SetWireFrameColor(BackgroundWireframe, InstanceIndex, RenderConfig.BackgroundWireframeColor,
		                  RenderConfig.BackgroundDrawLocationOffset.Z, 0.f);
	}
}

void AGridMapRenderer::OnTilesDataBuildCancel()
{
	if (TilesDataBuildOverHandle.IsValid())
	{
		GridModel->OnTilesDataBuildComplete.Remove(TilesDataBuildOverHandle);
	}
}

void AGridMapRenderer::OnTilesDataBuildComplete()
{
	if (TilesDataBuildOverHandle.IsValid())
	{
		GridModel->OnTilesDataBuildComplete.Remove(TilesDataBuildOverHandle);
	}

	RenderTiles();
}

void AGridMapRenderer::DrawBackgroundWireframe()
{
	BackgroundWireframe->ClearInstances();

	auto Config = GridModel->GetMapConfigPtr();

	SetBackgroundWireframeMesh(Config->MapType);

	auto GridRotator = GetGridRotator();
	GridModel->StableForEachMapGrid([this, GridRotator](const FHCubeCoord& Coord, int32 Row, int32 Column)
	{
		// UE_LOG(LogTemp, Log, TEXT("[DrawBackgroundWireframe] Row: %d, Col: %d"), Row, Column);
		auto TileLocation = GridModel->StableCoordToWorld(Coord);
		FTransform WireFrameTransform = FTransform(
			GridRotator, TileLocation + RenderConfig.BackgroundDrawLocationOffset,
			FVector::OneVector);
		auto Index = BackgroundWireframe->AddInstance(WireFrameTransform, true);
		static TArray<float> DefaultColorData{
			RenderConfig.BackgroundWireframeColor.R,
			RenderConfig.BackgroundWireframeColor.G,
			RenderConfig.BackgroundWireframeColor.B,
			RenderConfig.BackgroundWireframeColor.A
		};
		BackgroundWireframe->SetCustomData(Index, DefaultColorData);
	});
}

FRotator AGridMapRenderer::GetGridRotator() const
{
	auto Config = GridModel->GetMapConfigPtr();
	if (Config->MapType == EGridMapType::HEX_STANDARD)
	{
		if (Config->TileOrientation == ETileOrientationFlag::FLAT)
		{
			return FlatRotator;
		}

		if (Config->TileOrientation == ETileOrientationFlag::POINTY)
		{
			return HexPointyRotator;
		}
	}
	else if (Config->MapType == EGridMapType::RECTANGLE_SIX_DIRECTION)
	{
		if (Config->TileOrientation == ETileOrientationFlag::FLAT)
		{
			return FlatRotator;
		}

		if (Config->TileOrientation == ETileOrientationFlag::POINTY)
		{
			return RectPointyRotator;
		}
	}

	return FRotator::ZeroRotator;
}

void AGridMapRenderer::SetWireFrameColor(TObjectPtr<UInstancedStaticMeshComponent> InWireFrame, int Index,
                                         const FLinearColor& InColor, float DefaultHeight, float NewHeight)
{
	TArray<float> CustomData{InColor.R, InColor.G, InColor.B, InColor.A};
	InWireFrame->SetCustomData(Index, CustomData);

	// Todo: 暂不考虑高度缩放的的问题
	// const auto& Tile = GridTiles[Index];
	// float ScaleZ =  (Tile.Height - 1) * TileConfig.RenderHeightScale;
	// ScaleZ = ScaleZ<=0.1f? 1.0f : ScaleZ;

	float ScaleZ = 1.f;

	FTransform Transform;
	InWireFrame->GetInstanceTransform(Index, Transform, true);
	auto NewLocation = Transform.GetLocation();
	NewLocation.Z = ScaleZ * DefaultHeight + NewHeight;
	Transform.SetLocation(NewLocation);
	InWireFrame->UpdateInstanceTransform(Index, Transform, true);
}

void AGridMapRenderer::OnTileModify(EGridMapModelTileModifyType GridMapModelTileModify, const FHCubeCoord& InCoord,
                                    const FTileInfo& OldTileInfo, const FTileInfo& NewTileInfo)
{
	// 处理Tile修改事件
}


void AGridMapRenderer::InitializeEnvironmentComponents()
{
	// 清空现有组件
	for (auto& ISMC : DynamicEnvironmentComponents)
	{
		if (ISMC)
		{
			ISMC->DestroyComponent();
		}
	}
	DynamicEnvironmentComponents.Empty();
	Mesh2ISMCIndexMap.Empty();
	EnvTypeToMeshMap.Empty();
	EnvType2DefaultCustomDataMap.Empty();

	// 获取环境类型设置
	const UGridPathFindingSettings* Settings = GetDefault<UGridPathFindingSettings>();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("无法加载GridPathFindingSettings"));
		return;
	}

	// 按Mesh分组创建组件
	for (const TSoftObjectPtr<UGridEnvironmentType>& TypePtr : Settings->EnvironmentTypes)
	{
		UGridEnvironmentType* EnvironmentType = TypePtr.LoadSynchronous();
		if (!EnvironmentType || EnvironmentType->TypeID.IsNone())
		{
			continue;
		}

		// 加载BuildGridMapMesh
		UStaticMesh* Mesh = EnvironmentType->BuildGridMapMesh.LoadSynchronous();
		if (!Mesh)
		{
			continue;
		}

		// 记录环境类型与Mesh的关系
		EnvTypeToMeshMap.Add(EnvironmentType->TypeID, Mesh);
		EnvType2DefaultCustomDataMap.Add(EnvironmentType->TypeID, EnvironmentType->BuildGridMapMaterialCustomData);

		// 如果已存在相同Mesh的组件，则跳过
		if (Mesh2ISMCIndexMap.Contains(Mesh))
		{
			continue;
		}

		// 创建新的ISM组件
		FName ComponentName = FName(*FString::Printf(TEXT("Mesh_%d"), DynamicEnvironmentComponents.Num()));
		UInstancedStaticMeshComponent* NewComponent = NewObject<UInstancedStaticMeshComponent>(this, ComponentName);
		NewComponent->SetStaticMesh(Mesh);
		NewComponent->SetMaterial(0, EnvironmentType->BuildGridMapMaterial.LoadSynchronous());
		NewComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		NewComponent->SetupAttachment(SceneRoot);
		NewComponent->SetNumCustomDataFloats(2); // 第一个用于区分使用哪个Category， 第二用于指向Texture2dArray的index；
#if WITH_EDITOR
		NewComponent->CreationMethod = EComponentCreationMethod::Native; // 关键设置
		NewComponent->SetFlags(RF_Transactional); // 使组件可在编辑器中序列化
#endif
		NewComponent->RegisterComponent();

		// 将组件添加到Actor的OwnedComponents数组中
		AddOwnedComponent(NewComponent);
		DynamicEnvironmentComponents.Add(NewComponent);
		// 存储组件引用
		Mesh2ISMCIndexMap.Add(Mesh, DynamicEnvironmentComponents.Num() - 1);

		UE_LOG(LogGridPathFinding, Log, TEXT("为Mesh创建了ISM组件，共有%d个环境类型使用此Mesh"),
		       Settings->EnvironmentTypes.FilterByPredicate([Mesh](const TSoftObjectPtr<UGridEnvironmentType>& TypePtr) {
			       return TypePtr.LoadSynchronous() && TypePtr.LoadSynchronous()->BuildGridMapMesh.LoadSynchronous() == Mesh;
			       }).Num());
	}
}

UInstancedStaticMeshComponent* AGridMapRenderer::GetEnvironmentComponent(FName TypeID)
{
	// 通过环境类型ID查找对应的Mesh
	UStaticMesh** FoundMesh = EnvTypeToMeshMap.Find(TypeID);
	if (!FoundMesh || !*FoundMesh)
	{
		return nullptr;
	}

	// 通过Mesh查找对应的组件
	if (Mesh2ISMCIndexMap.Contains(*FoundMesh))
	{
		int32 Index = Mesh2ISMCIndexMap[*FoundMesh];
		return DynamicEnvironmentComponents.IsValidIndex(Index) ? DynamicEnvironmentComponents[Index] : nullptr;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("未找到对应的ISM组件"));
	return nullptr;
}

// Todo: 可以考虑把设置CustomData的功能放到接口类中， 不同项目可以继承接口实现自己的材质球绘制方式
void AGridMapRenderer::UpdateTile(FHCubeCoord Coord, const FName& OldEnvType, const FName& NewEnvType, const FTileEnvData& InTileEnvData)
{
	// 不可能出现Empty 2 Empty的情况
	if (OldEnvType != UGridEnvironmentType::EmptyEnvTypeID)
	{
		auto OldEnvTypeISM = GetEnvironmentComponent(OldEnvType);
		check(OldEnvTypeISM);
		if (OldEnvType != NewEnvType)
		{
			// 移除地块
			// OldEnvTypeISM->RemoveInstance(EnvISMCIndexMap[Coord]);
			// EnvISMCIndexMap.Remove(Coord);

			auto Idx = EnvISMCIndexMap.Find(Coord);
			check(Idx != nullptr);

			int32 RemoveIdx = *Idx;
			OldEnvTypeISM->RemoveInstance(EnvISMCIndexMap[Coord]);
			EnvISMCIndexMap.Remove(Coord);

			for (auto& Pair : EnvISMCIndexMap)
			{
				if (Pair.Value > RemoveIdx)
				{
					Pair.Value--;
				}
			}
		}
		else
		{
			// 此时应当已经创建过了实例， 更新直接材质球CustomData即可
			auto Index = EnvISMCIndexMap[Coord];
			check(EnvType2DefaultCustomDataMap.Contains(OldEnvType))
			const auto& DefaultCustomData = EnvType2DefaultCustomDataMap[OldEnvType];
			OldEnvTypeISM->SetCustomData(Index, {
				                             DefaultCustomData.TextureArrayCategory, static_cast<float>(InTileEnvData.TextureIndex),
				                             DefaultCustomData.Roughness
			                             });
			return;
		}
	}

	if (NewEnvType == UGridEnvironmentType::EmptyEnvTypeID)
	{
		// 新环境类型为空， 则只移除旧的实例
		return;
	}

	// 创建新实例
	UInstancedStaticMeshComponent* NewEnvISM = GetEnvironmentComponent(NewEnvType);
	check(NewEnvISM);
	auto GridRotator = GetGridRotator();
	// 需要创建新的Mesh， 并设置TextureIndex
	auto TileLocation = GridModel->StableCoordToWorld(Coord);
	FTransform MeshTransform = FTransform(
		GridRotator, TileLocation + RenderConfig.BackgroundDrawLocationOffset,
		FVector::OneVector);
	auto Index = NewEnvISM->AddInstance(MeshTransform, true);
	EnvISMCIndexMap.Add(Coord, Index);
	// Todo: 赋值正确的CustomData
	check(EnvType2DefaultCustomDataMap.Contains(NewEnvType))
	const auto& DefaultCustomData = EnvType2DefaultCustomDataMap[NewEnvType];
	NewEnvISM->SetCustomData(Index, {
		                         DefaultCustomData.TextureArrayCategory, static_cast<float>(InTileEnvData.TextureIndex),
		                         DefaultCustomData.Roughness
	                         });
}


void AGridMapRenderer::DebugDrawChunks()
{
	auto World = GetWorld();
	// 根据地图配置， 绘制一个方框表示chunk范围， 中心点绘制一个球
	auto Settings = GetDefault<UGridPathFindingSettings>();
	const int32 ChunkSize = Settings->MapChunkSize.X; // 假设X和Y相同
	const float DebugDrawDuration = 10.0f; // 调试线条显示时间
	const float LineThickness = 2.0f; // 线条粗细

	auto MapConfig = GridModel->GetMapConfigPtr();
	switch (MapConfig->DrawMode)
	{
	case EGridMapDrawMode::BaseOnRowColumn:
		{
			// 计算地图边界
			const auto RowStart = -FMath::FloorToInt(MapConfig->MapSize.X / 2.f);
			const auto RowEnd = FMath::CeilToInt(MapConfig->MapSize.X / 2.f);
			const auto ColumnStart = -FMath::FloorToInt(MapConfig->MapSize.Y / 2.f);
			const auto ColumnEnd = FMath::CeilToInt(MapConfig->MapSize.Y / 2.f);

			// 计算区块数量
			const int32 NumChunksX = FMath::CeilToInt((float)(RowEnd - RowStart) / ChunkSize);
			const int32 NumChunksY = FMath::CeilToInt((float)(ColumnEnd - ColumnStart) / ChunkSize);

			auto DesiredChunkCount = GridModel->StableGetChunkCount();
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			                                 TEXT("DesiredChunkCount: ") + FString::Printf(TEXT("%d, %s"), DesiredChunkCount, DesiredChunkCount == NumChunksX * NumChunksY ? TEXT("OK") : TEXT("Error")));
			// 定义一组颜色用于不同的Chunk
			TArray<FLinearColor> ChunkColors;
			ChunkColors.Add(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)); // 红色
			ChunkColors.Add(FLinearColor(0.0f, 1.0f, 0.0f, 1.0f)); // 绿色
			ChunkColors.Add(FLinearColor(0.0f, 0.0f, 1.0f, 1.0f)); // 蓝色
			ChunkColors.Add(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // 黄色
			ChunkColors.Add(FLinearColor(1.0f, 0.0f, 1.0f, 1.0f)); // 紫色
			ChunkColors.Add(FLinearColor(0.0f, 1.0f, 1.0f, 1.0f)); // 青色
			ChunkColors.Add(FLinearColor(1.0f, 0.5f, 0.0f, 1.0f)); // 橙色
			ChunkColors.Add(FLinearColor(0.5f, 0.0f, 1.0f, 1.0f)); // 紫蓝色

			// 为每个格子设置颜色
			GridModel->StableForEachMapGrid(
				[this, &ChunkColors, NumChunksY, ChunkSize, RowStart, ColumnStart](
				const FHCubeCoord& Coord, int32 Row, int32 Column)
				{
					// 计算区块索引
					int32 ChunkIndex = GridModel->StableGetCoordChunkIndex(Coord);
					// 获取格子的实例索引
					int32 InstanceIndex = GridModel->StableGetFullMapGridIterIndex(Coord);
					if (InstanceIndex != INDEX_NONE)
					{
						// 选择颜色 (循环使用颜色数组)
						int32 ColorIndex = ChunkIndex % ChunkColors.Num();
						FLinearColor ChunkColor = ChunkColors[ColorIndex];

						// 设置格子颜色
						SetWireFrameColor(BackgroundWireframe, InstanceIndex, ChunkColor,
						                  RenderConfig.BackgroundDrawLocationOffset.Z, 0.0f);
					}
				});


			// 遍历每个区块
			for (int32 ChunkRow = 0; ChunkRow < NumChunksX; ++ChunkRow)
			{
				for (int32 ChunkCol = 0; ChunkCol < NumChunksY; ++ChunkCol)
				{
					// 计算区块在地图上的边界坐标
					int32 StartRow = RowStart + ChunkRow * ChunkSize;
					int32 StartCol = ColumnStart + ChunkCol * ChunkSize;
					int32 EndRow = FMath::Min(StartRow + ChunkSize, RowEnd);
					int32 EndCol = FMath::Min(StartCol + ChunkSize, ColumnEnd);
					int32 CenterRow = StartRow + (EndRow - StartRow) / 2;
					int32 CenterCol = StartCol + (EndCol - StartCol) / 2;

					// 将行列坐标转换为世界坐标
					FHCubeCoord TopLeftCoord, TopRightCoord, BottomLeftCoord, BottomRightCoord;
					FHCubeCoord CenterCoord;
					if (MapConfig->TileOrientation == ETileOrientationFlag::FLAT)
					{
						// 计算四个角的Cube坐标
						auto Q1 = StartCol;
						auto R1 = StartRow - (StartCol - (StartCol & 1)) / 2;
						TopLeftCoord = FHCubeCoord(FIntVector(Q1, R1, -Q1 - R1));

						auto Q2 = EndCol - 1;
						auto R2 = StartRow - (Q2 - (Q2 & 1)) / 2;
						TopRightCoord = FHCubeCoord(FIntVector(Q2, R2, -Q2 - R2));

						auto Q3 = StartCol;
						auto R3 = EndRow - 1 - (Q3 - (Q3 & 1)) / 2;
						BottomLeftCoord = FHCubeCoord(FIntVector(Q3, R3, -Q3 - R3));

						auto Q4 = EndCol - 1;
						auto R4 = EndRow - 1 - (Q4 - (Q4 & 1)) / 2;
						BottomRightCoord = FHCubeCoord(FIntVector(Q4, R4, -Q4 - R4));

						auto Q = CenterCol;
						auto R = CenterRow - (CenterCol - (CenterCol & 1)) / 2;
						CenterCoord = FHCubeCoord(FIntVector(Q, R, -Q - R));
					}
					else // POINTY
					{
						// 计算四个角的Cube坐标
						auto Q1 = StartCol - (StartRow - (StartRow & 1)) / 2;
						auto R1 = StartRow;
						TopLeftCoord = FHCubeCoord(FIntVector(Q1, R1, -Q1 - R1));

						auto Q2 = EndCol - 1 - (StartRow - (StartRow & 1)) / 2;
						auto R2 = StartRow;
						TopRightCoord = FHCubeCoord(FIntVector(Q2, R2, -Q2 - R2));

						auto Q3 = StartCol - (EndRow - 1 - (EndRow - 1 & 1)) / 2;
						auto R3 = EndRow - 1;
						BottomLeftCoord = FHCubeCoord(FIntVector(Q3, R3, -Q3 - R3));

						auto Q4 = EndCol - 1 - (EndRow - 1 - (EndRow - 1 & 1)) / 2;
						auto R4 = EndRow - 1;
						BottomRightCoord = FHCubeCoord(FIntVector(Q4, R4, -Q4 - R4));
						auto Q = CenterCol - (CenterRow - (CenterRow & 1)) / 2;
						auto R = CenterRow;
						CenterCoord = FHCubeCoord(FIntVector(Q, R, -Q - R));
					}

					// 转换为世界坐标
					FVector TopLeft = GridModel->StableCoordToWorld(TopLeftCoord);
					FVector TopRight = GridModel->StableCoordToWorld(TopRightCoord);
					FVector BottomLeft = GridModel->StableCoordToWorld(BottomLeftCoord);
					FVector BottomRight = GridModel->StableCoordToWorld(BottomRightCoord);

					// 绘制区块边界
					FColor ChunkColor = FColor::Green;
					DrawDebugLine(World, TopLeft, TopRight, ChunkColor, false, DebugDrawDuration, 0, LineThickness);
					DrawDebugLine(World, TopRight, BottomRight, ChunkColor, false, DebugDrawDuration, 0,
					              LineThickness);
					DrawDebugLine(World, BottomRight, BottomLeft, ChunkColor, false, DebugDrawDuration, 0,
					              LineThickness);
					DrawDebugLine(World, BottomLeft, TopLeft, ChunkColor, false, DebugDrawDuration, 0,
					              LineThickness);

					// 计算区块中心位置
					FVector ChunkCenter = GridModel->StableCoordToWorld(CenterCoord);

					// 计算区块索引
					int32 ChunkIndex = ChunkRow * NumChunksY + ChunkCol;

					// 在区块中心绘制球体和索引文本
					DrawDebugSphere(World, ChunkCenter, 10.0f, 8, FColor::Red, false, DebugDrawDuration);
					DrawDebugString(World, ChunkCenter, FString::Printf(TEXT("Chunk %d"), ChunkIndex),
					                nullptr, FColor::White, DebugDrawDuration);
				}
			}
		}
		break;

	case EGridMapDrawMode::BaseOnRadius:
	case EGridMapDrawMode::BaseOnVolume:
		UE_LOG(LogGridPathFinding, Warning, TEXT("暂不支持在 %s 模式下绘制区块"),
		       *UEnum::GetValueAsString(MapConfig->DrawMode));
		break;
	}
}
