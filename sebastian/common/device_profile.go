package common

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func GetDeviceProfileById(simpleProfiles *domain.SimpleDeviceProfiles, profileId int64) *domain.SimpleDeviceProfile {
	for _, profile := range simpleProfiles.Profiles {
		if profileId == profile.Id {
			return &profile
		}
	}

	return nil
}

func FindDeviceProfileByModelNameAndModules(simpleProfiles *domain.SimpleDeviceProfiles, modelName string, modules *netdl.Modules) *domain.SimpleDeviceProfile {

	var buildInPower string

	if simpleProfiles == nil {
		Logger.Warnf("input simpleProfiles is nil")
		return nil
	}

	if modules != nil && len(modules.Power) > 0 {
		for _, pm := range modules.Power {
			if pm != nil {
				if pm.SlotID == 1 {
					buildInPower = pm.ModuleName
				}
			}
		}
	}

	// var tmp_profile *domain.SimpleDeviceProfile
	var need_advanced_mapping = false
	for _, profile := range simpleProfiles.Profiles {
		if modelName != "" && profile.PhysicalModelName == modelName {
			// if tmp_profile == nil {
			// 	tmp_profile = &profile
			// }

			if profile.PhysicalModelName == profile.ModelName {
				return &profile
			} else {
				need_advanced_mapping = true
			}

			if buildInPower == "" {
				Logger.Warnf("Device buildInPower is empty")
				return nil
				// return &profile
			}

			// For eds to advance mapping module
			// Try to Mapping buildInPower
			for _, value := range profile.SupportPowerModules {
				Logger.Infof("SupportPowerModules value(%s)", value)

				if value == buildInPower {
					return &profile
				}
			}
		}
	}

	if need_advanced_mapping {
		Logger.Warnf("Device buildInPower(%s) not match any DeviceProfile", buildInPower)
	}

	// if tmp_profile != nil {
	// 	Logger.Info(fmt.Sprintf("Not Mapping the BuildInPower(%s) so use the first match PhysicalModelName profile", buildInPower))
	// 	return tmp_profile
	// }

	return nil
}

func GetUnknownDeviceProfile(simpleProfiles *domain.SimpleDeviceProfiles) *domain.SimpleDeviceProfile {
	for _, profile := range simpleProfiles.Profiles {
		if profile.ModelName == "Unknown" {
			return &profile
		}
	}
	return nil
}
