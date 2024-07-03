// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#include "WETerrain.h"
#include "KismetProceduralMeshLibrary.h"
#include "WEUtilities.h"
#include "Async/Async.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AWETerrain::AWETerrain()
{
	PrimaryActorTick.bCanEverTick = true;
	
	PROC_Terrain = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("PROC_Terrain"));
	PROC_Terrain->bUseAsyncCooking = true;
	PROC_Terrain->SetupAttachment(RootComponent);

	TileReplaceableDistance = CellSize * (NumOfSectionX + NumOfSectionY) / 2 * (XVertexCount + YVertexCount);
}

// Called when the game starts or when spawned
void AWETerrain::BeginPlay()
{
	Super::BeginPlay();
	
	RandomizeTerrain();
}

// Called every frame
void AWETerrain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWETerrain::GenerateTerrain(const int InSectionIndexX, const int InSectionIndexY, const int LODFactor)
{
	int LODXVertexCount = XVertexCount / LODFactor;
	int LODYVertexCount = YVertexCount / LODFactor;
	float LODCellSize = CellSize * LODFactor;

	float XGap = ((XVertexCount-1) * CellSize - (LODXVertexCount-1) * LODCellSize) / LODCellSize;
	LODXVertexCount += FMath::CeilToInt(XGap);

	float YGap = ((YVertexCount-1) * CellSize - (LODYVertexCount-1) * LODCellSize) / LODCellSize;
	LODYVertexCount += FMath::CeilToInt(YGap);

	FVector Offset = FVector(InSectionIndexX * (XVertexCount - 1), InSectionIndexY * (YVertexCount - 1), 0.f) * CellSize;

	TArray<FVector> Vertices;
	// ReSharper disable once CppTooWideScope
	FVector Vertex;

	TArray<FVector2D> UVs;
	// ReSharper disable once CppTooWideScope
	FVector2D UV;

	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
		
	// Vertices and UVs
	for (int32 iVY = -1; iVY <= LODYVertexCount; iVY++)
	{
		for (int32 iVX = -1; iVX <= LODXVertexCount; iVX++)
		{
			// Vertex calculation
			Vertex.X = iVX * LODCellSize + Offset.X;
			Vertex.Y = iVY * LODCellSize + Offset.Y;
			Vertex.Z = GetHeight(FVector2D(Vertex.X, Vertex.Y));
			Vertices.Add(Vertex);

			// Set UVs
			UV.X = (iVX + (InSectionIndexX * (LODXVertexCount - 1))) * LODCellSize / 100;
			UV.Y = (iVY + (InSectionIndexY * (LODYVertexCount - 1))) * LODCellSize / 100;
			UVs.Add(UV);
		}
	}

	// Triangles
	Triangles.Empty();
	for (int32 iTY = 0; iTY <= LODYVertexCount; iTY++)
	{
		for (int32 iTX = 0; iTX <= LODXVertexCount; iTX++)
		{
			Triangles.Add(iTX + iTY * (LODXVertexCount + 2));
			Triangles.Add(iTX + (iTY + 1) * (LODXVertexCount + 2));
			Triangles.Add(iTX + iTY * (LODXVertexCount + 2) + 1);

			Triangles.Add(iTX + (iTY + 1) * (LODXVertexCount + 2));
			Triangles.Add(iTX + (iTY + 1) * (LODXVertexCount + 2) + 1);
			Triangles.Add(iTX + iTY * (LODXVertexCount + 2) + 1);
		}
	}
	
	int VertexIndex = 0;
	
	// Calculate tangents to procedural mesh
 	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(Vertices, Triangles, UVs, Normals, Tangents);

	// Subset vertices and subset UVs
	for (int32 iVY = -1; iVY <= LODYVertexCount; iVY++)
	{
		for (int32 iVX = -1; iVX <= LODXVertexCount; iVX++)
		{
			if (-1 < iVY && iVY < LODYVertexCount && -1 < iVX && iVX < LODXVertexCount)
			{
				SubVertices.Add(Vertices[VertexIndex]);
				SubUVs.Add(UVs[VertexIndex]);
				SubNormals.Add(Normals[VertexIndex]);
				SubTangents.Add(Tangents[VertexIndex]);
			}
			VertexIndex++;
		}
	}
	
	SubTriangles.Empty();
	for (int32 iTY = 0; iTY <= LODYVertexCount - 2; iTY++)
	{
		for (int32 iTX = 0; iTX <= LODXVertexCount - 2; iTX++)
		{
			SubTriangles.Add(iTX + iTY * LODXVertexCount);
			SubTriangles.Add(iTX + iTY * LODXVertexCount + LODXVertexCount);
			SubTriangles.Add(iTX + iTY * LODXVertexCount + 1);

			SubTriangles.Add(iTX + iTY * LODXVertexCount + LODXVertexCount);
			SubTriangles.Add(iTX + iTY * LODXVertexCount + LODXVertexCount + 1);
			SubTriangles.Add(iTX + iTY * LODXVertexCount + 1);
		}
	}

	UWEUtilities::NavUpdate(PROC_Terrain);
	
	TileDataReady = true;
}

