// Fill out your copyright notice in the Description page of Project Settings.


#include "GridPathFindingBlueprintFunctionLib.h"

#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "HGTypes.h"
#include "JsonObjectConverter.h"

TArray<FName> UGridPathFindingBlueprintFunctionLib::GetAllMapSaveNames()
{
	auto Settings = GetDefault<UGridPathFindingSettings>();
	TArray<FName> MapNames;
	TArray<FString> FileNames;
	FString SavePath = FPaths::ProjectContentDir() / Settings->MapSaveFolder;
	IFileManager::Get().FindFiles(FileNames, *SavePath, TEXT("*.txt"));

	// 地图文件名格式为 MapName_Map.txt
	for (const FString& FileName : FileNames)
	{
		FString MapName = FPaths::GetBaseFilename(FileName);
		if (MapName.Contains(TEXT("_Map")))
		{
			MapName.RemoveFromEnd(TEXT("_Map"));
			MapNames.Add(FName(*MapName));
		}
	}

	return MapNames;
}

FGridMapSave UGridPathFindingBlueprintFunctionLib::LoadGridMapSave(FName InMapName)
{
	auto Settings = GetDefault<UGridPathFindingSettings>();
	FString SavePath = FPaths::ProjectContentDir() / Settings->MapSaveFolder / (InMapName.ToString() + TEXT("_Map.txt"));
	FString JsonString;
	FFileHelper::LoadFileToString(JsonString, *SavePath);
	FGridMapSave GridMapSave;
	FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &GridMapSave, 0, 0);

	UE_LOG(LogGridPathFinding, Log, TEXT("Load Map Save: %s, CreateTime: %s"), *GridMapSave.MapName.ToString(), *GridMapSave.CreateTime.ToString());
	return GridMapSave;
}

#include "Misc/Base64.h"

FString UGridPathFindingBlueprintFunctionLib::GGB_SerializeSaveData(FGGB_SaveData Data)
{
	TArray<uint8> BinaryData;
	FMemoryWriter MemoryWriter(BinaryData, true);
	MemoryWriter.SetIsSaving(true);
	MemoryWriter << Data;

	return FBase64::Encode(BinaryData);
}

FGGB_SaveData UGridPathFindingBlueprintFunctionLib::GGB_DeSerializeSaveData(FString Base64Str)
{
	FGGB_SaveData Data;

	if (!Base64Str.IsEmpty())
	{
		TArray<uint8> BinaryData;
		BinaryData.SetNumUninitialized(Base64Str.Len());

		if (FBase64::Decode(Base64Str, BinaryData))
		{
			FMemoryReader MemoryReader(BinaryData, true);
			MemoryReader.SetIsLoading(true);
			MemoryReader << Data;

			UE_LOG(LogGridPathFinding, Log, TEXT("UGGBHelper::GGB_SerializeSaveData Base64Str.Len(%i) Data.HISM[%i] Data.Class(%s)"), Base64Str.Len(), Data.HISM.Num(), IsValid(Data.Class) ? *Data.Class->GetFName().ToString() : TEXT("nullptr"));
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("UGGBHelper::GGB_SerializeSaveData FBase64::Decode FAILED!!!"));
		}
	}

	return Data;
}

FString UGridPathFindingBlueprintFunctionLib::SerializeGridMapTiles(FGridMapTilesSave TilesSave)
{
	TArray<uint8> BinaryData;
	FMemoryWriter MemoryWriter(BinaryData, true);
	MemoryWriter.SetIsSaving(true);
	MemoryWriter << TilesSave;

	return FBase64::Encode(BinaryData);
}

FGridMapTilesSave UGridPathFindingBlueprintFunctionLib::DeserializeGridMapTiles(const FString& Base64Str)
{
	FGridMapTilesSave TilesSave;

	if (!Base64Str.IsEmpty())
	{
		TArray<uint8> BinaryData;
		if (FBase64::Decode(Base64Str, BinaryData))
		{
			FMemoryReader MemoryReader(BinaryData, true);
			MemoryReader.SetIsLoading(true);
			MemoryReader << TilesSave;

			UE_LOG(LogGridPathFinding, Log, TEXT("UGridPathFindingBlueprintFunctionLib::DeserializeGridMapTiles: Successfully deserialized %d tiles (Version: %d)"),
			       TilesSave.GridTiles.Num(), TilesSave.Version);
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("UGridPathFindingBlueprintFunctionLib::DeserializeGridMapTiles: Base64 decode failed!"));
		}
	}
	else
	{
		UE_LOG(LogGridPathFinding, Warning, TEXT("UGridPathFindingBlueprintFunctionLib::DeserializeGridMapTiles: Empty Base64 string provided"));
	}

	return TilesSave;
}

