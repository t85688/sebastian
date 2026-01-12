package monitor

import (
	"fmt"
	"strconv"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/ipcompare"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func getMafLinkIdentifier(mafLink *netdl.Link) (string, error) {
	if mafLink == nil {
		return "", fmt.Errorf("mafLink is nil")
	}

	if mafLink.FromDevice.IP == "" ||
		mafLink.ToDevice.IP == "" {
		return "", fmt.Errorf("invalid mafLink: %v:%v-%v:%v", mafLink.FromDevice.IP, mafLink.FromPort, mafLink.ToDevice.IP, mafLink.ToPort)
	}

	srcDevIPStr, dstDevIPStr := mafLink.FromDevice.IP, mafLink.ToDevice.IP

	compareIPResult := ipcompare.CompareIPByString(srcDevIPStr, dstDevIPStr)

	if compareIPResult == ipcompare.CompareIPResultFirstIPNotValid {
		return "", fmt.Errorf("invalid source device IP address format: %q", srcDevIPStr)
	}

	if compareIPResult == ipcompare.CompareIPResultSecondIPNotValid {
		return "", fmt.Errorf("invalid destination device IP address format: %q", dstDevIPStr)
	}

	srcPortId, dstPortId := mafLink.FromPort, mafLink.ToPort
	if compareIPResult == ipcompare.CompareIPResultGreater {
		srcDevIPStr, dstDevIPStr = dstDevIPStr, srcDevIPStr
		srcPortId, dstPortId = dstPortId, srcPortId
	}

	return fmt.Sprintf("%s:%v-%s:%v", srcDevIPStr, srcPortId, dstDevIPStr, dstPortId), nil
}

func temperatureCToF(tempCStr string) (string, error) {
	if tempCStr == "" {
		return "", fmt.Errorf("empty temperatureC")
	}

	tempCFloat64, err := strconv.ParseFloat(tempCStr, 64)
	if err != nil {
		return "", fmt.Errorf("invalid temperature input: %w", err)
	}

	tempFFloat64 := (tempCFloat64 * 9.0 / 5.0) + 32.0

	return fmt.Sprintf("%.2f", tempFFloat64), nil
}

func getUnknownDeviceProfile(simpleProfiles *domain.SimpleDeviceProfiles) *domain.SimpleDeviceProfile {
	for _, profile := range simpleProfiles.Profiles {
		if profile.ModelName == "Unknown" {
			return &profile
		}
	}

	return nil
}
