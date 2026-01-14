package configmapper

import (
	"fmt"
	"strconv"
	"strings"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func MappingDeviceConfigUnicastStaticForwarding(unicastStaticForwarding *netdl.UnicastStaticForwardingStruct, actDeviceID int64) (*domain.StaticForwardTable, error) {
	staticEntries := []domain.StaticForwardEntry{}

	for _, entry := range unicastStaticForwarding.UnicastStaticForwardingSetting {
		egressPortInts := []int{}
		egressPortInt := entry.EgressPortId
		egressPortInts = append(egressPortInts, egressPortInt)
		staticEntries = append(staticEntries, domain.StaticForwardEntry{
			VlanId:      entry.VlanId,
			MAC:         entry.MacAddress,
			EgressPorts: egressPortInts, // entry.EgressPortIds,
			Dot1qStatus: 3,              // missing field
		})
	}

	result := &domain.StaticForwardTable{
		DeviceId:             actDeviceID,
		StaticForwardEntries: staticEntries,
	}

	return result, nil
}

func MappingDeviceConfigMgmtInterfaceSetting(mgmtInterfaceSetting *netdl.MgmtInterfaceSetting, actDeviceID int64) (*domain.ManagementInterfaceTable, error) {
	snmpEnabledString := "Disabled"
	if mgmtInterfaceSetting.Snmp != nil && mgmtInterfaceSetting.Snmp.Enable != nil && *mgmtInterfaceSetting.Snmp.Enable {
		snmpEnabledString = "Enabled"
	}

	sshPort := 22
	snmpPort := 161
	httpPort := 80
	httpsPort := 443
	telnetPort := 23
	maxTerminalSession := 0
	maxHttpSession := 0
	snmpProtocol := "UDP"

	if mgmtInterfaceSetting.Ssh != nil && mgmtInterfaceSetting.Ssh.Port != nil {
		sshPort = *mgmtInterfaceSetting.Ssh.Port
	}

	if mgmtInterfaceSetting.Snmp != nil && mgmtInterfaceSetting.Snmp.Port != nil {
		snmpPort = *mgmtInterfaceSetting.Snmp.Port
	}

	if mgmtInterfaceSetting.Http != nil && mgmtInterfaceSetting.Http.Port != nil {
		httpPort = *mgmtInterfaceSetting.Http.Port
	}

	if mgmtInterfaceSetting.Https != nil && mgmtInterfaceSetting.Https.Port != nil {
		httpsPort = *mgmtInterfaceSetting.Https.Port
	}

	if mgmtInterfaceSetting.Telnet != nil && mgmtInterfaceSetting.Telnet.Port != nil {
		telnetPort = *mgmtInterfaceSetting.Telnet.Port
	}

	if mgmtInterfaceSetting.MaxLoginTelnetSsh != nil {
		maxTerminalSession = *mgmtInterfaceSetting.MaxLoginTelnetSsh
	}

	if mgmtInterfaceSetting.MaxLoginHttpHttps != nil {
		maxHttpSession = *mgmtInterfaceSetting.MaxLoginHttpHttps
	}

	if mgmtInterfaceSetting.Snmp != nil && mgmtInterfaceSetting.Snmp.Protocol != nil {
		if mappedProtocol, ok := MapMafTransportProtocolToActTransportProtocol(*mgmtInterfaceSetting.Snmp.Protocol); ok {
			snmpProtocol = mappedProtocol
		}
	}

	result := &domain.ManagementInterfaceTable{
		DeviceId: actDeviceID,
		EncryptedMoxaService: struct {
			Enable bool `json:"Enable"`
		}{
			Enable: mgmtInterfaceSetting.MoxaService != nil && mgmtInterfaceSetting.MoxaService.Enable != nil && *mgmtInterfaceSetting.MoxaService.Enable,
		},
		HttpMaxLoginSessions: maxHttpSession,
		HttpService: struct {
			Enable bool `json:"Enable"`
			Port   int  `json:"Port"`
		}{
			Enable: mgmtInterfaceSetting.Http != nil && mgmtInterfaceSetting.Http.Enable != nil && *mgmtInterfaceSetting.Http.Enable,
			Port:   httpPort,
		},
		HttpsService: struct {
			Enable bool `json:"Enable"`
			Port   int  `json:"Port"`
		}{
			Enable: mgmtInterfaceSetting.Https != nil && mgmtInterfaceSetting.Https.Enable != nil && *mgmtInterfaceSetting.Https.Enable,
			Port:   httpsPort,
		},
		SSHService: struct {
			Enable bool `json:"Enable"`
			Port   int  `json:"Port"`
		}{
			Enable: mgmtInterfaceSetting.Ssh != nil && mgmtInterfaceSetting.Ssh.Enable != nil && *mgmtInterfaceSetting.Ssh.Enable,
			Port:   sshPort,
		},
		SnmpService: struct {
			Mode                   string `json:"Mode"`
			Port                   int    `json:"Port"`
			TransportLayerProtocol string `json:"TransportLayerProtocol"`
		}{
			Mode:                   snmpEnabledString,
			Port:                   snmpPort,
			TransportLayerProtocol: snmpProtocol,
		},
		TelnetService: struct {
			Enable bool `json:"Enable"`
			Port   int  `json:"Port"`
		}{
			Enable: mgmtInterfaceSetting.Telnet != nil && mgmtInterfaceSetting.Telnet.Enable != nil && *mgmtInterfaceSetting.Telnet.Enable,
			Port:   telnetPort,
		},
		TerminalMaxLoginSessions: maxTerminalSession,
	}

	return result, nil
}

func MappingDeviceConfigTimeSyncSetting(timsSyncSetting *netdl.TimeSyncSetting, actDeviceID int64) (*domain.TimeSyncTable, error) {
	// Map the time sync profile
	var profile domain.TimeSyncProfile = domain.TimeSyncProfileIEEE1588_2008
	if mappedProfile, ok := MapTimeSyncProfile(timsSyncSetting.Profile); ok {
		profile = mappedProfile
	} else {
		return nil, fmt.Errorf("Unsupported time sync profile: %s for device %v", timsSyncSetting.Profile, actDeviceID)
	}

	timeSyncTable := &domain.TimeSyncTable{
		DeviceId: actDeviceID,
		Enabled:  timsSyncSetting.Enable,
		Profile:  profile.String(),
	}

	if timsSyncSetting.Ptp1588Default2008 != nil {
		ieee1588 := domain.NewIEEE1588_2008_Profile()

		if mappedClockType, ok := Map1588ClockType(timsSyncSetting.Ptp1588Default2008.ClockType); ok {
			ieee1588.ClockType = mappedClockType.String()
		}

		if mappedDelayMech, ok := Map1588DelayMechanism(timsSyncSetting.Ptp1588Default2008.DelayMechanism); ok {
			ieee1588.DelayMechanism = mappedDelayMech.String()
		}

		if mappedTransportType, ok := Map1588TransportMode(timsSyncSetting.Ptp1588Default2008.TransportMode); ok {
			ieee1588.TransportType = mappedTransportType.String()
		}

		if mappedClockMode, ok := Map1588ClockMode(timsSyncSetting.Ptp1588Default2008.ClockMode); ok {
			ieee1588.ClockMode = mappedClockMode.String()
		}

		ieee1588.AccuracyAlert = timsSyncSetting.Ptp1588Default2008.AccuracyAlert
		ieee1588.Priority1 = timsSyncSetting.Ptp1588Default2008.Priority1
		ieee1588.Priority2 = timsSyncSetting.Ptp1588Default2008.Priority2
		ieee1588.DomainNumber = timsSyncSetting.Ptp1588Default2008.DomainNumber

		if timsSyncSetting.Ptp1588Default2008.MaximumStepsRemoved != nil {
			ieee1588.MaximumStepsRemoved = *timsSyncSetting.Ptp1588Default2008.MaximumStepsRemoved
		}

		for _, portEntry := range timsSyncSetting.TimeSyncPortEntries {
			actPortEntry := domain.NewIEEE1588_PortEntry()
			actPortEntry.AnnounceInterval = portEntry.AnnounceInterval
			actPortEntry.AnnounceReceiptTimeout = portEntry.AnnounceReceiptTimeout
			actPortEntry.Enable = portEntry.Enable
			actPortEntry.PortId = portEntry.PortID
			actPortEntry.SyncInterval = portEntry.SyncInterval

			if portEntry.PdelayRequestInterval != nil {
				actPortEntry.PdelayReqInterval = *portEntry.PdelayRequestInterval
			}
			if portEntry.DelayRequestInterval != nil {
				actPortEntry.DelayReqInterval = *portEntry.DelayRequestInterval
			}

			ieee1588.PortEntries = append(ieee1588.PortEntries, *actPortEntry)
		}

		timeSyncTable.IEEE1588_2008 = ieee1588
	}

	if timsSyncSetting.As2011Setting != nil {
		ieee8021as := domain.NewIEEE8021AS_2011_Profile()
		ieee8021as.AccuracyAlert = timsSyncSetting.As2011Setting.AccuracyAlert
		ieee8021as.Priority1 = timsSyncSetting.As2011Setting.Priority1
		ieee8021as.Priority2 = timsSyncSetting.As2011Setting.Priority2

		for _, portEntry := range timsSyncSetting.TimeSyncPortEntries {
			actPortEntry := domain.NewIEEE8021AS_PortEntry()
			actPortEntry.AnnounceInterval = portEntry.AnnounceInterval
			actPortEntry.AnnounceReceiptTimeout = portEntry.AnnounceReceiptTimeout
			actPortEntry.Enable = portEntry.Enable
			actPortEntry.PortId = portEntry.PortID
			actPortEntry.SyncInterval = portEntry.SyncInterval

			if portEntry.SyncReceiptTimeout != nil {
				actPortEntry.SyncReceiptTimeout = *portEntry.SyncReceiptTimeout
			}

			if portEntry.NeighborPropagationDelayThreshold != nil {
				actPortEntry.NeighborPropDelayThresh = *portEntry.NeighborPropagationDelayThreshold
			}

			if portEntry.PdelayRequestInterval != nil {
				actPortEntry.PdelayReqInterval = *portEntry.PdelayRequestInterval
			}

			ieee8021as.PortEntries = append(ieee8021as.PortEntries, *actPortEntry)
		}
		timeSyncTable.IEEE8021AS_2011 = ieee8021as
	}

	return timeSyncTable, nil
}

func appendUnique(m map[int64][]int, key int64, val int) {
	for _, v := range m[key] {
		if v == val {
			return // exists
		}
	}
	m[key] = append(m[key], val)
}

func MappingDeviceConfigVLANSetting(vlanSetting *netdl.VlanSetting, actDeviceID int64) (*domain.VlanTable, error) {
	portVLANEntries := []domain.PortVlanEntry{}
	portTypeEntries := []domain.VlanPortTypeEntry{}
	vlanStaticEntries := []domain.VlanStaticEntry{}

	vlanEgressPortsMap := make(map[int64][]int)   // vlan -> EgressPorts
	vlanUntaggedPortsMap := make(map[int64][]int) //  vlan -> UntaggedPorts

	var vlanModeMap = map[netdl.VlanModeEnum]string{
		netdl.VlanModeAccess: "Access",
		netdl.VlanModeTrunk:  "Trunk",
		netdl.VlanModeHybrid: "Hybrid",
		netdl.VlanModeNone:   "None",
	}

	for _, portEntry := range vlanSetting.VlanPortEntries {
		if strings.HasPrefix(portEntry.AliasName, "po") {
			logger.Debug("Skip LAG port %s in VLAN mapping, ActDeviceID: %d", portEntry.AliasName, actDeviceID)
			continue
		}

		portIdInt := portEntry.PortId
		portVLANEntries = append(portVLANEntries, domain.PortVlanEntry{
			PortId: int64(portIdInt),
			PVID:   int(portEntry.Pvid),
			// [NonTSN,TSNSystem,TSNUser]
			VlanPriority: "NonTSN",
		})

		for _, vlan := range portEntry.TaggedVlans {
			appendUnique(vlanEgressPortsMap, vlan, portIdInt)
		}

		for _, vlan := range portEntry.UntaggedVlans {
			appendUnique(vlanEgressPortsMap, vlan, portIdInt)
			appendUnique(vlanUntaggedPortsMap, vlan, portIdInt)
		}

		portTypeEntries = append(portTypeEntries, domain.VlanPortTypeEntry{
			PortId:       int64(portIdInt),
			VlanPortType: vlanModeMap[portEntry.Mode],
			// [NonTSN,TSNSystem,TSNUser]
			VlanPriority: "NonTSN",
		})

	}

	/*
				type VlanStaticEntry struct {
			VlanId               int    `json:"VlanId"`
			Name                 string `json:"Name"`
			RowStatus            int    `json:"RowStatus"`
			TeMstid              bool   `json:"TeMstid"`
			EgressPorts          []int  `json:"EgressPorts"`
			UntaggedPorts        []int  `json:"UntaggedPorts"`
			ForbiddenEgressPorts []int  `json:"ForbiddenEgressPorts"`
			VlanPriority         string `json:"VlanPriority"`
		}

		type VlanEntry struct {
			VlanId      int64    `json:"vlanId"`
			VlanName    string   `json:"vlanName"`
			Temstid     *bool    `json:"temstid,omitempty"`
			MemberPorts []string `json:"memberPort"`
		}

		type VlanPortTypeEntry struct {
			PortId       int64  `json:"PortId"`
			VlanPortType string `json:"VlanPortType"`
			VlanPriority string `json:"VlanPriority"`
		}
	*/
	for _, vlanEntry := range vlanSetting.VlanEntries {
		vlanId := int(vlanEntry.VlanId)
		vlanStaticEntries = append(vlanStaticEntries, domain.VlanStaticEntry{
			VlanId:        vlanId,
			Name:          vlanEntry.VlanName,
			TeMstid:       vlanEntry.Temstid != nil && *vlanEntry.Temstid,
			EgressPorts:   vlanEgressPortsMap[int64(vlanId)],
			UntaggedPorts: vlanUntaggedPortsMap[int64(vlanId)],
			// [NonTSN,TSNSystem,TSNUser]
			VlanPriority: "NonTSN",
		})
	}

	mgmtVlan := -1
	if vlanSetting.MgmtVlan != nil {
		mgmtVlan = int(*vlanSetting.MgmtVlan)
	}

	result := &domain.VlanTable{
		DeviceId:            actDeviceID,
		ManagementVlan:      mgmtVlan,
		VlanPortTypeEntries: portTypeEntries,
		PortVlanEntries:     portVLANEntries,
		VlanStaticEntries:   vlanStaticEntries,
	}

	return result, nil
}

func MappingDeviceConfigRSTPSetting(rstpSetting *netdl.RedundancySpanningTreeSetting, actDeviceID int64) (*domain.RstpTable, error) {
	errorRecoverTime := 0
	if rstpSetting.ErrorRecoveryTime != nil {
		errorRecoverTime = int(*rstpSetting.ErrorRecoveryTime)
	}

	var portEdgeMap = map[string]string{ // key: NetDL, value: Product domain
		"auto": "Auto",
		"yes":  "Yes",
		"no":   "No",
	}

	var linkTypeMap = map[string]string{ // key: NetDL, value: Product domain
		"pointToPoint": "Point-to-point",
		"shared":       "Shared",
		"auto":         "Auto",
	}

	rstpVersion := domain.SpanningTreeVersionNotSupport

	if rstpSetting.StpMode != "stpRstp" {
		err := fmt.Errorf("Unsupported STP mode: %s for device %v", rstpSetting.StpMode, actDeviceID)
		return nil, err
	}
	// SpanningTreeVersionSTP
	switch rstpSetting.Compatibility {
	case netdl.SpanningTreeVersionSTP:
		rstpVersion = domain.SpanningTreeVersionSTP
	case netdl.SpanningTreeVersionRSTP:
		rstpVersion = domain.SpanningTreeVersionRSTP
	default:
		err := fmt.Errorf("Unsupported STP compatibility: %s for device %v", rstpSetting.Compatibility, actDeviceID)
		return nil, err
	}

	rstpPortEntries := make([]domain.RstpPortEntry, 0, len(rstpSetting.StpRstpPortEntries))

	for _, portEntry := range rstpSetting.StpRstpPortEntries {
		// workaround: skip LAG ports
		if strings.HasPrefix(portEntry.AliasName, "po") {
			logger.Debug("Skip LAG port %s in RSTP mapping, ActDeviceID: %d", portEntry.AliasName, actDeviceID)
			continue
		}

		mafPortId := portEntry.PortId
		rstpPortEntries = append(rstpPortEntries, domain.RstpPortEntry{
			PortId:       int64(mafPortId),
			PortPriority: int(portEntry.Priority),
			PathCost:     int(portEntry.PathCost),
			RstpEnable:   portEntry.Enable != nil && *portEntry.Enable,
			BpduFilter:   portEntry.BpduFilter != nil && *portEntry.BpduFilter,
			BpduGuard:    portEntry.BpduGuard != nil && *portEntry.BpduGuard,
			LoopGuard:    portEntry.LoopGuard != nil && *portEntry.LoopGuard,
			RootGuard:    portEntry.RootGuard != nil && *portEntry.RootGuard,
			Edge:         portEdgeMap[portEntry.Edge],
			LinkType:     linkTypeMap[portEntry.LinkType],
		})
	}

	result := &domain.RstpTable{
		DeviceId:              actDeviceID,
		Active:                rstpSetting.Enable,
		ForwardDelay:          int(rstpSetting.ForwardDelayTime),
		HelloTime:             int(rstpSetting.HelloTime),
		MaxAge:                int(rstpSetting.MaxAge),
		Priority:              int(rstpSetting.BridgePriority),
		RstpConfigRevert:      rstpSetting.Revert != nil && *rstpSetting.Revert,
		RstpConfigSwift:       rstpSetting.Swift != nil && *rstpSetting.Swift,
		RstpErrorRecoveryTime: errorRecoverTime,
		RstpPortEntries:       rstpPortEntries,
		SpanningTreeVersion:   rstpVersion.String(),
	}

	return result, nil
}

func MappingDeviceConfigLoopProtectionSetting(loopProtectionSetting *netdl.LoopProtectionSetting, actDeviceID int64) *domain.LoopProtection {
	result := &domain.LoopProtection{
		DeviceId:              actDeviceID,
		NetworkLoopProtection: loopProtectionSetting.Enable,
		DetectInterval:        int(loopProtectionSetting.DetectInterval),
	}

	return result
}

func MappingDeviceConfigPortSetting(input []netdl.PortSetting, actDeviceID int64) *domain.PortSettingTable {
	result := &domain.PortSettingTable{
		DeviceId:           actDeviceID,
		PortSettingEntries: make([]domain.PortSettingEntry, 0, len(input)),
	}

	for _, port := range input {
		if strings.HasPrefix(port.PortName, "po") {
			logger.Debug("Skip LAG port %s in Port Setting mapping, ActDeviceID: %d", port.PortName, actDeviceID)
			continue
		}

		result.PortSettingEntries = append(result.PortSettingEntries, domain.PortSettingEntry{
			PortId:      int64(port.PortId),
			AdminStatus: port.Enable,
		})
	}

	return result
}

func MappingDeviceConfigMulticastStaticForwarding(actDeviceID int64, input *netdl.MulticastStaticForwardingStruct) (*domain.StaticForwardTable, error) {
	staticEntries := []domain.StaticForwardEntry{}

	for _, entry := range input.MulticastStaticForwardingSetting {
		forbiddenEgressPorts := []int{}

		if entry.ForbiddenPortIds != nil {
			forbiddenEgressPorts = *entry.ForbiddenPortIds
		}

		staticEntries = append(staticEntries, domain.StaticForwardEntry{
			VlanId:               entry.VlanId,
			MAC:                  entry.MacAddress,
			EgressPorts:          entry.EgressPortIds,
			Dot1qStatus:          3,                    // missing field
			ForbiddenEgressPorts: forbiddenEgressPorts, // entry.ForbiddenPortIds,
		})
	}

	result := &domain.StaticForwardTable{
		DeviceId:             actDeviceID,
		StaticForwardEntries: staticEntries,
	}

	return result, nil
}

func MappingTimeSettingTables(
	actDeviceID int64,
	inputSystemTime *netdl.SystemTimeConfig,
	inputDayLightSaving *netdl.SetSystemDaylightSavingInput) (*domain.TimeSettingTable, error) {

	var clockSourceMap = map[string]string{ // key: NetDL, value: Product domain
		"local": "Local",
		"sntp":  "SNTP",
		"ntp":   "NTP",
		"ptp":   "PTP",
	}

	timeSetting := &domain.TimeSettingTable{
		DeviceId:           actDeviceID,
		ClockSource:        clockSourceMap[inputSystemTime.ClockSource],
		DaylightSavingTime: inputDayLightSaving.Enable,
		NTPTimeServer1:     "",
		NTPTimeServer2:     "",
		Offset:             inputDayLightSaving.Offset,
		SNTPTimeServer1:    "",
		SNTPTimeServer2:    "",
		TimeZone:           inputDayLightSaving.TimeZone,
	}

	if inputSystemTime.ClockSource == "ntp" {
		timeSetting.NTPTimeServer1 = inputSystemTime.FirstTimeServer
		timeSetting.NTPTimeServer2 = inputSystemTime.SecondTimeServer
	} else if inputSystemTime.ClockSource == "sntp" {
		timeSetting.SNTPTimeServer1 = inputSystemTime.FirstTimeServer
		timeSetting.SNTPTimeServer2 = inputSystemTime.SecondTimeServer
	}

	if inputDayLightSaving.RTime != nil {
		timeSetting.Start = domain.TransitionTime{
			Day:    inputDayLightSaving.RTime.Start.Day,
			Hour:   inputDayLightSaving.RTime.Start.Hour,
			Minute: inputDayLightSaving.RTime.Start.Minute,
			Month:  inputDayLightSaving.RTime.Start.Month,
			Week:   inputDayLightSaving.RTime.Start.Week,
		}

		timeSetting.End = domain.TransitionTime{
			Day:    inputDayLightSaving.RTime.End.Day,
			Hour:   inputDayLightSaving.RTime.End.Hour,
			Minute: inputDayLightSaving.RTime.End.Minute,
			Month:  inputDayLightSaving.RTime.End.Month,
			Week:   inputDayLightSaving.RTime.End.Week,
		}
	} else if inputDayLightSaving.YTime != nil {
		startDate := time.Date(
			inputDayLightSaving.YTime.Start.Year,
			time.Month(inputDayLightSaving.YTime.Start.Month),
			inputDayLightSaving.YTime.Start.Date,
			inputDayLightSaving.YTime.Start.Hour,
			inputDayLightSaving.YTime.Start.Minute,
			0, 0, time.UTC,
		)

		startWeek := int((startDate.Day()-1)/7) + 1
		if startWeek > 5 {
			startWeek = 5
		}

		if startWeek < 1 {
			startWeek = 1
		}

		startWeekday := int(startDate.Weekday())
		if startWeekday <= 0 {
			startWeekday = 7
		}

		timeSetting.Start = domain.TransitionTime{
			Day:    startWeekday,
			Hour:   inputDayLightSaving.YTime.Start.Hour,
			Minute: inputDayLightSaving.YTime.Start.Minute,
			Month:  inputDayLightSaving.YTime.Start.Month,
			Week:   startWeek,
		}

		endDate := time.Date(
			inputDayLightSaving.YTime.End.Year,
			time.Month(inputDayLightSaving.YTime.End.Month),
			inputDayLightSaving.YTime.End.Date,
			inputDayLightSaving.YTime.End.Hour,
			inputDayLightSaving.YTime.End.Minute,
			0, 0, time.UTC,
		)

		endWeek := int((endDate.Day()-1)/7) + 1
		if endWeek > 5 {
			endWeek = 5
		}

		if endWeek < 1 {
			endWeek = 1
		}

		endWeekday := int(endDate.Weekday())
		if endWeekday <= 0 {
			endWeekday = 7
		}

		timeSetting.End = domain.TransitionTime{
			Day:    endWeekday,
			Hour:   inputDayLightSaving.YTime.End.Hour,
			Minute: inputDayLightSaving.YTime.End.Minute,
			Month:  inputDayLightSaving.YTime.End.Month,
			Week:   endWeek,
		}
	}

	return timeSetting, nil
}

func MappingNetworkSettingTables(actDeviceID int64, input []netdl.EthInterfaceSettings) (*domain.NetworkSettingTable, error) {
	// Pick the first interface that has a valid IPv4 address; fallback to the first item
	var picked *netdl.EthInterfaceSettings
	for i := range input {
		if input[i].Ipv4NSettings.IpAddress != "" {
			picked = &input[i]
			break
		}
	}
	if picked == nil && len(input) > 0 {
		picked = &input[0]
	}

	if picked == nil {
		// No interfaces provided; return an empty table with DeviceId set
		return &domain.NetworkSettingTable{DeviceId: actDeviceID}, nil
	}

	networkSettingMode := MapNetworkSettingMode(picked.Ipv4NSettings.Mode)
	if networkSettingMode == domain.NetworkSettingModeUnknown {
		return nil, fmt.Errorf("invalid maf network setting mode: %s for device %d", picked.Ipv4NSettings.Mode, actDeviceID)
	}

	networkSetting := domain.NetworkSettingTable{
		DeviceId:           actDeviceID,
		IpAddress:          picked.Ipv4NSettings.IpAddress,
		SubnetMask:         picked.Ipv4NSettings.Netmask,
		Gateway:            picked.Ipv4NSettings.Gateway,
		DNS1:               picked.Ipv4NSettings.Dns1,
		DNS2:               picked.Ipv4NSettings.Dns2,
		NetworkSettingMode: networkSettingMode.String(),
	}

	return &networkSetting, nil
}

func MappingSnmpTrapSettingTables(actDeviceID int64, input []netdl.SNMPTrapSetting) (*domain.SnmpTrapSettingTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	var modeMap = map[string]string{ // key: NetDL, value: Product domain
		"Trap V1":    "TrapV1",
		"Trap V2c":   "TrapV2c",
		"Inform V2c": "InformV2c",
		"Trap V3":    "TrapV3",
	}

	result := &domain.SnmpTrapSettingTable{
		DeviceId: actDeviceID,
		HostList: []domain.SnmpTrapSettingEntry{},
	}

	for _, trap := range input {
		result.HostList = append(result.HostList, domain.SnmpTrapSettingEntry{
			HostName:      trap.HostName,
			Mode:          modeMap[trap.Mode],
			TrapCommunity: trap.V1v2cCommunity,
		})
	}

	return result, nil
}

func MappingPcpSettingTables(actDeviceID int64, input *netdl.PcpSetting, portIdMap map[int]bool) (*domain.PortDefaultPCPTable, error) {

	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	result := &domain.PortDefaultPCPTable{

		DeviceId:               actDeviceID,
		DefaultPriorityEntries: []domain.DefaultPriorityEntry{},
	}

	for _, ent := range input.PriorityCodePointSetting {
		portIdInt := ent.PortID
		if portIdMap != nil && !portIdMap[portIdInt] {
			continue
		}

		result.DefaultPriorityEntries = append(result.DefaultPriorityEntries, domain.DefaultPriorityEntry{
			PortId:     int64(portIdInt),
			DefaultPCP: int(ent.PriorityCodePoint),
		})
	}

	return result, nil
}

func MappingStreamPriorityEgressTable(actDeviceID int64, input *netdl.PerStreamPrioritySetting) (*domain.StreamPriorityEgressTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	return nil, fmt.Errorf("stream Priority Egress Table mapping not supportted yet")

	// result := &domain.StreamPriorityEgressTable{
	// 	DeviceId:          actDeviceID,
	// 	StadConfigEntries: []domain.StadConfigEntry{},
	// }

	// return result, nil
}

func MappingStreamPriorityIngressTables(actDeviceID int64, input *netdl.PerStreamPrioritySetting) (*domain.StreamPriorityIngressTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}
	// Build entries grouped by port. StadPortEntries structure is unspecified in domain (interface{}), keep empty.
	result := &domain.StreamPriorityIngressTable{
		DeviceId:                 actDeviceID,
		InterfaceStadPortEntries: []domain.InterfaceStadPortEntry{},
	}

	portMap := make(map[int64][]domain.StadPortEntry)
	for _, streamPriority := range input.PerStreamPrioritySetting {
		portEntry := domain.NewStadPortEntry()
		logger.Info(fmt.Sprintf("PERSTREAM streamPriority: %+v\n", streamPriority))

		// ingress index: set per port, from 0 and counting up for each occurrence
		portEntry.PortId = int64(streamPriority.PortId)
		portEntry.IngressIndex = len(portMap[portEntry.PortId])
		portEntry.VlanId = streamPriority.VlanId
		portEntry.VlanPcp = streamPriority.PriorityCodePoint

		switch streamPriority.Type {
		case netdl.PSPTypeInactive:
			portEntry.Type = []string{domain.StreamPriorityTypeInactive.String()}
		case netdl.PSPTypeL2:
			portEntry.Type = []string{domain.StreamPriorityTypeEthertype.String()}
		case netdl.PSPTypeL3Tcp:
			portEntry.Type = []string{domain.StreamPriorityTypeTcp.String()}
		case netdl.PSPTypeL3Udp:
			portEntry.Type = []string{domain.StreamPriorityTypeUdp.String()}
		}

		if streamPriority.Type == netdl.PSPTypeInactive {
			portEntry.Enable = domain.IngressDisabled
		} else {
			portEntry.Enable = domain.IngressEnabled
		}

		if streamPriority.EtherType != nil {
			ethTypeInt, err := strconv.ParseInt(*streamPriority.EtherType, 0, 64)
			if err != nil {
				logger.Warnf("Invalid Ethertype %s in stream priority entry, skip it. Error: %v", *streamPriority.EtherType, err)
			} else {
				portEntry.Ethertype = int(ethTypeInt)
			}
		}

		if streamPriority.SubType != nil {
			subTypeInt, err := strconv.ParseInt(*streamPriority.SubType, 0, 64)
			if err != nil {
				logger.Warnf("Invalid Subtype %s in stream priority entry, skip it. Error: %v", *streamPriority.SubType, err)
				portEntry.SubtypeEnable = domain.IngressSubTypeDisabled
			} else {
				portEntry.Subtype = int(subTypeInt)
				portEntry.SubtypeEnable = domain.IngressSubTypeEnabled
			}
		}

		if streamPriority.TCPPort != nil {
			portEntry.TcpPort = *streamPriority.TCPPort
		}

		if streamPriority.UDPPort != nil {
			portEntry.UdpPort = *streamPriority.UDPPort
		}

		portMap[portEntry.PortId] = append(portMap[portEntry.PortId], *portEntry)
	}

	ifaceStadSlice := make([]domain.InterfaceStadPortEntry, 0, len(portMap))
	for portId, ports := range portMap {
		ifaceStad := domain.InterfaceStadPortEntry{
			InterfaceId:     portId,
			StadPortEntries: ports,
		}
		ifaceStadSlice = append(ifaceStadSlice, ifaceStad)
	}

	result.InterfaceStadPortEntries = ifaceStadSlice

	return result, nil
}

