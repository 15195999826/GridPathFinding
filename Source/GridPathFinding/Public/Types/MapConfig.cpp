#include "MapConfig.h"

#include "GridPathFinding.h"

FVector2D FGridMapConfig::GetGridSize() const
{
	switch (MapType)
	{
		case EGridMapType::HEX_STANDARD:
			return FVector2D(HexGridRadius, HexGridRadius);
		case EGridMapType::SQUARE_STANDARD:
			return FVector2D(SquareSize, SquareSize);
		case EGridMapType::RECTANGLE_STANDARD:
		case EGridMapType::RECTANGLE_SIX_DIRECTION:
			return RectSize;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("未知的地图类型: %d"), (int)MapType);
	return FVector2D::ZeroVector;
}
