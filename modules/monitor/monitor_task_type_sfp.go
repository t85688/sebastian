package monitor

import (
	"context"
	"fmt"
	"runtime/debug"
	"strconv"

	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type SFPLinks []*SFPLink

type SFPLink struct {
	LinkId int64
	Source SFPPort
	Target SFPPort
}

type SFPPort struct {
	DeviceId          int64
	DeviceIp          string
	InterfaceId       int
	InterfaceName     string
	ModelName         string
	RxPower           string
	RxPowerLimit      []string
	SerialNumber      string
	TemperatureC      string
	TemperatureF      string
	TemperatureLimitC string
	TemperatureLimitF string
	TxPower           string
	TxPowerLimit      []string
	Voltage           string
	Wavelength        string
}

func (m *DefaultMonitor) makeMonitorSFPTask() MonitorTask {
	return func(ctx context.Context) error {
		sfpLinks, err := m.collectSFP(ctx)
		if err != nil {
			logger.Warnf("failed to collect sfp links: %v", err)
			return err
		}

		m.updateSFPCache(sfpLinks)
		return nil
	}
}

func (m *DefaultMonitor) collectSFP(ctx context.Context) (SFPLinks, error) {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return nil, fmt.Errorf("project id not found in context")
	}

	collectSFPList := func() SFPLinks {
		defer func() {
			if r := recover(); r != nil {
				logger.Errorf("panic recovered in collectSFPList: %v, callstack: %v", r, string(debug.Stack()))
			}
		}()

		mafLinksWithSFP := make([]*netdl.Link, 0)
		mafLinks := m.mafLinksCache.GetAll()
		sfpLinks := make(SFPLinks, 0)

		/*
			maf port
			maf link with sfp
			maf link port index to get device and port
			maf device to get act device
		*/

		// MAF v0.3.0 support
		for _, mafLink := range mafLinks {
			if mafLink.LinkType != nil && mafLink.LinkType[netdl.LinkTypeSFP.String()] {
				mafLinksWithSFP = append(mafLinksWithSFP, mafLink)
			}
		}

		for _, mafLink := range mafLinksWithSFP {
			actLink, actLinkExists := m.findActLinkInCurrentByMafLink(mafLink)
			if !actLinkExists {
				continue
			}

			mafLinkStr, err := getMafLinkIdentifier(mafLink)
			if err != nil {
				logger.Warnf("failed to get mafLinkStr by mafLink: %v, err: %+v", mafLink, err)
				continue
			}

			srcMafDevice, srcMafDeviceExist := m.mafDevicesCache.Get(mafLink.FromDevice.DeviceId)
			if !srcMafDeviceExist {
				logger.Warnf("failed to find src maf device by mafLink: %v", mafLinkStr)
				continue
			}

			dstMafDevice, dstMafDeviceExist := m.mafDevicesCache.Get(mafLink.ToDevice.DeviceId)
			if !dstMafDeviceExist {
				logger.Warnf("failed to find dst maf device by mafLink: %v", mafLinkStr)
				continue
			}

			hasSrcSFPPort := false
			var srcMafPort *netdl.Port
			for _, port := range srcMafDevice.Ports {
				if port.ID == mafLink.FromPort && port.PortSFP != nil && port.PortSFP.Model != "" {
					hasSrcSFPPort = true
					srcMafPort = port
				}
			}

			if !hasSrcSFPPort {
				logger.Warnf("no sfp port found in src maf device by mafLink: %v", mafLinkStr)
				continue
			}

			hasDstSFPPort := false
			var dstMafPort *netdl.Port
			for _, port := range dstMafDevice.Ports {
				if port.ID == mafLink.ToPort && port.PortSFP != nil && port.PortSFP.Model != "" {
					hasDstSFPPort = true
					dstMafPort = port
				}
			}

			if !hasDstSFPPort {
				logger.Warnf("no sfp port found in dst maf device by mafLink: %v", mafLinkStr)
				continue
			}

			srcActDevice, dstActDevice, err := m.getEndpointsByActLink(actLink)
			if err != nil {
				logger.Warnf("failed to get endpoints by actLink: %v, err: %v", actLink, err)
				continue
			}

			srcTempC := ""
			srcTempF := ""
			srcMafPortSFP := srcMafPort.PortSFP
			if srcMafPortSFP != nil {
				if srcMafPortSFP.Temperature != "" {
					srcTempC = srcMafPortSFP.Temperature
					srcTempF, err = temperatureCToF(srcMafPortSFP.Temperature)
					if err != nil {
						srcTempF = ""
					}
				}
			}

			// MAF Port.AliasName 目前的值是錯的 (目前是 Port.Description)
			interfaceName := strconv.Itoa(srcMafPort.ID)
			srcPort := SFPPort{
				DeviceId:          srcActDevice.Id,
				DeviceIp:          srcActDevice.Ipv4.IpAddress,
				InterfaceId:       srcMafPort.ID,
				InterfaceName:     interfaceName,
				ModelName:         srcMafPort.PortSFP.Model,
				RxPower:           srcMafPort.PortSFP.RxPower,
				RxPowerLimit:      srcMafPort.PortSFP.RxPowerWarn, // mafLink.SFP.FromRxPowerLimit,
				SerialNumber:      srcMafPort.PortSFP.SN,          //mafLink.SFP.FromSerialNumber,
				TemperatureC:      srcTempC,
				TemperatureF:      srcTempF,
				TemperatureLimitC: "",
				TemperatureLimitF: "",
				TxPower:           srcMafPort.PortSFP.TxPower,
				TxPowerLimit:      srcMafPort.PortSFP.TxPowerWarn,
				Voltage:           srcMafPort.PortSFP.Voltage,
				Wavelength:        "",
			}

			dstTempC := ""
			dstTempF := ""
			dstMafPortSFP := dstMafPort.PortSFP
			if dstMafPortSFP != nil {
				if dstMafPortSFP.Temperature != "" {
					dstTempC = dstMafPortSFP.Temperature
					dstTempF, err = temperatureCToF(dstMafPortSFP.Temperature)
					if err != nil {
						dstTempF = ""
					}
				}
			}

			interfaceName = strconv.Itoa(dstMafPort.ID)
			dstPort := SFPPort{
				DeviceId:          dstActDevice.Id,
				DeviceIp:          dstActDevice.Ipv4.IpAddress,
				InterfaceId:       dstMafPort.ID,
				InterfaceName:     interfaceName,
				ModelName:         dstMafPort.PortSFP.Model,
				RxPower:           dstMafPort.PortSFP.RxPower,
				RxPowerLimit:      dstMafPort.PortSFP.RxPowerWarn,
				SerialNumber:      dstMafPort.PortSFP.SN,
				TemperatureC:      dstTempC,
				TemperatureF:      dstTempF,
				TemperatureLimitC: "",
				TemperatureLimitF: "",
				TxPower:           dstMafPort.PortSFP.TxPower,
				TxPowerLimit:      dstMafPort.PortSFP.TxPowerWarn,
				Voltage:           dstMafPort.PortSFP.Voltage,
				Wavelength:        "",
			}

			sfpLink := SFPLink{
				LinkId: int64(actLink.Id),
				Source: srcPort,
				Target: dstPort,
			}

			sfpLinks = append(sfpLinks, &sfpLink)
		}

		return sfpLinks
	}

	if !m.isMonitoring(projectId) {
		return nil, fmt.Errorf("monitor is not running")
	}

	if ctx.Err() != nil {
		return nil, ctx.Err()
	}

	sfpLinks := collectSFPList()
	return sfpLinks, nil
}

func (m *DefaultMonitor) updateSFPCache(sfpLinks SFPLinks) {
	sfpLinkMap := make(map[int64]*SFPLink, len(sfpLinks))
	for _, sfpLink := range sfpLinks {
		sfpLinkMap[sfpLink.LinkId] = sfpLink
	}

	m.sfpCache.SetAll(sfpLinkMap)
}
