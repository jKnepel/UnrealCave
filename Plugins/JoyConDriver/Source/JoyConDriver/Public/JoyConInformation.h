// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JoyConInformation.generated.h"

USTRUCT(BlueprintType)
struct FJoyConInformation {
	GENERATED_USTRUCT_BODY()

public:
	FJoyConInformation();
	FJoyConInformation(int TempProductId, int TempVendorId, int TempInterfaceNumber, int TempReleaseNumber, FString TempManufacturerString,
		FString TempBluetoothPath, FString TempProductString, FString TempSerialNumber, int TempControllerId, int TempUsage,
		int TempUsagePage, bool TempIsLeft, bool TempIsConnected);

	/*
	 * Device Info
	 */

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int ProductId;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int VendorId;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int InterfaceNumber;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int ReleaseNumber;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		FString ManufacturerString;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		FString BluetoothPath;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		FString ProductString;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		FString SerialNumber;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int ControllerId;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int Usage;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		int UsagePage;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		bool IsLeft;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		bool IsConnected;

	UPROPERTY(Category="JoyConDriver", EditAnywhere, BlueprintReadOnly)
		bool IsAttached;
};
