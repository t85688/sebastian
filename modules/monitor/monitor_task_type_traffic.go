package monitor

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
)

type LinkTraffic struct {
	LinkId                        int64  `json:"LinkId"`
	SourceDeviceId                int64  `json:"SourceDeviceId"`
	SourceInterfaceId             int64  `json:"SourceInterfaceId"`
	SourceTrafficUtilization      int    `json:"SourceTrafficUtilization"`
	DestinationDeviceId           int64  `json:"DestinationDeviceId"`
	DestinationInterfaceId        int64  `json:"DestinationInterfaceId"`
	DestinationTrafficUtilization int    `json:"DestinationTrafficUtilization"`
	Speed                         uint64 `json:"Speed"`
	Timestamp                     int64  `json:"Timestamp"`
}

func (m *DefaultMonitor) makeMonitorTrafficUpdateLinksTask() MonitorTask {
	return func(ctx context.Context) error {
		if ctx.Err() != nil {
			return ctx.Err()
		}

		err := m.doMonitorTraffic(ctx)
		return err
	}
}

func (m *DefaultMonitor) onMonitorTrafficResult(ctx context.Context, trafficData []*LinkTraffic) {
	if ctx.Err() != nil {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onMonitorTrafficResult] No project ID found in context")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandMonitorTrafficUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionUpdate.String(),
		trafficData,
	)

	wsResp.Path = fmt.Sprintf("Projects/%v/Links", projectId)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal monitor traffic update response: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) doMonitorTraffic(ctx context.Context) error {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return fmt.Errorf("[doMonitorTraffic] project id not found in context")
	}

	collectTrafficData := func() []*LinkTraffic {
		result := []*LinkTraffic{}

		mafLinks := m.mafLinksCache.GetAll()
		if len(mafLinks) == 0 {
			return result
		}

		for _, mafLink := range mafLinks {
			if mafLink.Traffic == nil {
				continue
			}

			actLink, currentLinkExists := m.findActLinkInCurrentByMafLink(mafLink)
			if !currentLinkExists {
				continue
			}

			srcActDeviceId, srcActDeviceIdExists := m.actDeviceManager.GetActDeviceID(mafLink.FromDevice.DeviceId)
			if !srcActDeviceIdExists {
				continue
			}

			dstActDeviceId, dstActDeviceIdExists := m.actDeviceManager.GetActDeviceID(mafLink.ToDevice.DeviceId)
			if !dstActDeviceIdExists {
				continue
			}

			result = append(result, &LinkTraffic{
				LinkId:                        int64(actLink.Id),
				SourceDeviceId:                srcActDeviceId,
				SourceInterfaceId:             int64(mafLink.FromPort),
				SourceTrafficUtilization:      int(mafLink.Traffic.InUtilization / 10000),
				DestinationDeviceId:           dstActDeviceId,
				DestinationInterfaceId:        int64(mafLink.ToPort),
				DestinationTrafficUtilization: int(mafLink.Traffic.OutUtilization / 10000),
				Speed:                         mafLink.Speed / 1000000,
				Timestamp:                     int64(mafLink.Traffic.RecordTime),
			})
		}

		return result
	}

	if !m.isMonitoring(projectId) {
		return fmt.Errorf("monitor is not running")
	}

	if ctx.Err() != nil {
		return ctx.Err()
	}

	collectedTrafficData := collectTrafficData()
	m.onMonitorTrafficResult(ctx, collectedTrafficData)
	return nil
}
