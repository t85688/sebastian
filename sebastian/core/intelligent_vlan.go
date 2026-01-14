package core

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
)

type egressPort struct {
	fromDevId  int64
	currDevId  int64
	currPortId int
	nextDevId  int64
	nextPortId int
	linkId     int64
}

func insertPortMap(m map[int64]map[int]int64, devId int64, portId int, linkId int64) {
	ports, ok := m[devId]
	if !ok {
		ports = make(map[int]int64)
		m[devId] = ports
	}
	ports[portId] = linkId
}

func findAllLinksBetweenEdges(edgeSet []int64, topology map[int64][]*domain.Link) (edgePort map[int64]map[int]int64, throughPort map[int64]map[int]int64) {
	edgePort = make(map[int64]map[int]int64)
	throughPort = make(map[int64]map[int]int64)

	queue := [][]egressPort{}
	visited := make(map[int64]int64) // device -> visited
	for _, edge := range edgeSet {
		for _, link := range topology[edge] {
			sourcePort := egressPort{
				fromDevId:  edge,
				currDevId:  int64(link.SourceDeviceId),
				currPortId: link.SourceInterfaceId,
				nextDevId:  int64(link.DestinationDeviceId),
				nextPortId: link.DestinationInterfaceId,
				linkId:     link.Id,
			}
			if int64(link.SourceDeviceId) != edge {
				sourcePort = egressPort{
					fromDevId:  edge,
					currDevId:  int64(link.DestinationDeviceId),
					currPortId: link.DestinationInterfaceId,
					nextDevId:  int64(link.SourceDeviceId),
					nextPortId: link.SourceInterfaceId,
					linkId:     link.Id,
				}
			}
			queue = append(queue, []egressPort{sourcePort})
			visited[edge] = edge
		}
	}

	for len(queue) > 0 {
		curPath := queue[0]
		logger.Infof("findAllLinksBetweenEdges: curPath=%+v", curPath)
		queue = queue[1:]
		curPort := curPath[len(curPath)-1]

		logger.Infof("findAllLinksBetweenEdges: curPort=%+v", curPort)

		// 找到一條從 source 到 target 的路徑
		if visited[curPort.nextDevId] != 0 && visited[curPort.nextDevId] != curPort.fromDevId {
			logger.Infof("findAllLinksBetweenEdges: found path to edge device %d", visited[curPort.nextDevId])
			logger.Infof("findAllLinksBetweenEdges: curPath=%+v", curPath)
			for idx, egressPort := range curPath {
				if idx == 0 {
					insertPortMap(edgePort, egressPort.nextDevId, egressPort.nextPortId, egressPort.linkId)
					insertPortMap(throughPort, egressPort.nextDevId, egressPort.nextPortId, egressPort.linkId)
				} else {
					insertPortMap(throughPort, egressPort.currDevId, egressPort.currPortId, egressPort.linkId)
					insertPortMap(throughPort, egressPort.nextDevId, egressPort.nextPortId, egressPort.linkId)
				}
			}
			continue
		}

		for _, link := range topology[curPort.nextDevId] {
			if link.Id == curPort.linkId {
				continue
			}

			nextPort := egressPort{
				fromDevId:  curPort.fromDevId,
				currDevId:  int64(link.SourceDeviceId),
				currPortId: link.SourceInterfaceId,
				nextDevId:  int64(link.DestinationDeviceId),
				nextPortId: link.DestinationInterfaceId,
				linkId:     link.Id,
			}
			if int64(link.SourceDeviceId) != curPort.nextDevId {
				nextPort = egressPort{
					fromDevId:  curPort.fromDevId,
					currDevId:  int64(link.DestinationDeviceId),
					currPortId: link.DestinationInterfaceId,
					nextDevId:  int64(link.SourceDeviceId),
					nextPortId: link.SourceInterfaceId,
					linkId:     link.Id,
				}
			}

			newPath := append(append([]egressPort(nil), curPath...), nextPort)
			queue = append(queue, newPath)
		}

		visited[curPort.nextDevId] = curPort.fromDevId
	}

	logger.Infof("findAllLinksBetweenEdges: edgePort=%+v", edgePort)
	logger.Infof("findAllLinksBetweenEdges: throughPort=%+v", throughPort)

	return edgePort, throughPort
}

func checkVlanIdUsed(project domain.Project, vlan_id uint16) bool {
	for _, vlan_table := range project.DeviceConfig.VlanTables {
		for _, vlan_static_entry := range vlan_table.VlanStaticEntries {
			if uint16(vlan_static_entry.VlanId) == vlan_id {
				return true
			}
		}
	}
	return false
}