func MappingLoginPolicy(actDeviceID int64, input *netdl.LoginPolicySetting) (*domain.LoginPolicyTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	return &domain.LoginPolicyTable{
		DeviceId:                          actDeviceID,
		LoginMessage:                      input.LoginMessage,
		LoginAuthenticationFailureMessage: input.LoginFailureMessage,
		LoginFailureLockout:               input.AccountLockout,
		RetryFailureThreshold:             input.RetryFailureThreshold,
		LockoutDuration:                   input.FailureLockoutTime,
		AutoLogoutAfter:                   input.AutoLogout,
	}, nil
}

func MappingUserAccounts(actDeviceID int64, input *schema.GetUserAccountData) (*domain.UserAccountTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	// Initialize result with empty accounts map to avoid nil map issues
	result := &domain.UserAccountTable{
		DeviceId:              actDeviceID,
		SyncConnectionAccount: "",
		Accounts:              map[string]domain.UserAccount{},
	}

	for _, acc := range input.AccountList {
		if acc.UserName == "" {
			// Skip entries without a username
			continue
		}
		result.Accounts[acc.UserName] = domain.UserAccount{
			Username: acc.UserName,
			Password: acc.Password, // Typically not provided by devices; mapped if present
			Role:     acc.Authority,
			Email:    acc.Email,
			Active:   acc.Enable != nil && *acc.Enable,
		}
	}

	return result, nil
}

