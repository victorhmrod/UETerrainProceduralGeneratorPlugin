// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023


#include "ObjectSpawner.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

// Sets default values
AObjectSpawner::AObjectSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PrimaryActorTick.TickInterval = 50;
}

// Called when the game starts or when spawned
void AObjectSpawner::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AObjectSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateTiles();
}

FVector AObjectSpawner::GetPlayerCell()
{
	FVector Location = FVector::Zero();
	const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (const AActor* Player = PlayerController->GetPawn())
	{
		Location = Player->GetActorLocation();
	}
	
	Location = Location / CellSize;
	Location = FVector(FMath::RoundToInt(Location.X), FMath::RoundToInt(Location.Y), FMath::RoundToInt(Location.Z)) * CellSize;
	
	return Location;
}

void AObjectSpawner::UpdateTiles()
{
	RemoveFarTiles();
	
	FVector Origin = GetPlayerCell();

	for (int Y = CellCount * (-.5f); Y <= CellCount * (0.5f); Y++)
	{
		for (int X = CellCount * (-.5f); X <= CellCount * (0.5f); X++)
		{
			FVector TileCenter = Origin + FVector(X, Y, 0) * CellSize;

			FHitResult Hit;
			FCollisionQueryParams CollisionParams;
			const APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			AActor* Player = PlayerController->GetPawn();
			CollisionParams.AddIgnoredActor(Player);

			if (GetWorld()->LineTraceSingleByChannel(Hit, TileCenter + FVector::UpVector * TraceDistance,
			                                         TileCenter - FVector::UpVector * TraceDistance,
			                                         ECC_Visibility, CollisionParams))
			{
				if (!SpawnedTiles.Contains(FVector2D(TileCenter.X, TileCenter.Y)))
				{
					//DrawDebugBox(GetWorld(), Hit.Location, CellSize * FVector(1, 1, 1) * .5f, FColor::Red, false, 5);
					SpawnedTiles.Add(FVector2D(TileCenter.X, TileCenter.Y));
					UpdateTile(Hit.Location);
				}
			}
		}
	}
}


void AObjectSpawner::UpdateTile(const FVector TileCenter)
{
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.bReturnPhysicalMaterial = true;
	bool bHit;
		
	for (int Y = CellSize * (-.5); Y <= CellSize * (.5f); Y+= SubCellSize)
	{
		for (int X = CellSize * (-.5f); X < CellSize * (.5f); X+= SubCellSize)
		{
			FVector SubCellLocation = TileCenter + FVector(
				X + FMath::RandRange(-SubCellRandomOffset, SubCellRandomOffset),
				Y + FMath::RandRange(-SubCellRandomOffset, SubCellRandomOffset), 0);
			bHit = GetWorld()->LineTraceSingleByChannel(
				Hit,
				SubCellLocation + FVector::UpVector * TraceDistance,
				SubCellLocation - FVector::UpVector * TraceDistance,
				ECC_Visibility,
				CollisionParams)
			;
			if (bHit)
			{
				// DrawDebugLine(GetWorld(), Hit.Location + FVector::UpVector * 100, Hit.Location, FColor::Green, false, 10);
				SpawnObject(Hit, TileCenter);
			}
		}
	}
}

void AObjectSpawner::SpawnObject(const FHitResult Hit, const FVector ParentTileCenter)
{
}


void AObjectSpawner::RemoveFarTiles()
{
	FVector PlayerCell = GetPlayerCell();
	TArray<FVector2d> SpawnedTilesCopy;
	SpawnedTilesCopy.Append(SpawnedTiles);

	for (int TileIndex = 0; TileIndex < SpawnedTilesCopy.Num(); ++TileIndex)
	{
		FVector2d RelativeTileLocation = SpawnedTilesCopy[TileIndex] - FVector2d(PlayerCell);
		if (FMath::Abs(RelativeTileLocation.X) > CellCount + 0.5f * CellSize ||
			FMath::Abs(RelativeTileLocation.Y) > CellCount + 0.5f * CellSize)
		{
			FHitResult Hit;
			FVector TileCenter = FVector(SpawnedTilesCopy[TileIndex], PlayerCell.Z);
			FCollisionQueryParams CollisionParams;
			APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
			AActor* Player = PlayerController->GetPawn();
			CollisionParams.AddIgnoredActor(Player);

			bool bHit = GetWorld()->LineTraceSingleByChannel(Hit,
				TileCenter + FVector::UpVector * TraceDistance,
				TileCenter - FVector::UpVector * TraceDistance,
				ECC_Visibility, CollisionParams);
			if (bHit)
			{
				//DrawDebugBox(GetWorld(), Hit.Location, FVector(1,1,1) * CellSize * 0.5f, FColor::Blue, false, 5);
				RemoveTile(Hit.Location);
				SpawnedTiles.Remove(SpawnedTilesCopy[TileIndex]);
			}
		}
	}
}

void AObjectSpawner::RemoveTile(const FVector TileCenter)
{
	
}