func findUniqueIntelligentVlanId(project domain.Project) (uint16, bool) {
	usedIds := make(map[uint16]bool)
	for _, vlan_table := range project.DeviceConfig.VlanTables {
		for _, vlan_static_entry := range vlan_table.VlanStaticEntries {
			usedIds[uint16(vlan_static_entry.VlanId)] = true
		}
	}
	for id := uint16(project.ProjectSetting.VlanRange.Min); id <= uint16(project.ProjectSetting.VlanRange.Max); id++ {
		if !usedIds[id] {
			return id, true
		}
	}
	return 0, false // 表示沒有可用的 ID
}

func CreateIntelligentVLAN(project_id int64, intelligent_vlan domain.IntelligentVlan) (res statuscode.Response) {
	logger.Infof("CreateIntelligentVLAN: project_id=%d, intelligent_vlan=%+v", project_id, intelligent_vlan)

	project, res := GetProject(project_id, false, false)
	if !res.IsSuccess() {
		logger.Infof("CreateIntelligentVLAN: failed to get project_id=%d, res=%+v", project_id, res)
		return res
	}
	logger.Infof("CreateIntelligentVLAN: fetched vlan_group=%+v", project.TopologySetting.IntelligentVlanGroup)

	// 檢查 EndStationList 至少要 2 個
	if len(intelligent_vlan.EndStationList) < 2 {
		return statuscode.StatusBadRequest("EndStationList must contain at least 2 devices", 0)
	}

	// 檢查 EndStationList 的 device 是否都存在於 project.Devices
	deviceExists := make(map[int64]bool)
	for _, d := range project.Devices {
		deviceExists[d.Id] = true
	}
	for _, edgeId := range intelligent_vlan.EndStationList {
		if !deviceExists[edgeId] {
			return statuscode.StatusBadRequest(fmt.Sprintf("Device ID %d in EndStationList does not exist in the project", edgeId), 0)
		}
	}

	// 分配 VLAN ID
	if intelligent_vlan.StreamType == domain.StreamTypeEnum_Untagged {
		vlan_id, found := findUniqueIntelligentVlanId(project)
		logger.Infof("CreateIntelligentVLAN: found unique VLAN ID %d for untagged intelligent VLAN", intelligent_vlan.VlanId)
		if !found {
			return statuscode.StatusBadRequest("No available VLAN ID", 0)
		}
		intelligent_vlan.VlanId = vlan_id
	} else if intelligent_vlan.StreamType == domain.StreamTypeEnum_Tagged {
		// 檢查 VLAN ID 範圍
		if intelligent_vlan.VlanId < uint16(project.ProjectSetting.VlanRange.Min) || intelligent_vlan.VlanId > uint16(project.ProjectSetting.VlanRange.Max) {
			return statuscode.StatusBadRequest(fmt.Sprintf("VLAN ID must be between %d and %d for tagged stream type", project.ProjectSetting.VlanRange.Min, project.ProjectSetting.VlanRange.Max), 0)
		}
		// 檢查 VLAN ID 是否已被使用
		if checkVlanIdUsed(project, intelligent_vlan.VlanId) {
			return statuscode.StatusBadRequest(fmt.Sprintf("VLAN ID %d is already in use", intelligent_vlan.VlanId), 0)
		}
	} else {
		return statuscode.StatusBadRequest("Unknown StreamType", 0)
	}

	vlan_id := intelligent_vlan.VlanId
	edgeSet := intelligent_vlan.EndStationList

	// deviceId -> interfaceId -> 是否 Active && Used
	ifaceUsable := make(map[int64]map[int]bool)
	for _, d := range project.Devices {
		ifaceUsable[d.Id] = make(map[int]bool)
		for _, ifc := range d.Interfaces {
			if ifc.Active && ifc.Used {
				ifaceUsable[d.Id][ifc.InterfaceId] = true
			}
		}
	}

	adj := make(map[int64][]*domain.Link)
	for i := range project.Links {
		l := &project.Links[i]

		srcDev := int64(l.SourceDeviceId)
		dstDev := int64(l.DestinationDeviceId)

		// 兩端介面都要 Active+Used 才算在拓樸裡
		if !ifaceUsable[srcDev][l.SourceInterfaceId] {
			continue
		}
		if !ifaceUsable[dstDev][l.DestinationInterfaceId] {
			continue
		}

		// undirected: 兩邊都掛這條 link
		adj[srcDev] = append(adj[srcDev], l)
		adj[dstDev] = append(adj[dstDev], l)
	}

	edgePort, throughPort := findAllLinksBetweenEdges(edgeSet, adj) // edgePort, throughPort: device_id -> port_id -> link_id

	// 把 set 轉成 map[deviceId][]portId 回傳 ----------
	name := fmt.Sprintf("v%d", vlan_id)
	if intelligent_vlan.StreamType == domain.StreamTypeEnum_Untagged {
		for dev, portsSet := range edgePort {
			vlan_table := project.DeviceConfig.VlanTables[dev]
			for port := range portsSet {
				port_vlan_entry := domain.PortVlanEntry{
					PortId: int64(port),
					PVID:   int(vlan_id),
				}
				vlan_table.PortVlanEntries = append(vlan_table.PortVlanEntries, port_vlan_entry)
			}
			project.DeviceConfig.VlanTables[dev] = vlan_table
		}
	}

	for dev, portsSet := range throughPort {
		vlan_table := project.DeviceConfig.VlanTables[dev]

		egressPorts := []int{}
		for port := range portsSet {
			egressPorts = append(egressPorts, port)

			// 設定為 Access Port
			vlan_port_type_entry := domain.VlanPortTypeEntry{
				PortId:       int64(port),
				VlanPortType: "Access",
			}
			vlan_table.VlanPortTypeEntries = append(vlan_table.VlanPortTypeEntries, vlan_port_type_entry)
		}

		untaggedPorts := []int{}
		for port := range edgePort[dev] {
			untaggedPorts = append(untaggedPorts, port)
		}

		vlan_static_entry := domain.VlanStaticEntry{
			VlanId:        int(vlan_id),
			Name:          name,
			TeMstid:       false,
			EgressPorts:   egressPorts,
			UntaggedPorts: untaggedPorts,
		}
		vlan_table.VlanStaticEntries = append(vlan_table.VlanStaticEntries, vlan_static_entry)

		project.DeviceConfig.VlanTables[dev] = vlan_table
	}

	logger.Infof("CreateIntelligentVLAN: project_id=%d, vlanTables=%+v", project_id, project.DeviceConfig.VlanTables)

	project.TopologySetting.IntelligentVlanGroup = append(project.TopologySetting.IntelligentVlanGroup, intelligent_vlan)
	logger.Infof("CreateIntelligentVLAN: updated vlan_group=%+v", project.TopologySetting.IntelligentVlanGroup)

	res = UpdateFullProject(&project, false)
	if !res.IsSuccess() {
		return res
	}

	return res
}

