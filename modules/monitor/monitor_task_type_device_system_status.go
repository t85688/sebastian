package monitor

import (
	"context"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
)

type DeviceSystemStatus struct {
	DeviceId   int64
	DeviceIp   string
	Alias      string
	MacAddress string
	ModelName  string

	ModularInfo       *DeviceModularInfo
	FirmwareVersion   string
	ProductRevision   string
	SerialNumber      string
	DeviceName        string
	Role              string
	RedundantProtocol []string
	CPUUsage          string
	MemoryUsage       string
	SystemUptime      *int64
	Interfaces        []*DeviceInterfaceEntry
}

type DeviceInterfaceEntry struct {
	InterfaceId   int64
	InterfaceName string
	Description   string
	MacAddress    string
	// IpAddress     string
	Active bool
}

type DeviceModularInfo struct {
	Ethernet map[int]EthernetModule
	Power    map[int]PowerModule
}

type EthernetModule struct {
	Exist           bool
	ModuleName      string
	SerialNumber    string
	ProductRevision string
	Status          string
	ModuleId        int64
}

type PowerModule struct {
	Exist           bool
	ModuleName      string
	SerialNumber    string
	ProductRevision string
	Status          string
}

func (m *DefaultMonitor) makeMonitorDeviceSystemStatusTask() MonitorTask {
	return func(ctx context.Context) error {
		err := m.doMonitorDeviceUpdate(ctx)
		return err
	}
}

func formatSystemUpTime(sec int64) string {
	// "0d0h35m13s"
	days := sec / 86400
	sec %= 86400

	hours := sec / 3600
	sec %= 3600

	minutes := sec / 60
	seconds := sec % 60
	return fmt.Sprintf("%dd%dh%dm%ds", days, hours, minutes, seconds)
}