// Transforms infinite generation to the Async format, to make the system more optimized
void AWETerrain::GenerateTerrainAsync(const int InSectionIndexX, const int InSectionIndexY, const int LODLevel)
{
	GeneratorBusy = true;
	SectionIndexX = InSectionIndexX;
	SectionIndexY = InSectionIndexY;
	CellLODLevel = FMath::Max(1, LODLevel);
	
	QueuedTiles.Add(FIntPoint(InSectionIndexX, InSectionIndexY), FIntPoint(MeshSectionIndex, CellLODLevel));

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&]()
		{
			auto WorldGenTask = new FAsyncTask<FAsyncWorldGeneration>(this);
			WorldGenTask->StartBackgroundTask();
			WorldGenTask->EnsureCompletion();
			delete WorldGenTask;
		}
	);
}

// Draw a simples section tile
int AWETerrain::DrawTile()
{
	int DrawnMeshSection;
	TileDataReady = false;
	TArray<FIntPoint> ValueArray;
	TArray<FIntPoint> KeyArray;
	int FurthestTileIndex = GetFurthestUpdatableTile();
	if (FurthestTileIndex > -1)
	{
		QueuedTiles.GenerateKeyArray(KeyArray);
		QueuedTiles.GenerateValueArray(ValueArray);
		int ReplaceableMeshSection = ValueArray[FurthestTileIndex].X;
		FIntPoint ReplaceableTile = KeyArray[FurthestTileIndex];
		DrawnMeshSection = ReplaceableMeshSection;

		// Remove foliage
		if (FurthestTileIndex)
		{
			RemoveFoliageTile(ReplaceableMeshSection);
		}

		PROC_Terrain->ClearMeshSection(ReplaceableMeshSection);
		UWEUtilities::NavUpdate(PROC_Terrain);
		PROC_Terrain->CreateMeshSection(ReplaceableMeshSection, SubVertices, SubTriangles, SubNormals, SubUVs,
		                                TArray<FColor>(), SubTangents, true);
		UWEUtilities::NavUpdate(PROC_Terrain);
		
		QueuedTiles.Add(FIntPoint(SectionIndexX, SectionIndexY), FIntPoint(ReplaceableMeshSection, CellLODLevel));
		QueuedTiles.Remove(ReplaceableTile);
	}
	else
	{
		// Create mesh section
		PROC_Terrain->CreateMeshSection(MeshSectionIndex, SubVertices, SubTriangles, SubNormals, SubUVs,
		                                TArray<FColor>(), SubTangents, true);
		UWEUtilities::NavUpdate(PROC_Terrain);
		if (TerrainMaterial)
		{
			PROC_Terrain->SetMaterial(MeshSectionIndex, TerrainMaterial);
		}
		DrawnMeshSection = MeshSectionIndex;
		
		MeshSectionIndex++;
	}
	
	SubVertices.Empty();
	SubNormals.Empty();
	SubNormals.Empty();
	SubUVs.Empty();
	SubTangents.Empty();

	KeyArray.Empty();
	ValueArray.Empty();
	RemoveLODQueue.GenerateKeyArray(KeyArray);
	RemoveLODQueue.GenerateValueArray(ValueArray);
	
	if (RemoveLODQueue.Contains(FIntPoint(SectionIndexX, SectionIndexY)))
	{
		FIntPoint* Val = RemoveLODQueue.Find(FIntPoint(SectionIndexX, SectionIndexY));
		RemoveFoliageTile(Val->X);
		PROC_Terrain->ClearMeshSection(Val->X);
		UWEUtilities::NavUpdate(PROC_Terrain);
		RemoveLODQueue.Remove(FIntPoint(SectionIndexX, SectionIndexY));
	}
		
	return DrawnMeshSection;
}

// Get exactly player location, if player is not spawned returns 0,0,0 to vector
FVector AWETerrain::GetPlayerLocation()
{
	// ReSharper disable once CppTooWideScope
	TObjectPtr<APawn> PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (PlayerPawn)
	{
		return PlayerPawn->GetActorLocation();
	}
	return FVector(0, 0, 0);
}