func MappingInformationSetting(actDeviceID int64, input *schema.DeviceBase) (*domain.InformationSettingTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	informationSetting := &domain.InformationSettingTable{
		DeviceId: actDeviceID,
	}
	if input.ContactInfo != nil {
		informationSetting.ContactInformation = *input.ContactInfo
	}
	if input.DeviceName != nil {
		informationSetting.DeviceName = *input.DeviceName
	}
	if input.Description != nil {
		informationSetting.Description = *input.Description
	}
	if input.Location != nil {
		informationSetting.Location = *input.Location
	}
	return informationSetting, nil
}

func MappingSyslogSetting(actDeviceID int64, input *netdl.SyslogSetting) (*domain.SyslogSettingTable, error) {
	if input == nil {
		return nil, fmt.Errorf("nil input for device %d", actDeviceID)
	}

	result := &domain.SyslogSettingTable{
		DeviceId: actDeviceID,
		Enabled:  input.LoggingEnable,
	}

	// Map up to 3 syslog servers from the server table
	for i, server := range input.ServerTable {
		if i >= 3 {
			break // Only support up to 3 servers
		}

		switch i {
		case 0:
			result.SyslogServer1 = server.Enable
			result.Address1 = server.Address
			result.Port1 = server.UdpPort
		case 1:
			result.SyslogServer2 = server.Enable
			result.Address2 = server.Address
			result.Port2 = server.UdpPort
		case 2:
			result.SyslogServer3 = server.Enable
			result.Address3 = server.Address
			result.Port3 = server.UdpPort
		}
	}

	return result, nil
}

