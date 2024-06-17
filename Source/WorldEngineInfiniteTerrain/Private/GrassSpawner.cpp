// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#include "GrassSpawner.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

void AGrassSpawner::SpawnObject(const FHitResult Hit, const FVector ParentTileCenter)
{
	if (Hit.Location.Z<250)
	{
		return;
	}
	if (Hit.PhysMaterial != nullptr)
	{
		if (Hit.PhysMaterial->SurfaceType != SupportedSurfaceType)
		{
			return;
		}
		for (int FoliageTypeIndex = 0; FoliageTypeIndex < GrassComponents.Num(); FoliageTypeIndex++)
		{
			const UFoliageType_InstancedStaticMesh* FoliageType = GrassTypes[FoliageTypeIndex];
			
			// Check foliage growing altitude
			if (Hit.Location.Z < FoliageType->Height.Min || Hit.Location.Z > FoliageType->Height.Max)
				continue;

			// Growth density check
			if (FoliageType->InitialSeedDensity < RandomStream.FRandRange(0.f, 10.f))
				continue;
			
			// Check ground slope
			const float DotProduct = FVector::DotProduct(Hit.ImpactNormal, FVector::UpVector);

			if (const float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotProduct)); SlopeAngle < FoliageType->GroundSlopeAngle.Min || SlopeAngle > FoliageType->GroundSlopeAngle.Max)
				continue;
			
			
			FTransform InstanceTransform = FTransform();
			InstanceTransform.SetLocation(Hit.Location);

			// Align to normal
			if (FoliageType->AlignToNormal)
			{
				InstanceTransform.SetRotation(FRotationMatrix::MakeFromZ(Hit.ImpactNormal).Rotator().Quaternion());
			}
			else
			{
				InstanceTransform.SetRotation(FRotator(0, FMath::RandRange(0, 360), 0).Quaternion());
			}
						
			InstanceTransform.SetScale3D(FVector(1,1,1) * RandomStream.FRandRange(FoliageType->ProceduralScale.Min, FoliageType->ProceduralScale.Max));
			GrassComponents[FoliageTypeIndex]->AddInstance(InstanceTransform,true);
		}
	}
}

void AGrassSpawner::RemoveTile(const FVector TileCenter)
{
	Super::RemoveTile(TileCenter);
	FVector Min = FVector(TileCenter + FVector::One() * CellSize * (-0.5f));
	FVector Max = FVector(TileCenter + FVector::One() * CellSize * (0.5f));
	Min.Z = TileCenter.Z - TraceDistance;
	Max.Z = TileCenter.Z + TraceDistance;
	
	FBox Box = FBox(Min, Max);
	
	for (int FoliageComponentIndex = 0; FoliageComponentIndex < GrassComponents.Num(); ++FoliageComponentIndex)
	{
		UInstancedStaticMeshComponent* FoliageComponent = GrassComponents[FoliageComponentIndex];
		TArray<int> Instances = FoliageComponent->GetInstancesOverlappingBox(Box, true);

		if (Instances.Num() > 0)
		{
			FoliageComponent->RemoveInstances(Instances);
		}
	}
}
