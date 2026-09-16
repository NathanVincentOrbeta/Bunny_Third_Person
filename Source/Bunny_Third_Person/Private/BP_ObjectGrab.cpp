#include "BP_ObjectGrab.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Bunny_Third_PersonCharacter.h"

// Sets default values
ABP_ObjectGrab::ABP_ObjectGrab()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Collostion
	CollisonSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collison"));
	RootComponent = CollisonSphere;

	// Creating the Test Object
	MyObjectBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InterObject"));
	MyObjectBox->SetupAttachment(RootComponent);

	//Gravety settings
	MyObjectBox->SetSimulatePhysics(true);
	MyObjectBox->SetEnableGravity(true);
	MyObjectBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MyObjectBox->SetCollisionObjectType(ECC_WorldDynamic);
}

// Called when the game starts or when spawned
void ABP_ObjectGrab::BeginPlay()
{
	Super::BeginPlay();
	
	if (CollisonSphere) // add in the overlap 
	{
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
	if (OtherActor && OtherActor->IsA<ACharacter>()) 
	{
		ACharacter* Charather = Cast<ACharacter>(OtherActor);
		APlayerController* PlayerController = Cast<APlayerController>(Charather->Controller);

		if (PlayerController)
		{
			PlayerController->EnableInput(PlayerController);
		}
	}
}

void ABP_ObjectGrab::OnEndOverlap(UPrimitiveComponent* OverlappedCompnent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->IsA<ACharacter>())
	{
		ACharacter* Charather = Cast<ACharacter>(OtherActor);
		APlayerController* PlayerController = Cast<APlayerController>(Charather->Controller);

		if (PlayerController)
		{
			PlayerController->DisableInput(PlayerController);
		}
	}
}

void ABP_ObjectGrab::GrabObjcet()
{
	if (OverlappingCharacther && MyObjectBox)
	{
		OverlappingCharacther->PickupObject(MyObjectBox->GetStaticMesh());
	}
}

