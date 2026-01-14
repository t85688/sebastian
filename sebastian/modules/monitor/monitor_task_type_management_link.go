package monitor

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type ManagementLink struct {
	SourceDeviceId         int64
	SourceDeviceIp         string
	SourceInterfaceId      int64
	DestinationDeviceId    int64
	DestinationDeviceIp    string
	DestinationInterfaceId int64
}

func (m *DefaultMonitor) makeMonitorManagementLinkTask() MonitorTask {
	return func(ctx context.Context) error {
		err := m.doMonitorManagementLink(ctx)
		return err
	}
}

func (m *DefaultMonitor) onMonitorManagementLinkTaskResult(ctx context.Context, managementLinks []*ManagementLink) {
	if ctx.Err() != nil {
		return
	}

	projectId, exists := getProjectId(ctx)
	if !exists {
		logger.Debug("[onMonitorManagementLinkTaskResult] No project ID found in context")
		return
	}

	hasManagementLink := len(managementLinks) > 0
	path := fmt.Sprintf("Projects/%v/Endpoint", projectId)

	var wsResp *wscommand.BaseWsCommandResponse

	if hasManagementLink {
		managementLink := managementLinks[0]

		var mgmtEndpointDeviceId int64 = -1
		var mgmtEndpointInterfaceId int64 = -1

		if managementLink.SourceDeviceIp == localhostIP {
			mgmtEndpointDeviceId = managementLink.DestinationDeviceId
			mgmtEndpointInterfaceId = managementLink.DestinationInterfaceId
		} else if managementLink.DestinationDeviceIp == localhostIP {
			mgmtEndpointDeviceId = managementLink.SourceDeviceId
			mgmtEndpointInterfaceId = managementLink.SourceInterfaceId
		} else {
			logger.Warnf("invalid management link, neither source nor destination device ip is localhost, link: %+v", managementLink)
			return
		}

		wsResp = wscommand.NewBaseWsCommandResponse(
			wscommand.ActWSCommandMonitorEndpointUpdate.Int64(),
			0,
			wscommand.PatchUpdateActionUpdate.String(),
			&wscommand.MonitorManagementEndpoint{
				DeviceId:    mgmtEndpointDeviceId,
				InterfaceId: mgmtEndpointInterfaceId,
			})

		wsResp.Path = path
	}

	if !hasManagementLink {
		wsResp = wscommand.NewBaseWsCommandResponse(
			wscommand.ActWSCommandMonitorEndpointUpdate.Int64(),
			0,
			wscommand.PatchUpdateActionDelete.String(),
			nil)

		wsResp.Path = path
	}

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Error("failed to marshal ws response: ", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)

}

func (m *DefaultMonitor) doMonitorManagementLink(ctx context.Context) error {
	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		return fmt.Errorf("project id not found in context")
	}

	collectManagementLinks := func() []*ManagementLink {
		result := []*ManagementLink{}
		mafManagementLinks := []*netdl.Link{}

		mafLinks := m.mafLinksCache.GetAll()
		for _, mafLink := range mafLinks {
			if mafLink.FromDevice.IP == localhostIP || mafLink.ToDevice.IP == localhostIP {
				mafManagementLinks = append(mafManagementLinks, mafLink)
			}
		}

		if len(mafManagementLinks) == 0 {
			logger.Debug("no management link found")
			return result
		}

		for _, mafLink := range mafManagementLinks {
			isLocalFrom := false

			if mafLink.FromDevice.IP == localhostIP && mafLink.ToDevice.IP == localhostIP {
				logger.Warnf("invalid management link, both source and destination device ip are localhost, link: %+v", mafLink)
				continue
			} else if mafLink.FromDevice.IP == localhostIP {
				isLocalFrom = true
			} else if mafLink.ToDevice.IP == localhostIP {
				isLocalFrom = false
			} else {
				logger.Warnf("invalid management link, neither source nor destination device ip is localhost, link: %+v", mafLink)
				continue
			}

			var peerDevice *netdl.Device
			if isLocalFrom {
				peerDevice = &netdl.Device{
					DeviceBasic: netdl.DeviceBasic{
						IP:  mafLink.ToDevice.IP,
						MAC: mafLink.ToDevice.MAC,
					},
				}
			} else {
				peerDevice = &netdl.Device{
					DeviceBasic: netdl.DeviceBasic{
						IP:  mafLink.FromDevice.IP,
						MAC: mafLink.FromDevice.MAC,
					},
				}
			}

			peerActDevice, peerActDeviceExists := m.findActDeviceInCurrentByMafIP(peerDevice)

			if peerActDeviceExists {

				var localDeviceId int64 = -1
				var localDeviceIp string
				var localInterfaceId int = -1
				var peerDeviceId int64 = -1
				var peerDeviceIp string
				var peerInterfaceId int = -1

				localDeviceIp = localhostIP
				peerDeviceIp = peerActDevice.Ipv4.IpAddress
				peerDeviceId = peerActDevice.Id

				if isLocalFrom {
					peerInterfaceId = mafLink.ToPort
				} else {
					peerInterfaceId = mafLink.FromPort
				}

				managementLink := &ManagementLink{
					SourceDeviceId:         localDeviceId,
					SourceDeviceIp:         localDeviceIp,
					SourceInterfaceId:      int64(localInterfaceId),
					DestinationDeviceId:    peerDeviceId,
					DestinationDeviceIp:    peerDeviceIp,
					DestinationInterfaceId: int64(peerInterfaceId),
				}

				result = append(result, managementLink)
			}
		}

		return result
	}

	if !m.isMonitoring(projectId) {
		logger.Info("monitor is not running, skip monitoring management link")
		return nil
	}

	if ctx.Err() != nil {
		return ctx.Err()
	}

	managementLinks := collectManagementLinks()
	m.onMonitorManagementLinkTaskResult(ctx, managementLinks)

	return nil

}
