#include "BP_ObjectGrab.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Bunny_Third_PersonCharacter.h"

// Sets default values
ABP_ObjectGrab::ABP_ObjectGrab()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Creating the Test Object
	MyObjectBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InterObject"));
	RootComponent = MyObjectBox;
	
	//Gravety settings
	MyObjectBox->SetSimulatePhysics(true);
	MyObjectBox->SetEnableGravity(true);
	MyObjectBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MyObjectBox->SetCollisionObjectType(ECC_WorldDynamic);

	// Collostion
	CollisonSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisonSphere"));
	CollisonSphere->SetupAttachment(MyObjectBox);
	CollisonSphere->InitSphereRadius(150.0f);
	
	bIsHeld = false;
}

// Called when the game starts or when spawned
void ABP_ObjectGrab::BeginPlay()
{
	Super::BeginPlay();
	
	if (CollisonSphere) // add in the overlap 
	{
		CollisonSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisonSphere->SetCollisionObjectType(ECC_WorldDynamic);
		CollisonSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		CollisonSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		CollisonSphere->SetGenerateOverlapEvents(true);

		CollisonSphere->OnComponentBeginOverlap.AddDynamic(this, &ABP_ObjectGrab::OnBeginOverlap);
		CollisonSphere->OnComponentEndOverlap.AddDynamic(this, &ABP_ObjectGrab::OnEndOverlap);
	}
}

// Called every frame
void ABP_ObjectGrab::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

//Grab and endGrab
void ABP_ObjectGrab::OnBeginOverlap(UPrimitiveComponent* OverlappedCompnent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABunny_Third_PersonCharacter* Character = Cast<ABunny_Third_PersonCharacter>(OtherActor)) 
	{
		Character->SetNearbyGrabObject(this);
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("In box"));
	}
}

void ABP_ObjectGrab::OnEndOverlap(UPrimitiveComponent* OverlappedCompnent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABunny_Third_PersonCharacter * Character = Cast<ABunny_Third_PersonCharacter>(OtherActor))
	{
		if (!OverlappedCompnent->IsOverlappingActor(OtherActor))
		{
			Character->ClearNearbyGrabObject(this);
		}
	}
}

void ABP_ObjectGrab::GrabObject(USceneComponent* AttachToComponent, FName SocketName)
{
	if (!AttachToComponent) return;

	bIsHeld = true;

	MyObjectBox->SetSimulatePhysics(false);
	MyObjectBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Attach Object to socket 
	FAttachmentTransformRules AttachmentRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	MyObjectBox->AttachToComponent(AttachToComponent, AttachmentRule, SocketName);
}

void ABP_ObjectGrab::DropObject()
{
	bIsHeld = false;

	MyObjectBox->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	MyObjectBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	MyObjectBox->SetSimulatePhysics(true);
}

