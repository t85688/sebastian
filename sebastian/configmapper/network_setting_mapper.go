package configmapper

import "gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"

func MapNetworkSettingMode(input string) domain.NetworkSettingMode {
	switch input {
	case "static":
		return domain.NetworkSettingModeStatic
	case "dhcp":
		return domain.NetworkSettingModeDHCP
	case "bootp":
		return domain.NetworkSettingModeBootp
	}

	return domain.NetworkSettingModeUnknown
}