func (m *DefaultMonitor) onDeviceSystemStatusUpdate(ctx context.Context, statuses []*DeviceSystemStatus) {
	if ctx.Err() != nil {
		return
	}

	if len(statuses) == 0 {
		// todo: check if need to send empty list
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Warnf("[onDeviceSystemStatusUpdate] No project ID found in context")
		return
	}

	wsData := make(wscommand.MonitorSystemStatusUpdateDevices, 0)
	for _, status := range statuses {
		var modularInfo *wscommand.DeviceModularInfo
		if status.ModularInfo != nil {
			modularInfo = &wscommand.DeviceModularInfo{
				Ethernet: make(map[int]wscommand.EthernetModule),
				Power:    make(map[int]wscommand.PowerModule),
			}

			for slot, ethModule := range status.ModularInfo.Ethernet {
				modularInfo.Ethernet[slot] = wscommand.EthernetModule{
					Exist:           ethModule.Exist,
					ModuleName:      ethModule.ModuleName,
					SerialNumber:    ethModule.SerialNumber,
					ProductRevision: ethModule.ProductRevision,
					Status:          ethModule.Status,
					ModuleId:        ethModule.ModuleId,
				}
			}

			for slot, powerModule := range status.ModularInfo.Power {
				modularInfo.Power[slot] = wscommand.PowerModule{
					Exist:           powerModule.Exist,
					ModuleName:      powerModule.ModuleName,
					SerialNumber:    powerModule.SerialNumber,
					ProductRevision: powerModule.ProductRevision,
					Status:          powerModule.Status,
				}
			}
		}

		var systemUptime string
		if status.SystemUptime != nil && *status.SystemUptime >= 0 {
			systemUptime = formatSystemUpTime(*status.SystemUptime)
		}

		interfaces := make([]*wscommand.DeviceInterfaceEntry, 0, len(status.Interfaces))
		for _, iface := range status.Interfaces {
			ifaceEntry := &wscommand.DeviceInterfaceEntry{
				InterfaceId:   iface.InterfaceId,
				InterfaceName: iface.InterfaceName,
				Description:   iface.Description,
				MacAddress:    iface.MacAddress,
				// IpAddress:     iface.IpAddress,
				Active: iface.Active,
			}
			interfaces = append(interfaces, ifaceEntry)
		}

		wsData = append(wsData, &wscommand.MonitorSystemStatusUpdateDevice{
			DeviceId:          status.DeviceId,
			DeviceIp:          status.DeviceIp,
			Alias:             status.Alias,
			MacAddress:        status.MacAddress,
			ModelName:         status.ModelName,
			ModularInfo:       modularInfo,
			FirmwareVersion:   status.FirmwareVersion,
			ProductRevision:   status.ProductRevision,
			SerialNumber:      status.SerialNumber,
			DeviceName:        status.DeviceName,
			Role:              status.Role,
			RedundantProtocol: status.RedundantProtocol,
			CPUUsage:          status.CPUUsage,
			MemoryUsage:       status.MemoryUsage,
			SystemUptime:      systemUptime,
			Interfaces:        interfaces,
		})
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandMonitorStatusUpdate.Int64(),
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

func (m *DefaultMonitor) doMonitorDeviceUpdate(ctx context.Context) error {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return fmt.Errorf("project id not found in context")
	}

	if !m.isMonitoring(projectId) {
		logger.Warnf("Monitor is not running, skip sending device system status")
		return fmt.Errorf("Monitor is not running, skip sending device system status")
	}

	collectDeviceSystemStatus := func() []*DeviceSystemStatus {
		// collect current device system status
		currentDevices := m.currentDeviceMap.GetAll()
		result := make([]*DeviceSystemStatus, 0)

		for _, actDevice := range currentDevices {
			mafDevice, mafDeviceExists := m.findMafDeviceByActDeviceID(actDevice.Id)

			status := &DeviceSystemStatus{
				DeviceId:   actDevice.Id,
				DeviceIp:   actDevice.Ipv4.IpAddress,
				Alias:      actDevice.DeviceAlias,
				ModelName:  actDevice.DeviceProperty.ModelName,
				MacAddress: actDevice.MacAddress,
			}

			// actDevice.ModularConfiguration

			if mafDeviceExists {
				if mafDevice.Modules != nil {
					deviceModularInfo := &DeviceModularInfo{}
					if mafDevice.Modules.Ethernet != nil {
						deviceModularInfo.Ethernet = make(map[int]EthernetModule)
						for i, ethModule := range mafDevice.Modules.Ethernet {
							isExists := ethModule != nil

							slotID := i + 1

							if isExists {
								deviceModularInfo.Ethernet[slotID] = EthernetModule{
									Exist:           true,
									ModuleName:      ethModule.ModuleName,
									SerialNumber:    ethModule.SerialNumber,
									ProductRevision: ethModule.ProductRevision,
									Status:          ethModule.Status,
									ModuleId:        int64(ethModule.ModuleID),
								}
							} else {
								deviceModularInfo.Ethernet[slotID] = EthernetModule{
									Exist: false,
								}
							}
						}
					}

					if mafDevice.Modules.Power != nil {
						deviceModularInfo.Power = make(map[int]PowerModule)
						for i, powerModule := range mafDevice.Modules.Power {
							slotID := i + 1

							isExists := powerModule != nil
							if isExists {
								deviceModularInfo.Power[slotID] = PowerModule{
									Exist:           true,
									ModuleName:      powerModule.ModuleName,
									SerialNumber:    powerModule.SerialNumber,
									ProductRevision: powerModule.ProductRevision,
									Status:          powerModule.Status,
								}
							} else {
								deviceModularInfo.Power[slotID] = PowerModule{
									Exist: false,
								}
							}
						}
					}

					status.ModularInfo = deviceModularInfo
				}
				status.FirmwareVersion = mafDevice.FirmwareVersion
				status.ProductRevision = mafDevice.ProductRevision
				status.SerialNumber = mafDevice.SerialNumber
				if mafDevice.DeviceName != nil {
					status.DeviceName = *mafDevice.DeviceName
				}

				if mafDevice.Redundancy != nil && mafDevice.Redundancy.SpanningTree != nil {
					status.Role = mafDevice.Redundancy.SpanningTree.Role
				}

				if mafDevice.Configuration != nil && mafDevice.Configuration.RedundancyProtocols != nil {
					status.RedundantProtocol = make([]string, 0, len(mafDevice.Configuration.RedundancyProtocols))

					for _, protocol := range mafDevice.Configuration.RedundancyProtocols {
						protocolStrLowerCase := strings.ToLower(protocol)
						if protocolStrLowerCase == "rstp" ||
							protocolStrLowerCase == "stprstp" ||
							protocolStrLowerCase == "stp" {
							status.RedundantProtocol = append(status.RedundantProtocol, "STP/RSTP")
							break
						}
					}

				}

				if mafDevice.SystemStatus != nil {
					if mafDevice.SystemStatus.CPULoading != nil {
						var cpuUsageFloat64 float64 = *mafDevice.SystemStatus.CPULoading

						cpuUsageFloat64 = cpuUsageFloat64 * 100.0
						if cpuUsageFloat64 > 100.0 {
							cpuUsageFloat64 = 100.0
						}

						if cpuUsageFloat64 < 0.0 {
							cpuUsageFloat64 = 0.0
						}

						// 目前 MAF 數值只能提供到整數百分比
						status.CPUUsage = fmt.Sprintf("%.0f", cpuUsageFloat64)
					}

					if mafDevice.SystemStatus.MemoryUsage != nil {
						var memoryUsageFloat64 float64 = *mafDevice.SystemStatus.MemoryUsage
						memoryUsageFloat64 = memoryUsageFloat64 * 100.0
						if memoryUsageFloat64 > 100.0 {
							memoryUsageFloat64 = 100.0
						}
						if memoryUsageFloat64 < 0.0 {
							memoryUsageFloat64 = 0.0
						}

						// 目前 MAF 數值只能提供到整數百分比
						status.MemoryUsage = fmt.Sprintf("%.0f", memoryUsageFloat64)
					}

					if mafDevice.SystemStatus.Uptime != nil {
						uptime := *mafDevice.SystemStatus.Uptime
						status.SystemUptime = &uptime
					}
				}

				interfaces := make([]*DeviceInterfaceEntry, 0, len(mafDevice.Interfaces))

				for _, iface := range mafDevice.Interfaces {
					ifaceIDStr := strconv.Itoa(iface.ID)
					ifaceEntry := &DeviceInterfaceEntry{
						InterfaceId:   int64(iface.ID),
						InterfaceName: ifaceIDStr,
						Description:   iface.Description,
						MacAddress:    "",
						// 需要比對是否有插 module，之前討論先設定為 true
						Active: true,
					}

					interfaces = append(interfaces, ifaceEntry)
				}

				status.Interfaces = interfaces
			}

			result = append(result, status)
		}

		return result
	}

	if !m.isMonitoring(projectId) {
		return fmt.Errorf("monitor is not running")
	}

	if ctx.Err() != nil {
		return ctx.Err()
	}

	collectedData := collectDeviceSystemStatus()
	m.onDeviceSystemStatusUpdate(ctx, collectedData)
	return nil
}