bool UGridPathFindingBlueprintFunctionLib::SaveGridMapTilesToFile(const FGridMapTilesSave& TilesSave, const FString& FilePath)
{
	// 序列化为二进制数据
	TArray<uint8> BinaryData;
	FMemoryWriter MemoryWriter(BinaryData, true);
	MemoryWriter.SetIsSaving(true);

	// 因为需要序列化const对象，创建一个非const副本
	FGridMapTilesSave TilesSaveCopy = TilesSave;
	MemoryWriter << TilesSaveCopy;

	// 将二进制数据保存到文件
	bool bSuccess = FFileHelper::SaveArrayToFile(BinaryData, *FilePath);
	if (bSuccess)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("UGridPathFindingBlueprintFunctionLib::SaveGridMapTilesToFile: Successfully saved %d tiles to %s"),
		       TilesSave.GridTiles.Num(), *FilePath);
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("UGridPathFindingBlueprintFunctionLib::SaveGridMapTilesToFile: Failed to save to %s"), *FilePath);
	}

	return bSuccess;
}

bool UGridPathFindingBlueprintFunctionLib::LoadGridMapTilesFromFile(const FString& FilePath, FGridMapTilesSave& OutTilesSave)
{
	TArray<uint8> BinaryData;

	// 从文件加载二进制数据
	bool bSuccess = FFileHelper::LoadFileToArray(BinaryData, *FilePath);
	if (bSuccess && BinaryData.Num() > 0)
	{
		// 反序列化二进制数据
		FMemoryReader MemoryReader(BinaryData, true);
		MemoryReader.SetIsLoading(true);
		MemoryReader << OutTilesSave;

		UE_LOG(LogGridPathFinding, Log, TEXT("UGridPathFindingBlueprintFunctionLib::LoadGridMapTilesFromFile: Successfully loaded %d tiles from %s"),
		       OutTilesSave.GridTiles.Num(), *FilePath);

		return true;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("UGridPathFindingBlueprintFunctionLib::LoadGridMapTilesFromFile: Failed to load from %s"), *FilePath);

	return false;
}

bool UGridPathFindingBlueprintFunctionLib::IsCoordInMapArea(const FGridMapConfig& InMapConfig, const FHCubeCoord& InCoord)
{
	switch (InMapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			switch (InMapConfig.DrawMode)
			{
			case EGridMapDrawMode::BaseOnRowColumn:
				auto RowColumn = StableCoordToRowColumn(InMapConfig, InCoord);
				return RowColumn.X >= -InMapConfig.MapSize.X / 2.f &&
					RowColumn.X <= InMapConfig.MapSize.X / 2.f &&
					RowColumn.Y >= -InMapConfig.MapSize.Y / 2.f &&
					RowColumn.Y <= InMapConfig.MapSize.Y / 2.f;
			default:
				UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.IsCoordInMapArea]暂不支持的坐标转换: %s"), *UEnum::GetValueAsString(InMapConfig.DrawMode));
				break;
			}
		}
		break;
	default:
		UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.IsCoordInMapArea]尚未实现的坐标转换: %s"), *UEnum::GetValueAsString(InMapConfig.MapType));
		break;
	}

	return false;
}

