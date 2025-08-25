// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#include "WEAnimalSpawner.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/World.h"

void AWEAnimalSpawner::SpawnObject(const FHitResult Hit, const FVector ParentTileCenter)
{
	Super::SpawnObject(Hit, ParentTileCenter);

	if (Hit.Location.Z < 250)
	{
		return;
	}
	
	if (Hit.PhysMaterial != nullptr)
	{
		if (Hit.PhysMaterial->SurfaceType != SupportedSurfaceType)
		{
			return;
		}
	}

	const FVector SpawnLocation = Hit.Location + FVector::UpVector * 200;
	const FRotator SpawnRotation(0, FMath::RandRange(0.f, 360.f), 0.f);

	for (int i = 0; i < AnimalTypes.Num(); ++i)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.bNoFail = true;

		AActor* Animal = GetAnimalFromPool(AnimalTypes[i].AnimalClass);

		if (!Animal)
		{
			// Spawn the actor
			Animal = GetWorld()->SpawnActor<AActor>(AnimalTypes[i].AnimalClass, SpawnLocation, SpawnRotation, SpawnParameters);
			//DrawDebugLine(GetWorld(), SpawnLocation, SpawnLocation + FVector::UpVector * 10000, FColor::Red, false, 20);
		}
		else
		{
			Animal->SetActorLocation(SpawnLocation);
			//DrawDebugLine(GetWorld(), SpawnLocation, SpawnLocation + FVector::UpVector * 10000, FColor::Blue, false, 20);
		}

		if (!Animal)
		{
			continue;
		}
		
		FTileAnimalData* AnimalData = AnimalsOfTile.Find(ParentTileCenter);

		if (AnimalData)
		{
			AnimalData->SpawnedAnimals.Add(Animal);
		}
		else
		{
			AnimalsOfTile.Add(ParentTileCenter, FTileAnimalData({Animal}));
		}
	}
}

void AWEAnimalSpawner::RemoveTile(const FVector TileCenter)
{
	Super::RemoveTile(TileCenter);
	
	FTileAnimalData* AnimalData = AnimalsOfTile.Find(TileCenter);
	if (!AnimalData)
	{
		return;
	}
	
	for (int i = 0; i < AnimalData->SpawnedAnimals.Num(); ++i)
	{
		FTileAnimalData* PooledAnimalData = PooledAnimals.Find(AnimalData->SpawnedAnimals[i]->GetClass());
		if (PooledAnimalData)
		{
			PooledAnimalData->SpawnedAnimals.Add(AnimalData->SpawnedAnimals[i]);
		}
		else
		{
			PooledAnimals.Add(AnimalData->SpawnedAnimals[i]->GetClass(), FTileAnimalData({AnimalData->SpawnedAnimals[i]}));
		}
	}
	AnimalData->SpawnedAnimals.Empty();
}

AActor* AWEAnimalSpawner::GetAnimalFromPool(const TSubclassOf<AActor>& AnimalClass)
{
	AActor* Animal = nullptr;

	FTileAnimalData* AnimalData = PooledAnimals.Find(AnimalClass);
	if (AnimalData)
	{
		if (AnimalData->SpawnedAnimals.Num() > 0)
		{
			Animal = AnimalData->SpawnedAnimals.Pop();	
		}
	}
	return Animal;
}
