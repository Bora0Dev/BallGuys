// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BallGuysSettingsWidget.generated.h"

class UCheckBox;
class USlider;
class UComboBoxString;
class UButton;
class USoundMix;
class USoundClass;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSettingsClosed);

/**
 * Settings menu widget supporting Input (Invert X/Y), Video/Display (UGameUserSettings),
 * and Audio volume sliders (Master/Music/SFX).
 * All UMG widget bindings are optional so you can add or omit controls freely in your Widget Blueprint.
 */
UCLASS()
class BALLGUYS_API UBallGuysSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- Input Settings ---

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	void SetInvertX(bool bInvert);

	UFUNCTION(BlueprintCallable, Category = "Settings|Input")
	void SetInvertY(bool bInvert);

	// --- Audio Settings ---

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusicVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetSFXVolume(float Volume);

	// --- Video / Display Settings ---

	/** 0 = Fullscreen, 1 = Windowed Fullscreen, 2 = Windowed */
	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetWindowModeByIndex(int32 WindowModeIndex);

	/** Parses "WidthxHeight" (e.g. "1920x1080") and sets resolution */
	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetResolutionFromString(const FString& ResolutionString);

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetVSyncEnabled(bool bEnable);

	/** 0 = Low, 1 = Medium, 2 = High, 3 = Epic, 4 = Cinematic */
	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void SetGraphicsQuality(int32 QualityLevel);

	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void ApplyVideoSettings();

	// --- Navigation ---

	/** Refresh all bound UI controls from current GameInstance and GameUserSettings values */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadCurrentSettingsIntoUI();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void CloseSettings();

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsClosed OnSettingsClosed;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// --- Optional Audio Assets (assign in WBP defaults if using SoundMix overrides) ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundMix> DefaultSoundMix;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> SFXSoundClass;

	// --- Optional UMG Bindings (auto-bound if named matching in your Widget Blueprint) ---

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UCheckBox> InvertXCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UCheckBox> InvertYCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<USlider> MasterVolumeSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<USlider> MusicVolumeSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<USlider> SFXVolumeSlider;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UComboBoxString> WindowModeComboBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UComboBoxString> ResolutionComboBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UCheckBox> VSyncCheckBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UComboBoxString> GraphicsQualityComboBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Settings|UI")
	TObjectPtr<UButton> BackButton;

private:
	UFUNCTION()
	void HandleInvertXChanged(bool bIsChecked);

	UFUNCTION()
	void HandleInvertYChanged(bool bIsChecked);

	UFUNCTION()
	void HandleMasterVolumeChanged(float Value);

	UFUNCTION()
	void HandleMusicVolumeChanged(float Value);

	UFUNCTION()
	void HandleSFXVolumeChanged(float Value);

	UFUNCTION()
	void HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleVSyncChanged(bool bIsChecked);

	UFUNCTION()
	void HandleGraphicsQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleBackClicked();

	void ApplySoundClassVolume(USoundClass* SoundClass, float Volume);
};