const builtinEthernetModuleSlot = 1

func MappingDeviceModules(mafDeviceModules *netdl.Modules) (*domain.ModularConfiguration, error) {
	if mafDeviceModules == nil {
		return nil, fmt.Errorf("nil input for device modules")
	}

	modularConfig := &domain.ModularConfiguration{
		Ethernet: make(map[int]int64),
		Power:    make(map[int]int64),
	}

	if etherModuleMap == nil {
		err := initEtherModuleList()
		if err != nil {
			logger.Errorf("failed to init ether module list: %v", err)
			return nil, fmt.Errorf("failed to init ether module list: %v", err)
		}
	}

	if powerModuleMap == nil {
		err := initPowerModuleList()
		if err != nil {
			logger.Errorf("failed to init power module list: %v", err)
			return nil, fmt.Errorf("failed to init power module list: %v", err)
		}
	}

	for _, etherModule := range mafDeviceModules.Ethernet {
		// Skip not exists module
		if etherModule == nil {
			continue
		}

		// Skip builtin slot
		if etherModule.SlotID == builtinEthernetModuleSlot {
			continue
		}

		// case insensitive match, compared by uppercase
		moduleNameUppercase := strings.ToUpper(etherModule.ModuleName)

		if moduleName, ok := etherModuleNameAliasMap[moduleNameUppercase]; ok {
			if id, ok := etherModuleMap[moduleName]; ok {
				modularConfig.Ethernet[etherModule.SlotID] = id
			}
		} else {
			logger.Warn("Ethernet Module(%d:%s) not found", etherModule.SlotID, etherModule.ModuleName)
		}

	}

	for _, powerModule := range mafDeviceModules.Power {
		// Skip not exists module
		if powerModule == nil {
			continue
		}

		// case insensitive match, compared by uppercase
		moduleNameUppercase := strings.ToUpper(powerModule.ModuleName)

		if moduleName, ok := powerModuleNameAliasMap[moduleNameUppercase]; ok {
			if id, ok := powerModuleMap[moduleName]; ok {
				modularConfig.Power[powerModule.SlotID] = id
			}
		} else {
			logger.Warn("Power Module(%d:%s) not found", powerModule.SlotID, powerModule.ModuleName)
		}
	}

	return modularConfig, nil
}

