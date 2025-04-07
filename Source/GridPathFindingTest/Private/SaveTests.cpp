
#include "GridPathFindingBlueprintFunctionLib.h"
#include "Misc/AutomationTest.h"
#include "Types/GridMapSave.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 5
	// UE5 specific flags
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
FSaveMapTest,
"GridPathFinding.SaveMap",  // 测试路径，方便分类和过滤
EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter
);
#else
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
FSaveMapTest,
"GridPathFinding.SaveMap",  // 测试路径，方便分类和过滤
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
);
#endif


bool FSaveMapTest::RunTest(const FString& Parameters)
{
	// 序列化测试
	FGridMapTilesSave TilesSave;
	for (int32 i = 0; i < 10; ++i)
	{
		FSerializableTile Tile;
		Tile.Coord = FHCubeCoord(i, i, -2 * i);
		Tile.Height = 1.f;
		Tile.EnvironmentType = FName(*FString::Printf(TEXT("Env_%d"), i));
		FTileEnvData TileEnvData;
		TileEnvData.TextureIndex = i;
		Tile.TileEnvData = TileEnvData;
		TilesSave.GridTiles.Add(Tile);
	}

	auto s = UGridPathFindingBlueprintFunctionLib::SerializeGridMapTiles(TilesSave);

	auto deco = UGridPathFindingBlueprintFunctionLib::DeserializeGridMapTiles(s);

	for (int32 i = 0; i < deco.GridTiles.Num(); ++i)
	{
		const auto& Tile = deco.GridTiles[i];
		const auto& ToCompare = TilesSave.GridTiles[i];

		if (Tile.Coord != ToCompare.Coord)
		{
			UE_LOG(LogTemp, Error, TEXT("Coord不匹配"));
			return false;
		}

		if (Tile.Height != ToCompare.Height)
		{
			UE_LOG(LogTemp, Error, TEXT("Height不匹配"));
			return false;
		}

		if (Tile.EnvironmentType != ToCompare.EnvironmentType)
		{
			UE_LOG(LogTemp, Error, TEXT("EnvironmentType不匹配"));
			return false;
		}
		if (Tile.TileEnvData.TextureIndex != ToCompare.TileEnvData.TextureIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("TextureIndex不匹配"));
			return false;
		}
		if (Tile.CustomGameplayData != ToCompare.CustomGameplayData)
		{
			UE_LOG(LogTemp, Error, TEXT("CustomGameplayData不匹配"));
			return false;
		}
		UE_LOG(LogTemp, Log,
		       TEXT("%d{Coord: %s, Height: %f, EnvironmentType: %s, TextureIndex: %d, CustomGameplayData: %s}"
		       ),
		       i,
		       *Tile.Coord.ToString(), Tile.Height, *Tile.EnvironmentType.ToString(), Tile.TileEnvData.TextureIndex,
		       *Tile.CustomGameplayData);
	}
	
	return true;
}