func GetIntelligentVLAN(project_id int64) (res statuscode.Response) {
	project, res := GetProject(project_id, false, false)
	if !res.IsSuccess() {
		return res
	}
	return statuscode.StatusOK(project.TopologySetting.IntelligentVlanGroup)
}

func UpdateIntelligentVLAN(project_id int64, vlan_id uint16, intelligent_vlan domain.IntelligentVlan) (res statuscode.Response) {
	res = DeleteIntelligentVLAN(project_id, vlan_id)
	if !res.IsSuccess() {
		return res
	}

	res = CreateIntelligentVLAN(project_id, intelligent_vlan)
	if !res.IsSuccess() {
		return res
	}

	return res
}

func DeleteIntelligentVLAN(project_id int64, vlan_id uint16) (res statuscode.Response) {
	project, res := GetProject(project_id, false, false)
	if !res.IsSuccess() {
		return res
	}

	found := false
	for idx, intelligent_vlan := range project.TopologySetting.IntelligentVlanGroup {
		if intelligent_vlan.VlanId == vlan_id {
			// Remove from slice
			project.TopologySetting.IntelligentVlanGroup = append(
				project.TopologySetting.IntelligentVlanGroup[:idx],
				project.TopologySetting.IntelligentVlanGroup[idx+1:]...,
			)
			found = true
			break
		}
	}

	if !found {
		return statuscode.StatusBadRequest(fmt.Sprintf("Intelligent VLAN (vlan id=%v) not found", vlan_id), 0)
	}

	for dev_id, vlan_table := range project.DeviceConfig.VlanTables {
		for idx := range vlan_table.PortVlanEntries {
			if vlan_table.PortVlanEntries[idx].PVID == int(vlan_id) {
				vlan_table.PortVlanEntries[idx].PVID = vlan_table.ManagementVlan // ✔️ 修改 slice 中的 struct
			}
		}

		for idx, vlan_static_entry := range vlan_table.VlanStaticEntries {
			if vlan_static_entry.VlanId == int(vlan_id) {
				// Remove from slice
				vlan_table.VlanStaticEntries = append(
					vlan_table.VlanStaticEntries[:idx],
					vlan_table.VlanStaticEntries[idx+1:]...,
				)
				break
			}
		}

		project.DeviceConfig.VlanTables[dev_id] = vlan_table
	}

	res = UpdateFullProject(&project, false)
	if !res.IsSuccess() {
		return res
	}

	return res
}
