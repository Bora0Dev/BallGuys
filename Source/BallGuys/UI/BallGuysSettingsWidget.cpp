// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/BallGuysSettingsWidget.h"
#include "Game/BallGuysGameInstance.h"
#include "Components/CheckBox.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

void UBallGuysSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (InvertXCheckBox)
	{
		InvertXCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleInvertXChanged);
	}
	if (InvertYCheckBox)
	{
		InvertYCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleInvertYChanged);
	}
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleMasterVolumeChanged);
	}
	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleMusicVolumeChanged);
	}
	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.AddDynamic(this, &ThisClass::HandleSFXVolumeChanged);
	}
	if (WindowModeComboBox)
	{
		if (WindowModeComboBox->GetOptionCount() == 0)
		{
			WindowModeComboBox->AddOption(TEXT("Fullscreen"));
			WindowModeComboBox->AddOption(TEXT("Windowed Fullscreen"));
			WindowModeComboBox->AddOption(TEXT("Windowed"));
		}
		WindowModeComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleWindowModeChanged);
	}
	if (ResolutionComboBox)
	{
		if (ResolutionComboBox->GetOptionCount() == 0)
		{
			ResolutionComboBox->AddOption(TEXT("1280x720"));
			ResolutionComboBox->AddOption(TEXT("1920x1080"));
			ResolutionComboBox->AddOption(TEXT("2560x1440"));
			ResolutionComboBox->AddOption(TEXT("3840x2160"));
		}
		ResolutionComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleResolutionChanged);
	}
	if (VSyncCheckBox)
	{
		VSyncCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::HandleVSyncChanged);
	}
	if (GraphicsQualityComboBox)
	{
		if (GraphicsQualityComboBox->GetOptionCount() == 0)
		{
			GraphicsQualityComboBox->AddOption(TEXT("Low"));
			GraphicsQualityComboBox->AddOption(TEXT("Medium"));
			GraphicsQualityComboBox->AddOption(TEXT("High"));
			GraphicsQualityComboBox->AddOption(TEXT("Epic"));
			GraphicsQualityComboBox->AddOption(TEXT("Cinematic"));
		}
		GraphicsQualityComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleGraphicsQualityChanged);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddDynamic(this, &ThisClass::HandleApplyClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(this, &ThisClass::HandleBackClicked);
	}

	LoadCurrentSettingsIntoUI();
}

void UBallGuysSettingsWidget::NativeDestruct()
{
	if (InvertXCheckBox)
	{
		InvertXCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::HandleInvertXChanged);
	}
	if (InvertYCheckBox)
	{
		InvertYCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::HandleInvertYChanged);
	}
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &ThisClass::HandleMasterVolumeChanged);
	}
	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->OnValueChanged.RemoveDynamic(this, &ThisClass::HandleMusicVolumeChanged);
	}
	if (SFXVolumeSlider)
	{
		SFXVolumeSlider->OnValueChanged.RemoveDynamic(this, &ThisClass::HandleSFXVolumeChanged);
	}
	if (WindowModeComboBox)
	{
		WindowModeComboBox->OnSelectionChanged.RemoveDynamic(this, &ThisClass::HandleWindowModeChanged);
	}
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.RemoveDynamic(this, &ThisClass::HandleResolutionChanged);
	}
	if (VSyncCheckBox)
	{
		VSyncCheckBox->OnCheckStateChanged.RemoveDynamic(this, &ThisClass::HandleVSyncChanged);
	}
	if (GraphicsQualityComboBox)
	{
		GraphicsQualityComboBox->OnSelectionChanged.RemoveDynamic(this, &ThisClass::HandleGraphicsQualityChanged);
	}
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleApplyClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleBackClicked);
	}

	Super::NativeDestruct();
}

void UBallGuysSettingsWidget::LoadCurrentSettingsIntoUI()
{
	if (const UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		if (InvertXCheckBox)
		{
			InvertXCheckBox->SetIsChecked(GI->bInvertXPref);
		}
		if (InvertYCheckBox)
		{
			InvertYCheckBox->SetIsChecked(GI->bInvertYPref);
		}
		if (MasterVolumeSlider)
		{
			MasterVolumeSlider->SetValue(GI->MasterVolume);
		}
		if (MusicVolumeSlider)
		{
			MusicVolumeSlider->SetValue(GI->MusicVolume);
		}
		if (SFXVolumeSlider)
		{
			SFXVolumeSlider->SetValue(GI->SFXVolume);
		}
	}

	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		if (WindowModeComboBox)
		{
			switch (UserSettings->GetFullscreenMode())
			{
			case EWindowMode::Fullscreen:
				WindowModeComboBox->SetSelectedIndex(0);
				break;
			case EWindowMode::WindowedFullscreen:
				WindowModeComboBox->SetSelectedIndex(1);
				break;
			default:
				WindowModeComboBox->SetSelectedIndex(2);
				break;
			}
		}

		if (ResolutionComboBox)
		{
			const FIntPoint Res = UserSettings->GetScreenResolution();
			const FString ResString = FString::Printf(TEXT("%dx%d"), Res.X, Res.Y);
			if (ResolutionComboBox->FindOptionIndex(ResString) == INDEX_NONE)
			{
				ResolutionComboBox->AddOption(ResString);
			}
			ResolutionComboBox->SetSelectedOption(ResString);
		}

		if (VSyncCheckBox)
		{
			VSyncCheckBox->SetIsChecked(UserSettings->IsVSyncEnabled());
		}

		if (GraphicsQualityComboBox)
		{
			const int32 Quality = FMath::Clamp(UserSettings->GetOverallScalabilityLevel(), 0, 4);
			GraphicsQualityComboBox->SetSelectedIndex(Quality);
		}
	}
}

