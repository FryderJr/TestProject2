// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

USTRUCT()
//Contains information of a single linetrace
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FVector_NetQuantize TraceEnd;
};


UCLASS()
class MYPROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray < FLifetimeProperty >& OutLifetimeProps) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	USkeletalMeshComponent* MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName MuzzleSocketName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	FName TrailTargetName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	UParticleSystem* TrailEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
	float BaseDamage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (ClampMin = 0.0f, ClampMax = 0.1f))
	float InitialBulletSpread;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (ClampMin = 1.0f, ClampMax = 1.5f))
	float BulletSpreadIncreaseRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (ClampMin = 1.0f, ClampMax = 4.5f))
	float BulletSpreadDecreaseRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon, meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float BulletSpreadCoolDownTime;

	float CurrentBulletSpread;

	FTimerHandle FTimeBetweenShots;

	float LastTimeFired;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	float FireRate;

	float TimeBetweenShots;

	UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire();

	void PlayEffect(FVector TrailEnd);

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void StartFire();

	void StopFire();
};
