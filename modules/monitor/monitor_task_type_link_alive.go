package monitor

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type LinkAliveStatus struct {
	Id                     int64  `json:"Id"`
	Alive                  bool   `json:"Alive"`
	SourceDeviceId         int64  `json:"SourceDeviceId"`
	SourceDeviceIp         string `json:"SourceDeviceIp"`
	SourceInterfaceId      int64  `json:"SourceInterfaceId"`
	DestinationDeviceId    int64  `json:"DestinationDeviceId"`
	DestinationDeviceIp    string `json:"DestinationDeviceIp"`
	DestinationInterfaceId int64  `json:"DestinationInterfaceId"`
	Redundancy             bool   `json:"Redundancy"`
}

func (m *DefaultMonitor) makeMonitorLinkAliveStatusTask() MonitorTask {
	return func(ctx context.Context) error {
		err := m.doMonitorLinkAliveStatus(ctx)
		return err
	}
}

func (m *DefaultMonitor) onLinkAliveResult(ctx context.Context, alives []*LinkAliveStatus) {
	if ctx.Err() != nil {
		logger.Warnf("context error in onLinkAliveResult: %v", ctx.Err())
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onLinkAliveResult] No project ID found in context")
		return
	}

	wsData := make(wscommand.MonitorAliveLinks, len(alives))
	for i, alive := range alives {
		wsData[i] = &wscommand.MonitorAliveLink{
			Id:                     alive.Id,
			Alive:                  alive.Alive,
			SourceDeviceId:         alive.SourceDeviceId,
			SourceDeviceIp:         alive.SourceDeviceIp,
			SourceInterfaceId:      alive.SourceInterfaceId,
			DestinationDeviceId:    alive.DestinationDeviceId,
			DestinationDeviceIp:    alive.DestinationDeviceIp,
			DestinationInterfaceId: alive.DestinationInterfaceId,
			Redundancy:             alive.Redundancy,
		}
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandMonitorAliveUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionUpdate.String(),
		wsData)

	wsResp.Path = fmt.Sprintf("Projects/%v/Links", projectId)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal MonitorAliveDevices: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) doMonitorLinkAliveStatus(ctx context.Context) error {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return fmt.Errorf("[doMonitorLinkAliveStatus] project id not found in context")
	}

	collectLinksAliveStatus := func() []*LinkAliveStatus {
		result := []*LinkAliveStatus{}
		for _, actLink := range m.currentLinkMap.GetAll() {

			mafLink, existsMafLink := m.findMafLinkByActLink(actLink)

			srcDevice, dstDevice, err := m.getEndpointsByActLink(actLink)
			if err != nil {
				logger.Warnf("Failed to get endpoints for Act Link ID %d: %v", actLink.Id, err)
				continue
			}

			linkStr, err := m.getActCurrentLinkStr(actLink)
			if err != nil {
				logger.Warnf("Failed to get link string for Act Link ID %d: %v", actLink.Id, err)
				continue
			}

			// check if in baseline
			_, existsInBaseline := m.findActLinkInBaselineByLinkStr(linkStr)

			if existsMafLink {
				isRedundant := mafLink.LinkType != nil && mafLink.LinkType[netdl.LinkTypeRstpRedundancy.String()]

				alive := mafLink.Status == string(netdl.InterfaceStatusUp)

				result = append(result, &LinkAliveStatus{
					Id:                     int64(actLink.Id),
					SourceDeviceId:         int64(actLink.SourceDeviceId),
					SourceDeviceIp:         srcDevice.Ipv4.IpAddress,
					SourceInterfaceId:      int64(actLink.SourceInterfaceId),
					DestinationDeviceId:    int64(actLink.DestinationDeviceId),
					DestinationDeviceIp:    dstDevice.Ipv4.IpAddress,
					DestinationInterfaceId: int64(actLink.DestinationInterfaceId),
					Alive:                  alive,
					Redundancy:             isRedundant,
				})
			}

			if !existsMafLink && existsInBaseline {
				result = append(result, &LinkAliveStatus{
					Id:                     int64(actLink.Id),
					SourceDeviceId:         int64(actLink.SourceDeviceId),
					SourceDeviceIp:         srcDevice.Ipv4.IpAddress,
					SourceInterfaceId:      int64(actLink.SourceInterfaceId),
					DestinationDeviceId:    int64(actLink.DestinationDeviceId),
					DestinationDeviceIp:    dstDevice.Ipv4.IpAddress,
					DestinationInterfaceId: int64(actLink.DestinationInterfaceId),
					Alive:                  false,
					Redundancy:             false,
				})
			}
		}

		return result
	}

	if !m.isMonitoring(projectId) {
		return fmt.Errorf("monitor is not running")
	}

	if ctx.Err() != nil {
		return ctx.Err()
	}

	alives := collectLinksAliveStatus()
	m.onLinkAliveResult(ctx, alives)
	return nil
}
