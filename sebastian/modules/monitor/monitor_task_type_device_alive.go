package monitor

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type DeviceAliveStatus struct {
	DeviceId int64
	DeviceIP string
	Alive    bool
}

func (m *DefaultMonitor) makeMonitorDeviceAliveStatusTask() MonitorTask {
	return func(ctx context.Context) error {
		err := m.doMonitorDeviceAliveStatus(ctx)
		return err
	}
}

func (m *DefaultMonitor) onDeviceAliveResult(ctx context.Context, alives []*DeviceAliveStatus) {
	if ctx.Err() != nil {
		logger.Warnf("context error in onDeviceAliveResult: %v", ctx.Err())
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onDeviceAliveResult] No project ID found in context")
		return
	}

	wsData := make(wscommand.MonitorAliveDevices, len(alives))
	for i, alive := range alives {
		wsData[i] = &wscommand.MonitorAliveDevice{
			Alive:     alive.Alive,
			Id:        alive.DeviceId,
			IpAddress: alive.DeviceIP,
		}
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandMonitorAliveUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionUpdate.String(),
		wsData)

	wsResp.Path = fmt.Sprintf("Projects/%v/Devices", projectId)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal MonitorAliveDevices: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) doMonitorDeviceAliveStatus(ctx context.Context) error {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return fmt.Errorf("project id not found in context")
	}

	collectDevicesAliveStatus := func() []*DeviceAliveStatus {
		currentDevices := m.currentDeviceMap.GetAll()

		result := []*DeviceAliveStatus{}

		for actDeviceId, actDevice := range currentDevices {
			// check if in baseline
			actDeviceInBaseline, existsInBaseline := m.findActDeviceInBaseline(actDeviceId)

			// getMafDeviceByActDeviceId
			mafDevice, mafDeviceExists := m.findMafDeviceByActDeviceID(actDeviceId)

			if mafDeviceExists {
				isAlive := mafDevice.Communication != nil && mafDevice.Communication.ICMP.Status == netdl.CommunicationStatusReachable.String()

				// check if baseline device's profileId changed
				isProfileChanged := existsInBaseline && actDeviceInBaseline.DeviceProfileId != actDevice.DeviceProfileId
				if isProfileChanged {
					isAlive = false
				}

				result = append(result, &DeviceAliveStatus{
					DeviceId: actDeviceId,
					DeviceIP: actDevice.Ipv4.IpAddress,
					Alive:    isAlive,
				})

				continue
			}

			if !mafDeviceExists && existsInBaseline {
				result = append(result, &DeviceAliveStatus{
					DeviceId: actDeviceId,
					DeviceIP: actDevice.Ipv4.IpAddress,
					Alive:    false,
				})

				continue
			}

			logger.Warnf("No MAF Device found for Act Device ID %d", actDeviceId)
		}

		return result
	}

	if !m.isMonitoring(projectId) {
		return fmt.Errorf("monitor is not running")
	}

	if ctx.Err() != nil {
		return ctx.Err()
	}

	alives := collectDevicesAliveStatus()
	m.onDeviceAliveResult(ctx, alives)
	return nil

	/*
		Baseline Act Device ID List
		Baseline Act Device ID List not on MAF Device list
		MAF Device list on baseline
		MAF Device list not on baseline

		Baseline 上有的 device，NM Device 有通     Alive true
		Baseline 上有的 device，NM Device 沒通 	   Alive false
		Baseline 沒有的 device，NM Device 掃到有通  Alive true
		Baseline 沒有的 device，NM Device 掃到沒通  Alive false

		Baseline 上有的 link，NM link 有通     Alive true
		Baseline 上有的 link，NM link 沒通 	   Alive false
		Baseline 沒有的 link，NM link 掃到有通  Alive true
		Baseline 沒有的 link，NM link 掃到沒通  Alive false
	*/
}
