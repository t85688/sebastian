package monitor

import (
	"fmt"
	"strconv"
	"strings"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/ipcompare"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/macutility"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func (m *DefaultMonitor) findActDeviceIDByMafDevice(mafDevice *netdl.Device) (int64, bool) {
	actDeviceID, exists := m.actDeviceManager.GetActDeviceID(mafDevice.DeviceId)
	if !exists {
		return -1, false
	}

	return actDeviceID, true
}

func (m *DefaultMonitor) findActDeviceByMafDevice(mafDevice *netdl.Device) (*domain.Device, bool) {
	actDeviceId, actDeviceIdExists := m.actDeviceManager.GetActDeviceID(mafDevice.DeviceId)
	if !actDeviceIdExists {
		return nil, false
	}

	actDevice, actDeviceExists := m.currentDeviceMap.Get(actDeviceId)
	if !actDeviceExists {
		return nil, false
	}

	return actDevice, true
}

func (m *DefaultMonitor) findActDeviceInBaseline(actDeviceId int64) (*domain.Device, bool) {
	actDevice, exists := m.baselineDeviceMap.Get(actDeviceId)
	return actDevice, exists
}

func (m *DefaultMonitor) findActDeviceInCurrent(actDeviceId int64) (*domain.Device, bool) {
	actDevice, exists := m.currentDeviceMap.Get(actDeviceId)
	return actDevice, exists
}

func (m *DefaultMonitor) findActDeviceInCurrentByIP(deviceIP string) (int64, bool) {
	currentDevices := m.currentDeviceMap.GetAll()
	for _, dev := range currentDevices {
		if dev.Ipv4.IpAddress == deviceIP {
			return dev.Id, true
		}
	}

	return -1, false
}

func (m *DefaultMonitor) findActLinkInBaselineByLinkStr(linkStr string) (*domain.Link, bool) {
	actLink, exists := m.baselineLinkMap.Get(linkStr)
	return actLink, exists
}

func (m *DefaultMonitor) findActLinkInBaselineById(actLinkId int64) (*domain.Link, bool) {
	actLink, exists := m.baselineLinkMap.GetByIndex(indexNameActLinkId, strconv.FormatInt(actLinkId, 10))
	return actLink, exists
}

func (m *DefaultMonitor) findActLinkInCurrentByMafLink(mafLink *netdl.Link) (*domain.Link, bool) {
	currentLinks := m.currentLinkMap.GetAll()
	mafLinkStr, mafLinkStrErr := getMafLinkIdentifier(mafLink)

	if mafLinkStrErr != nil {
		return nil, false
	}

	if mafLinkStr == "" {
		return nil, false
	}

	for _, link := range currentLinks {
		actLinkStr, actLinkStrErr := m.getActCurrentLinkStr(link)
		if actLinkStrErr != nil {
			continue
		}

		if actLinkStr == mafLinkStr {
			return link, true
		}
	}

	return nil, false
}

func (m *DefaultMonitor) findMafDeviceByActDeviceID(actDeviceID int64) (*netdl.Device, bool) {
	mafDeviceId, exists := m.actDeviceManager.GetMAFDeviceID(actDeviceID)
	if !exists {
		return nil, false
	}

	mafDevice, exists := m.mafDevicesCache.Get(mafDeviceId)
	if !exists {
		return nil, false
	}

	return mafDevice, true
}

func (m *DefaultMonitor) findActDeviceInCurrentByMafIP(mafDevice *netdl.Device) (*domain.Device, bool) {
	currentDevices := m.currentDeviceMap.GetAll()

	for _, actDevice := range currentDevices {
		if actDevice.Ipv4.IpAddress != "" && actDevice.Ipv4.IpAddress == mafDevice.IP {
			return actDevice, true
		}
	}

	return nil, false
}

func (m *DefaultMonitor) findActDeviceInCurrentByMafIPMAC(mafDevice *netdl.Device) (*domain.Device, bool) {
	currentDevices := m.currentDeviceMap.GetAll()

	for _, actDevice := range currentDevices {
		if actDevice.MacAddress != "" && mafDevice.MAC != "" {
			parsedActDeviceMacAddress, err := macutility.ParseMACAddress(actDevice.MacAddress)
			if err != nil {
				continue
			}

			parsedMafDeviceMacAddress, err := macutility.ParseMACAddress(mafDevice.MAC)
			if err != nil {
				continue
			}

			if parsedActDeviceMacAddress.String() == parsedMafDeviceMacAddress.String() {
				return actDevice, true
			}
		}

		if actDevice.Ipv4.IpAddress != "" && actDevice.Ipv4.IpAddress == mafDevice.IP {
			return actDevice, true
		}
	}

	return nil, false
}

func (m *DefaultMonitor) findMafLinkByActLink(actLink *domain.Link) (*netdl.Link, bool) {
	actLinkStr, err := m.getActCurrentLinkStr(actLink)
	if err != nil {
		logger.Warnf("Failed to get link identity string: %v", err)
		return nil, false
	}

	mafLink, exists := m.mafLinksCache.Get(actLinkStr)
	if !exists {
		return nil, false
	}

	return mafLink, true
}

func (m *DefaultMonitor) getActLinkEndpointsStr(link *domain.Link, deviceMap map[int64]*domain.Device) ([]string, error) {
	result := make([]string, 0, 2)

	srcDev, srcExists := deviceMap[int64(link.SourceDeviceId)]
	if !srcExists {
		return nil, fmt.Errorf("source device not found")
	}

	dstDev, dstExists := deviceMap[int64(link.DestinationDeviceId)]
	if !dstExists {
		return nil, fmt.Errorf("destination device not found")
	}

	if link.SourceInterfaceId <= 0 {
		return nil, fmt.Errorf("invalid source interface id:%v", link.SourceInterfaceId)
	}

	if link.DestinationInterfaceId <= 0 {
		return nil, fmt.Errorf("invalid destination interface id:%v", link.DestinationInterfaceId)
	}

	srcDevIPStr := srcDev.Ipv4.IpAddress
	dstDevIPStr := dstDev.Ipv4.IpAddress

	compareIPResult := ipcompare.CompareIPByString(srcDevIPStr, dstDevIPStr)

	if compareIPResult == ipcompare.CompareIPResultFirstIPNotValid {
		return nil, fmt.Errorf("invalid source device IP address format: %q", srcDevIPStr)
	}

	if compareIPResult == ipcompare.CompareIPResultSecondIPNotValid {
		return nil, fmt.Errorf("invalid destination device IP address format: %q", dstDevIPStr)
	}

	srcIPStr, dstIPStr := srcDev.Ipv4.IpAddress, dstDev.Ipv4.IpAddress
	srcIfaceId, dstIfaceId := link.SourceInterfaceId, link.DestinationInterfaceId

	if compareIPResult == ipcompare.CompareIPResultGreater {
		srcIPStr, dstIPStr = dstDev.Ipv4.IpAddress, srcDev.Ipv4.IpAddress
		srcIfaceId, dstIfaceId = link.DestinationInterfaceId, link.SourceInterfaceId
	}

	result = append(result, fmt.Sprintf("%v:%v", srcIPStr, srcIfaceId))
	result = append(result, fmt.Sprintf("%v:%v", dstIPStr, dstIfaceId))

	return result, nil
}

func (m *DefaultMonitor) getActLinkStr(link *domain.Link, deviceMap map[int64]*domain.Device) (string, error) {
	endpointStrPair, err := m.getActLinkEndpointsStr(link, deviceMap)
	if err != nil {
		return "", err
	}

	return strings.Join(endpointStrPair, "-"), nil
}

func (m *DefaultMonitor) getActCurrentLinkStr(link *domain.Link) (string, error) {
	srcDev, srcExists := m.currentDeviceMap.Get(int64(link.SourceDeviceId))
	if !srcExists {
		return "", fmt.Errorf("source device not found")
	}

	dstDev, dstExists := m.currentDeviceMap.Get(int64(link.DestinationDeviceId))
	if !dstExists {
		return "", fmt.Errorf("destination device not found")
	}

	if link.SourceInterfaceId <= 0 {
		return "", fmt.Errorf("invalid source interface id:%v", link.SourceInterfaceId)
	}

	if link.DestinationInterfaceId <= 0 {
		return "", fmt.Errorf("invalid destination interface id:%v", link.DestinationInterfaceId)
	}

	srcDevIPStr := srcDev.Ipv4.IpAddress
	dstDevIPStr := dstDev.Ipv4.IpAddress

	compareIPResult := ipcompare.CompareIPByString(srcDevIPStr, dstDevIPStr)

	if compareIPResult == ipcompare.CompareIPResultFirstIPNotValid {
		return "", fmt.Errorf("invalid source device IP address format: %q", srcDevIPStr)
	}

	if compareIPResult == ipcompare.CompareIPResultSecondIPNotValid {
		return "", fmt.Errorf("invalid destination device IP address format: %q", dstDevIPStr)
	}

	srcIPStr, dstIPStr := srcDev.Ipv4.IpAddress, dstDev.Ipv4.IpAddress
	srcIfaceId, dstIfaceId := link.SourceInterfaceId, link.DestinationInterfaceId

	if compareIPResult == ipcompare.CompareIPResultGreater {
		srcIPStr, dstIPStr = dstDev.Ipv4.IpAddress, srcDev.Ipv4.IpAddress
		srcIfaceId, dstIfaceId = link.DestinationInterfaceId, link.SourceInterfaceId
	}

	return fmt.Sprintf("%v:%v-%v:%v", srcIPStr, srcIfaceId, dstIPStr, dstIfaceId), nil
}

func (m *DefaultMonitor) getActLinkStrInBaseline(link *domain.Link) (string, error) {
	srcDev, srcExists := m.baselineDeviceMap.Get(int64(link.SourceDeviceId))
	if !srcExists {
		return "", fmt.Errorf("source device not found")
	}

	dstDev, dstExists := m.baselineDeviceMap.Get(int64(link.DestinationDeviceId))
	if !dstExists {
		return "", fmt.Errorf("destination device not found")
	}

	if link.SourceInterfaceId <= 0 {
		return "", fmt.Errorf("invalid source interface id:%v", link.SourceInterfaceId)
	}

	if link.DestinationInterfaceId <= 0 {
		return "", fmt.Errorf("invalid destination interface id:%v", link.DestinationInterfaceId)
	}

	srcDevIPStr := srcDev.Ipv4.IpAddress
	dstDevIPStr := dstDev.Ipv4.IpAddress

	compareIPResult := ipcompare.CompareIPByString(srcDevIPStr, dstDevIPStr)

	if compareIPResult == ipcompare.CompareIPResultFirstIPNotValid {
		return "", fmt.Errorf("invalid source device IP address format: %q", srcDevIPStr)
	}

	if compareIPResult == ipcompare.CompareIPResultSecondIPNotValid {
		return "", fmt.Errorf("invalid destination device IP address format: %q", dstDevIPStr)
	}

	srcIPStr, dstIPStr := srcDev.Ipv4.IpAddress, dstDev.Ipv4.IpAddress
	srcIfaceId, dstIfaceId := link.SourceInterfaceId, link.DestinationInterfaceId

	if compareIPResult == ipcompare.CompareIPResultGreater {
		srcIPStr, dstIPStr = dstDev.Ipv4.IpAddress, srcDev.Ipv4.IpAddress
		srcIfaceId, dstIfaceId = link.DestinationInterfaceId, link.SourceInterfaceId
	}

	return fmt.Sprintf("%v:%v-%v:%v", srcIPStr, srcIfaceId, dstIPStr, dstIfaceId), nil
}

func (m *DefaultMonitor) getEndpointsByActLink(actLink *domain.Link) (*domain.Device, *domain.Device, error) {
	srcDevice, srcExists := m.currentDeviceMap.Get(int64(actLink.SourceDeviceId))
	if !srcExists {
		return nil, nil, fmt.Errorf("source device ID %d not found", actLink.SourceDeviceId)
	}

	dstDevice, dstExists := m.currentDeviceMap.Get(int64(actLink.DestinationDeviceId))
	if !dstExists {
		return nil, nil, fmt.Errorf("destination device ID %d not found", actLink.DestinationDeviceId)
	}

	return srcDevice, dstDevice, nil
}