// Get the location of the simple tile
FVector2D AWETerrain::GetTileLocation(FIntPoint TileCoordinate)
{
	return FVector2D(TileCoordinate * FIntPoint(XVertexCount - 1, YVertexCount - 1) * CellSize) + FVector2D(XVertexCount - 1, YVertexCount - 1) * CellSize / 2;
}

// Get the closest tile generated
FIntPoint AWETerrain::GetClosestQueuedTile()
{
	float ClosestDistance = TNumericLimits<float>::Max();
	FIntPoint ClosestTile;
	for(auto& Entry: QueuedTiles)
	{
		FIntPoint& Key = Entry.Key;
		int Value = Entry.Value.X;
		if (Value == -1)
		{
			FVector2D TileLocation = GetTileLocation(Key);
			FVector PlayerLocation = GetPlayerLocation();
			float Distance = FVector2D::Distance(TileLocation, FVector2D(PlayerLocation));
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestTile = Key;
			}
		}
	}
	return ClosestTile;
}

// Get the furthest tile updatable simple tile
int AWETerrain::GetFurthestUpdatableTile()
{
	float FurthestDistance = -1;
	int FurthestTileIndex = -1;
	int CurrentIndex = 0;
	for(auto& Entry: QueuedTiles)
	{
		FIntPoint& Key = Entry.Key;
		int Value = Entry.Value.X;
		if (Value != -1)
		{
			FVector2D TileLocation = GetTileLocation(Key);
			FVector PlayerLocation = GetPlayerLocation();
			float Distance = FVector2D::Distance(TileLocation, FVector2D(PlayerLocation));
			if (Distance > FurthestDistance && Distance > TileReplaceableDistance)
			{
				FurthestDistance = Distance;
				FurthestTileIndex = CurrentIndex;
			}
		}
		CurrentIndex++;
	}
	return FurthestTileIndex;
}

void AWETerrain::RemoveFoliageTile(const int TileIndex)
{
	if (TileIndex > 0) return;
	TArray<FProcMeshVertex> Vertices = PROC_Terrain->GetProcMeshSection(TileIndex)->ProcVertexBuffer;
	FVector FirstVertex = Vertices[0].Position;
	FVector LastVertex = Vertices[Vertices.Num()-1].Position;
	FBox Box = FBox(FVector(FirstVertex.X, FirstVertex.Y, (MountainHeight + BoulderHeight) * (-1)), FVector(LastVertex.X, LastVertex.Y, (MountainHeight + BoulderHeight)));

	for (int FoliageComponentIndex = 0; FoliageComponentIndex < FoliageComponents.Num(); FoliageComponentIndex++)
	{
		UInstancedStaticMeshComponent* FoliageComponent = FoliageComponents[FoliageComponentIndex];
		TArray<int> Instances = FoliageComponent->GetInstancesOverlappingBox(Box);

		if (FFoliageInstanceData* FolData = ReplaceableFoliagePool.Find(FoliageComponent))
		{
			FolData->Instances.Append(Instances);
		}
		else
		{
			ReplaceableFoliagePool.Add(FoliageComponent, FFoliageInstanceData(Instances));
		}
	}
}

// Spawn foliage instances based on foliage clustered function
void AWETerrain::AddFoliageInstances(FVector InLocation)
{
	for (int FoliageTypeIndex = 0; FoliageTypeIndex < FoliageTypes.Num(); FoliageTypeIndex++)
	{
		UFoliageType_InstancedStaticMesh* FoliageType = FoliageTypes[FoliageTypeIndex];

		// Skip invalid foliage types
		if (!FoliageType)
			continue;

		// Check foliage growing altitude
		if (InLocation.Z < FoliageType->Height.Min || InLocation.Z > FoliageType->Height.Max)
			continue;

		// Growth density check
		if (FoliageType->InitialSeedDensity < RandomStream.FRandRange(0.f, 10.f))
			continue;

		UInstancedStaticMeshComponent* FoliageIsmComponent = FoliageComponents[FoliageTypeIndex];

		SpawnFoliageCluster(FoliageType, FoliageIsmComponent, InLocation);
	}
}