FIntVector2 UGridPathFindingBlueprintFunctionLib::StableCoordToRowColumn(const FGridMapConfig& InMapConfig, const FHCubeCoord& InCoord)
{
	switch (InMapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			if (InMapConfig.DrawMode == EGridMapDrawMode::BaseOnRadius)
			{
				UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.StableCoordToRowColumn]暂不支持的坐标转换: %s"), *UEnum::GetValueAsString(InMapConfig.DrawMode));
			}

			if (InMapConfig.TileOrientation == ETileOrientationFlag::FLAT)
			{
				return FIntVector2{InCoord.QRS.Y + (InCoord.QRS.X - (InCoord.QRS.X & 1)) / 2, InCoord.QRS.X};
			}

			if (InMapConfig.TileOrientation == ETileOrientationFlag::POINTY)
			{
				return FIntVector2{InCoord.QRS.Y, InCoord.QRS.X + (InCoord.QRS.Y - (InCoord.QRS.Y & 1)) / 2};
			}
		}
		break;
	default:
		UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.StableCoordToRowColumn]尚未实现的坐标转换: %s"), *UEnum::GetValueAsString(InMapConfig.MapType));
		break;
	}

	return FIntVector2::ZeroValue;
}

void UGridPathFindingBlueprintFunctionLib::StableForEachMapGrid(const FGridMapConfig& InMapConfig,
                                                                TFunction<void(const FHCubeCoord& Coord, int32 Row, int32 Column)> Func)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UGridPathFindingBlueprintFunctionLib.StableForEachMapGrid]"));

	switch (InMapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			switch (InMapConfig.DrawMode)
			{
			case EGridMapDrawMode::BaseOnRowColumn:
				{
					const auto RowStart = -FMath::FloorToInt(InMapConfig.MapSize.X / 2.f);
					const auto RowEnd = FMath::CeilToInt(InMapConfig.MapSize.X / 2.f);
					const auto ColumnStart = -FMath::FloorToInt(InMapConfig.MapSize.Y / 2.f);
					const auto ColumnEnd = FMath::CeilToInt(InMapConfig.MapSize.Y / 2.f);

					if (InMapConfig.TileOrientation == ETileOrientationFlag::FLAT)
					{
						for (int32 Column{ColumnStart}; Column < ColumnEnd; ++Column)
						{
							for (int32 Row{RowStart}; Row < RowEnd; ++Row)
							{
								auto Q = Column;
								auto R = Row - (Column - (Column & 1)) / 2;
								FHCubeCoord CCoord{FIntVector(Q, R, -Q - R)};

								// 调用传入的函数
								Func(CCoord, Row, Column);
							}
						}
					}
					else if (InMapConfig.TileOrientation == ETileOrientationFlag::POINTY)
					{
						for (int32 Row{RowStart}; Row < RowEnd; ++Row)
						{
							for (int32 Column{ColumnStart}; Column < ColumnEnd; ++Column)
							{
								auto Q = Column - (Row - (Row & 1)) / 2;
								auto R = Row;
								FHCubeCoord CCoord{FIntVector(Q, R, -Q - R)};

								// 调用传入的函数
								Func(CCoord, Row, Column);
							}
						}
					}
				}
				break;
			default:
				UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.StableForEachMapGrid] lack of impl, DrawMode: %d"), InMapConfig.DrawMode);
				break;
			}
		}
		break;
	default:
		UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.StableForEachMapGrid] lack of impl, MapType: %d"), InMapConfig.MapType);
		break;
	}
}

FVector UGridPathFindingBlueprintFunctionLib::StableCoordToWorld(const FGridMapConfig& InMapConfig, const FHTileOrientation& InTileOrientation, const FHCubeCoord& InCoord)
{
	switch (InMapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			// FHTileOrientation TileOrientation = GetTileOrientation(InMapConfig.MapType, InMapConfig.TileOrientation);
			FVector2D GridSize = InMapConfig.GetGridSize();

			float x = ((InTileOrientation.f0 * InCoord.QRS.X) + (InTileOrientation.f1 * InCoord.QRS.Y)) * GridSize.X;
			float y = ((InTileOrientation.f2 * InCoord.QRS.X) + (InTileOrientation.f3 * InCoord.QRS.Y)) * GridSize.Y;

			return FVector(x + InMapConfig.MapCenter.X, y + InMapConfig.MapCenter.Y, InMapConfig.MapCenter.Z);
		}
	default:
		break;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[UGridPathFindingBlueprintFunctionLib.StableCoordToWorld]尚未实现的坐标转换: %d"), InMapConfig.MapType);
	return FVector::ZeroVector;
}