void UBallGuysSettingsWidget::SetInvertX(bool bInvert)
{
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->bInvertXPref = bInvert;
	}
}

void UBallGuysSettingsWidget::SetInvertY(bool bInvert)
{
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->bInvertYPref = bInvert;
	}
}

void UBallGuysSettingsWidget::SetMasterVolume(float Volume)
{
	const float Clamped = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->MasterVolume = Clamped;
	}
	ApplySoundClassVolume(MasterSoundClass, Clamped);
}

void UBallGuysSettingsWidget::SetMusicVolume(float Volume)
{
	const float Clamped = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->MusicVolume = Clamped;
	}
	ApplySoundClassVolume(MusicSoundClass, Clamped);
}

void UBallGuysSettingsWidget::SetSFXVolume(float Volume)
{
	const float Clamped = FMath::Clamp(Volume, 0.0f, 1.0f);
	if (UBallGuysGameInstance* GI = GetGameInstance<UBallGuysGameInstance>())
	{
		GI->SFXVolume = Clamped;
	}
	ApplySoundClassVolume(SFXSoundClass, Clamped);
}

void UBallGuysSettingsWidget::ApplySoundClassVolume(USoundClass* SoundClass, float Volume)
{
	if (DefaultSoundMix && SoundClass && GetWorld())
	{
		UGameplayStatics::SetSoundMixClassOverride(
			GetWorld(),
			DefaultSoundMix,
			SoundClass,
			Volume,
			1.0f,
			0.0f,
			true);
		UGameplayStatics::PushSoundMixModifier(GetWorld(), DefaultSoundMix);
	}
}

void UBallGuysSettingsWidget::SetWindowModeByIndex(int32 WindowModeIndex)
{
	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		switch (WindowModeIndex)
		{
		case 0:
			UserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
			break;
		case 1:
			UserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
			break;
		default:
			UserSettings->SetFullscreenMode(EWindowMode::Windowed);
			break;
		}
	}
}

void UBallGuysSettingsWidget::SetResolutionFromString(const FString& ResolutionString)
{
	FString WidthStr, HeightStr;
	if (ResolutionString.Split(TEXT("x"), &WidthStr, &HeightStr))
	{
		const int32 Width = FCString::Atoi(*WidthStr);
		const int32 Height = FCString::Atoi(*HeightStr);
		if (Width > 0 && Height > 0)
		{
			if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
			{
				UserSettings->SetScreenResolution(FIntPoint(Width, Height));
			}
		}
	}
}

void UBallGuysSettingsWidget::SetVSyncEnabled(bool bEnable)
{
	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->SetVSyncEnabled(bEnable);
	}
}

void UBallGuysSettingsWidget::SetGraphicsQuality(int32 QualityLevel)
{
	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->SetOverallScalabilityLevel(FMath::Clamp(QualityLevel, 0, 4));
	}
}

void UBallGuysSettingsWidget::ApplyVideoSettings()
{
	if (UGameUserSettings* UserSettings = UGameUserSettings::GetGameUserSettings())
	{
		UserSettings->ApplySettings(false);
	}
}

void UBallGuysSettingsWidget::CloseSettings()
{
	ApplyVideoSettings();
	OnSettingsClosed.Broadcast();
}

void UBallGuysSettingsWidget::HandleInvertXChanged(bool bIsChecked)
{
	SetInvertX(bIsChecked);
}

void UBallGuysSettingsWidget::HandleInvertYChanged(bool bIsChecked)
{
	SetInvertY(bIsChecked);
}

void UBallGuysSettingsWidget::HandleMasterVolumeChanged(float Value)
{
	SetMasterVolume(Value);
}

void UBallGuysSettingsWidget::HandleMusicVolumeChanged(float Value)
{
	SetMusicVolume(Value);
}

void UBallGuysSettingsWidget::HandleSFXVolumeChanged(float Value)
{
	SetSFXVolume(Value);
}

void UBallGuysSettingsWidget::HandleWindowModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (WindowModeComboBox)
	{
		SetWindowModeByIndex(WindowModeComboBox->GetSelectedIndex());
	}
}

void UBallGuysSettingsWidget::HandleResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	SetResolutionFromString(SelectedItem);
}

void UBallGuysSettingsWidget::HandleVSyncChanged(bool bIsChecked)
{
	SetVSyncEnabled(bIsChecked);
}

void UBallGuysSettingsWidget::HandleGraphicsQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (GraphicsQualityComboBox)
	{
		SetGraphicsQuality(GraphicsQualityComboBox->GetSelectedIndex());
	}
}

void UBallGuysSettingsWidget::HandleApplyClicked()
{
	ApplyVideoSettings();
}

void UBallGuysSettingsWidget::HandleBackClicked()
{
	CloseSettings();
}