func MappingCBSetting(actDeviceID int64, mafFrer *netdl.FrerSetting, streamIdentity *domain.StreamIdentity) (*domain.CbTable, error) {
	return nil, fmt.Errorf("CB Table mapping not supported yet")

	// cbTable := &domain.CbTable{
	// 	DeviceId: actDeviceID,
	// }

	// return cbTable, nil
}

func MappingGCLSetting(actDeviceID int64, mafTimeAwareShaper *netdl.TimeAwareShaperSetting) (*domain.GCLTable, error) {
	if mafTimeAwareShaper == nil {
		return nil, fmt.Errorf("nil input for device TimeAwareSharperSetting")
	}

	gclTable := &domain.GCLTable{
		DeviceId:                 actDeviceID,
		InterfacesGateParameters: []domain.InterfaceGateParameter{},
	}

	for _, portSetting := range mafTimeAwareShaper.TimeAwareShaperSetting {
		portId := portSetting.PortID
		portEnable := portSetting.Enable
		gcl := portSetting.GateControlList

		controlList := []domain.AdminControlList{}
		var cycleTime int = 0

		for _, timeSlot := range gcl {
			interval := timeSlot.Interval
			slotId := timeSlot.Slot
			queueList := timeSlot.OpenQueues

			cycleTime = cycleTime + int(interval*1000)

			// queue list [0,1,2] -> 0000 0111 (binary) -> 7 (dec)
			var gateStateValue int = 0
			for _, queue := range queueList {
				gateStateValue += 1 << queue
			}

			controlSlot := domain.AdminControlList{
				Index:         slotId,
				OperationName: "set-gate-states",
				SgsParams: domain.SgsParams{
					GateStatesValue:   gateStateValue,
					TimeIntervalValue: int(interval * 1000),
				},
			}
			controlList = append(controlList, controlSlot)
		}

		gateParams := domain.GateParameters{
			AdminBaseTime: domain.BaseTime{
				FractionalSeconds: 0,
				Second:            0,
			},
			AdminControlList:       controlList,
			AdminControlListLength: len(controlList),
			AdminCycleTime: domain.TimeRatio{
				Numerator:   int64(cycleTime),
				Denominator: 1000000000,
			},
			AdminCycleTimeExtension: 0,
			AdminGateStates:         0,
			ConfigChange:            true,
			GateEnabled:             portEnable,
		}

		interfaceGateParam := domain.InterfaceGateParameter{
			InterfaceId:    int64(portId),
			GateParameters: gateParams,
		}

		gclTable.InterfacesGateParameters = append(gclTable.InterfacesGateParameters, interfaceGateParam)
	}

	return gclTable, nil
}
