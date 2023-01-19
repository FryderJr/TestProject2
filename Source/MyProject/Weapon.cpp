// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::White,text)


// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(FName("MeshComponent"));
	RootComponent = MeshComponent;
	MuzzleSocketName = FName("Muzzle");
	TrailTargetName = FName("BeamEnd");

	BaseDamage = 20.0f;
	FireRate = 600;

	InitialBulletSpread = 0.05f;
	BulletSpreadIncreaseRate = 1.5f;
	BulletSpreadDecreaseRate = 3.0f;
	BulletSpreadCoolDownTime = 0.2f;

	//Setup multiplayer
	SetReplicates(true);
	SetReplicateMovement(true);
	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60.0f / FireRate;

	CurrentBulletSpread = InitialBulletSpread;

	LastTimeFired = GetWorld()->GetTimeSeconds();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, HitScanTrace);
}

void AWeapon::OnRep_HitScanTrace()
{
	PlayEffect(HitScanTrace.TraceEnd);
}

void AWeapon::Fire()
{
	print(FString::Printf(TEXT("Fire begin")));

	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}

	AActor* MyOwner = GetOwner();
	if (!MyOwner)
	{
		return;
	}

	

	FVector ShootDirection = MyOwner->GetActorForwardVector();

	//Calculate bullet spread
	float SecondsPassedFromLastShot = GetWorld()->GetTimeSeconds() - LastTimeFired;
	if (SecondsPassedFromLastShot > BulletSpreadCoolDownTime)
	{
		CurrentBulletSpread += (BulletSpreadCoolDownTime - SecondsPassedFromLastShot) * BulletSpreadDecreaseRate * InitialBulletSpread;
	}
	else
	{
		CurrentBulletSpread += (BulletSpreadCoolDownTime - SecondsPassedFromLastShot) * BulletSpreadIncreaseRate * InitialBulletSpread;
	}

	CurrentBulletSpread = FMath::Clamp(CurrentBulletSpread, InitialBulletSpread, 0.1f);

	//Add bullet spread
	ShootDirection.Z += FMath::RandRange(-CurrentBulletSpread, CurrentBulletSpread);
	ShootDirection.Y += FMath::RandRange(-CurrentBulletSpread, CurrentBulletSpread);

	FVector TraceEnd = MyOwner->GetActorLocation() + (ShootDirection * 10000);

	//End of trail particle system component
	FVector TrailEnd = TraceEnd;

	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(MyOwner); //Igrone collision with owner
	CollisionQueryParams.AddIgnoredActor(this);	   //Ignore collision with gun itself
	CollisionQueryParams.bTraceComplex = true;	   //Trace with each individual triangle of complex mesh
	CollisionQueryParams.bReturnPhysicalMaterial = true;   //Return physical material

	EPhysicalSurface SurfaceType = SurfaceType_Default;

	if (GetWorld()->LineTraceSingleByChannel(HitResult, MyOwner->GetActorLocation(), TraceEnd, ECC_GameTraceChannel11, CollisionQueryParams))
	{
		AActor* HitActor = HitResult.GetActor();

		TrailEnd = HitResult.ImpactPoint;

		float ActualDamage = BaseDamage;

		
		UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShootDirection, HitResult, MyOwner->GetInstigatorController(), MyOwner, nullptr);

	}


	if (GetLocalRole() == ROLE_Authority)
	{
		HitScanTrace.TraceEnd = TrailEnd;
	}

	LastTimeFired = GetWorld()->GetTimeSeconds();

	print(FString::Printf(TEXT("Fire end")));
}

void AWeapon::ServerFire_Implementation()
{
	Fire();
}

// Called to play effects on client
void AWeapon::PlayEffect(FVector TrailEnd)
{
	print(FString::Printf(TEXT("Fire effect")));
	FVector SocketLocation = MeshComponent->GetSocketLocation(MuzzleSocketName);
	UParticleSystemComponent* Trail = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TrailEffect, SocketLocation);
	if (Trail)
	{
		Trail->SetVectorParameter(TrailTargetName, TrailEnd);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::StartFire()
{
	float FireDelay = FMath::Max(LastTimeFired + TimeBetweenShots - GetWorld()->GetTimeSeconds(), 0.0f);
	print(FString::Printf(TEXT("Fire gun %f, %f"), FireDelay, TimeBetweenShots));
	Fire();
	GetWorldTimerManager().SetTimer(FTimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true, FireDelay);
	//GetWorldTimerManager().SetTimer(FTimeBetweenShots, this, &AWeapon::Fire, TimeBetweenShots, true);
}

void AWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(FTimeBetweenShots);
}