// Spawn foliage in clustering
void AWETerrain::SpawnFoliageCluster(UFoliageType_InstancedStaticMesh* FoliageType,
                                           UInstancedStaticMeshComponent* FoliageIsmComponent,
                                           const FVector ClusterLocation)
{
	int MaxSteps = RandomStream.RandRange(0, FoliageType->NumSteps);
	int MaxSeeds = RandomStream.RandRange(0, FoliageType->SeedsPerStep);
	FVector ClusterBase = ClusterLocation;
	for (int Step = 0; Step < MaxSteps; Step++)
	{
		ClusterBase += RandomStream.GetUnitVector() * FoliageType->AverageSpreadDistance;

		int InstancesPerStep = 0;
		for (int SeedIndex = 0; SeedIndex < MaxSeeds; SeedIndex++)
		{
			FVector InstanceLocation = ClusterBase + RandomStream.GetUnitVector() * FoliageType->SpreadVariance;
			
			FHitResult HitResults;
			FCollisionQueryParams CollisionParams;

			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResults, InstanceLocation + FVector(0, 0, 2000),
			                                                 InstanceLocation + FVector(0, 0, -2000), ECC_Visibility,
			                                                 CollisionParams);
			
			if (!bHit)
				continue;

			// Check if the terrain is generated
			if (HitResults.Component != PROC_Terrain)
				continue;

			// Check ground slope
			float DotProduct = FVector::DotProduct(HitResults.ImpactNormal, FVector::UpVector);
			float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

			if (SlopeAngle < FoliageType->GroundSlopeAngle.Min || SlopeAngle > FoliageType->GroundSlopeAngle.Max)
				continue;

			FTransform InstanceTransform = FTransform();
			InstanceTransform.SetLocation(
				HitResults.Location + FVector(
					0, 0, RandomStream.FRandRange(FoliageType->ZOffset.Min, FoliageType->ZOffset.Max)));
			InstanceTransform.SetScale3D(
				FVector::One() * RandomStream.FRandRange(FoliageType->ProceduralScale.Min,
				                                         FoliageType->ProceduralScale.Max));
			
			if (FoliageType->AlignToNormal)
			{
				InstanceTransform.SetRotation(FRotationMatrix::MakeFromZ(HitResults.ImpactNormal).Rotator().Quaternion());
			}
			else
			{
				if (FoliageType->RandomYaw)
				{
					InstanceTransform.SetRotation(FRotator(0, RandomStream.FRandRange(0, 360), 0).Quaternion());
				}
			}
						
			// Relocate foliage instances from pool
			if (FFoliageInstanceData* FolData = ReplaceableFoliagePool.Find(FoliageIsmComponent))
			{
				if (FolData->Instances.Num()>0)
				{
					FoliageIsmComponent->UpdateInstanceTransform(FolData->Instances[FolData->Instances.Num()-1], InstanceTransform);
					FolData->Instances.RemoveAt(FolData->Instances.Num() - 1);
					InstancesPerStep++;
					continue;
				}
			}
			
			// Add new foliage instance
			FoliageIsmComponent->AddInstance(InstanceTransform, true);
			InstancesPerStep++;
		}
		
		if (InstancesPerStep == 0)
			return;
	}
}

void AWETerrain::RandomizeTerrain_Implementation()
{
	if (RandomizeTerrainLayout)
	{
		PerlinOffset = FVector2D(FMath::FRandRange(0.f, 1000000.f), FMath::FRandRange(0.f, 1000000.f));
		MountainHeight = MountainHeight * FMath::FRandRange(.5f, 3.f);
		BoulderHeight = BoulderHeight *  FMath::FRandRange(.5f, 2.f);
		MountainScale = MountainScale * FMath::FRandRange(.3f, 3.f);
		BoulderScale = BoulderScale * FMath::FRandRange(.6f, 3.f);
	}
}

// Get height to deform procedural mesh terrain
float AWETerrain::GetHeight(FVector2D Location) const
{
	return
	{
		PerlinNoiseExtended(Location, 1/MountainScale, MountainHeight, FVector2D(.1f)) +
		PerlinNoiseExtended(Location, 1/BoulderScale, BoulderHeight, FVector2D(.2f)) +
		PerlinNoiseExtended(Location, 1/PlainsScale, PlainsHeight, FVector2D(.3f)) +
		PerlinNoiseExtended(Location, 1/LakeScale, LakeHeight, FVector2D(.4f))
	};
}

// Set perlin noise
float AWETerrain::PerlinNoiseExtended(const FVector2D Location, const float Scale, const float Amplitude,
	const FVector2D Offset) const
{
	return FMath::PerlinNoise2D(Location * Scale + FVector2D(.1f, .1f) + Offset + PerlinOffset) * Amplitude;
}

void AWETerrain::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, RandomizeTerrainLayout);
}

// Async work system
// ReSharper disable once CppMemberFunctionMayBeConst
void FAsyncWorldGeneration::DoWork()
{
	WorldGeneration->GenerateTerrain(WorldGeneration->SectionIndexX, WorldGeneration->SectionIndexY, WorldGeneration->CellLODLevel);
}