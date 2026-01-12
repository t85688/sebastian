package monitor

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

// max 61440
var validBridgePriorityMap = map[int]bool{
	0:     true,
	4096:  true,
	8192:  true,
	12288: true,
	16384: true,
	20480: true,
	24576: true,
	28672: true,
	32768: true,
	36864: true,
	40960: true,
	45056: true,
	49152: true,
	53248: true,
	57344: true,
	61440: true,
}

func isValidBridgePriority(priority int) bool {
	_, exists := validBridgePriorityMap[priority]
	return exists
}

type MonitorSwiftDevices []*MonitorSwiftDevice

type MonitorSwiftDevice struct {
	DeviceId int64  `json:"DeviceId"`
	DeviceIp string `json:"DeviceIp"`
	Offline  bool   `json:"Offline"`
	Online   bool   `json:"Online"`
}

func (m *DefaultMonitor) makeMonitorSwiftTask(baseline *domain.Project) MonitorTask {
	return func(ctx context.Context) error {
		if ctx.Err() != nil {
			return ctx.Err()
		}

		swiftDevices, err := m.collectSwiftDevices(ctx, baseline)
		if err != nil {
			logger.Warnf("failed to collect swift devices: %v", err)
		}

		if swiftDevices == nil {
			return nil
		}

		m.onSwiftDevices(ctx, swiftDevices)

		return nil
	}
}

func (m *DefaultMonitor) onSwiftDevices(ctx context.Context, swiftDevices MonitorSwiftDevices) {
	if ctx.Err() != nil {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onSwiftDevices] No project ID found in context")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandMonitorSwiftStatusUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionUpdate.String(),
		swiftDevices,
	)

	wsResp.Path = fmt.Sprintf("Projects/%v/Devices", projectId)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal monitor swift devices status response: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) collectSwiftDevices(ctx context.Context, baseline *domain.Project) (MonitorSwiftDevices, error) {
	if ctx.Err() != nil {
		return nil, ctx.Err()
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		return nil, fmt.Errorf("[collectSwiftDevices] project id not found in context")
	}

	if !baseline.TopologySetting.RedundantGroup.Swift.Active {
		return nil, nil
	}

	swiftSetting := baseline.TopologySetting.RedundantGroup.Swift
	swiftDeviceMap := make(map[int64]*MonitorSwiftDevice)
	swiftDevices := make(MonitorSwiftDevices, 0, len(swiftSetting.DeviceTierMap))

	// GetRootDevice
	actRootDeviceId := swiftSetting.RootDevice
	_, actRootDeviceExists := m.findActDeviceInCurrent(actRootDeviceId)
	mafRootDevice, mafRootDeviceExist := m.findMafDeviceByActDeviceID(actRootDeviceId)

	// GetBackupRootDevice
	actBackupRootDeviceId := swiftSetting.BackupRootDevice
	_, actBackupRootDeviceExists := m.findActDeviceInCurrent(actBackupRootDeviceId)
	mafBackupRootDevice, mafBackupRootDeviceExist := m.findMafDeviceByActDeviceID(actBackupRootDeviceId)

	RootOrBackupRootNotExists := !actRootDeviceExists || !actBackupRootDeviceExists || !mafRootDeviceExist || !mafBackupRootDeviceExist

	rootAlive := mafRootDeviceExist && mafRootDevice.Communication != nil && mafRootDevice.Communication.ICMP.Status == netdl.CommunicationStatusReachable.String()

	backupRootAlive := mafBackupRootDeviceExist && mafBackupRootDevice.Communication != nil && mafBackupRootDevice.Communication.ICMP.Status == netdl.CommunicationStatusReachable.String()

	if RootOrBackupRootNotExists || !rootAlive || !backupRootAlive {
		for actDeviceId, _ := range swiftSetting.DeviceTierMap {
			deviceIP := ""
			actDevice, actDeviceExists := m.findActDeviceInCurrent(actDeviceId)
			if actDeviceExists {
				deviceIP = actDevice.Ipv4.IpAddress
			}

			swiftDevices = append(swiftDevices, &MonitorSwiftDevice{
				DeviceId: actDeviceId,
				DeviceIp: deviceIP,
				Offline:  true,
				Online:   false,
			})
		}

		return swiftDevices, nil
	}

	// update swift by device alive status and rstp setting
	/*
		缺少
		link alive status 比對
		link 數量
		link port 是否與 baseline 相同
		RSTP Support
		RSTP Enabled (global、per port)
		Swift Support
	*/
	for actDeviceId, tier := range swiftSetting.DeviceTierMap {
		if tier < 0 {
			logger.Warnf("invalid tier %v for swift device %v", tier, actDeviceId)
			continue
		}

		actDevice, actDeviceExists := m.findActDeviceInCurrent(actDeviceId)
		mafDevice, mafDeviceExists := m.findMafDeviceByActDeviceID(actDeviceId)
		isAlive := false
		offline := true
		online := false
		deviceIP := ""
		if actDeviceExists {
			deviceIP = actDevice.Ipv4.IpAddress
		}

		if mafDeviceExists {
			isAlive = mafDevice.Communication != nil && mafDevice.Communication.ICMP.Status == netdl.CommunicationStatusReachable.String()
			hasRstpSetting := mafDevice.Configuration != nil && mafDevice.Configuration.RstpSetting != nil

			if isAlive && hasRstpSetting {
				rstpSetting := mafDevice.Configuration.RstpSetting
				isCorrectHelloTime := rstpSetting.HelloTime == 1
				// isValidBridgePriority := isValidSwiftBridgePriority(int(rstpSetting.BridgePriority))

				if isCorrectHelloTime {
					if tier == 0 {
						online = rstpSetting.BridgePriority == 4096 &&
							rstpSetting.Swift != nil && *rstpSetting.Swift &&
							rstpSetting.Revert != nil && *rstpSetting.Revert
					} else if tier == 1 {
						online = rstpSetting.BridgePriority == 8192 &&
							rstpSetting.Swift != nil && *rstpSetting.Swift &&
							rstpSetting.Revert != nil && !*rstpSetting.Revert
					} else if tier >= 2 {
						online = rstpSetting.BridgePriority == 32768 &&
							rstpSetting.Swift != nil && !*rstpSetting.Swift &&
							rstpSetting.Revert != nil && !*rstpSetting.Revert
					} else {
						continue
					}
				}

			}
		}

		swiftDeviceMap[actDeviceId] = &MonitorSwiftDevice{
			DeviceId: actDeviceId,
			DeviceIp: deviceIP,
			Offline:  offline,
			Online:   online,
		}
	}

	// todo: update swiftDevice by link

	// fill result
	for _, device := range swiftDeviceMap {
		swiftDevices = append(swiftDevices, device)
	}

	if !m.isMonitoring(projectId) {
		return nil, fmt.Errorf("monitor is not running")
	}

	if ctx.Err() != nil {
		return nil, ctx.Err()
	}

	return swiftDevices, nil
}
