package scan

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func GetModules(mafDeviceId string) (*netdl.Modules, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	modules, _, err := dmManager.GetModules(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &modules, nil
}

func GetVlanSetting(mafDeviceId string) (*netdl.VlanSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	vlanSetting, _, err := dmManager.GetVlanSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &vlanSetting, nil
}

func GetStpRstpSetting(mafDeviceId string) (*netdl.RedundancySpanningTreeSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	rstpSetting, _, err := dmManager.GetStpRstpSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &rstpSetting, nil
}

func GetRedundancyProtocolsEnabled(mafDeviceId string) (*schema.RedundancyProtocols, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	enabledRdtProtocols, _, err := dmManager.GetRedundancyProtocolsEnabled(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &enabledRdtProtocols, nil
}

func GetLoginPolicy(mafDeviceId string) (*netdl.LoginPolicySetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	loginPolicy, _, err := dmManager.GetLoginPolicy(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &loginPolicy, nil
}

func GetSystemTime(mafDeviceId string) (*netdl.SystemTimeConfig, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	systemTime, _, err := dmManager.GetSystemTime(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &systemTime, nil
}

func GetMgmtInterface(mafDeviceId string) (*netdl.MgmtInterfaceSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	mgmtInterface, _, err := dmManager.GetMgmtInterface(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &mgmtInterface, nil
}

func GetPortSetting(mafDeviceId string) ([]netdl.PortSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	portSetting, _, err := dmManager.GetPortSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return portSetting, nil
}

func GetDaylightSaving(mafDeviceId string) (*netdl.SetSystemDaylightSavingInput, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	daylightSaving, _, err := dmManager.GetDaylightSaving(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &daylightSaving, nil
}

func GetLoopProtectionSetting(mafDeviceId string) (*netdl.LoopProtectionSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	loopProtectionSetting, _, err := dmManager.GetLoopProtectionSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &loopProtectionSetting, nil
}

func GetMulticastStaticForwarding(mafDeviceId string) (*netdl.MulticastStaticForwardingStruct, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	multicastStaticForwarding, _, err := dmManager.GetMulticastStaticForwarding(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &multicastStaticForwarding, nil
}

func GetUnicastStaticForwarding(mafDeviceId string) (*netdl.UnicastStaticForwardingStruct, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	unicastStaticForwarding, _, err := dmManager.GetUnicastStaticForwarding(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &unicastStaticForwarding, nil
}

func GetTimeSyncSetting(mafDeviceId string) (*netdl.TimeSyncSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	timeSyncSetting, _, err := dmManager.GetTimeSyncSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &timeSyncSetting, nil
}

// InformationSettingTables
func GetDeviceInformation(mafDeviceId string) (*schema.DeviceBase, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	deviceInfo, _, err := dmManager.FetchDeviceInfo(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &deviceInfo, nil
}

func GetUserAccounts(mafDeviceId string) (*schema.GetUserAccountData, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	userAccounts, _, err := dmManager.GetUserAccounts(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &userAccounts, nil
}

func GetEthInterface(mafDeviceId string, interfaceID string) (*netdl.EthInterfaceSettings, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	ethInterface, _, err := dmManager.GetEthInterface(mafDeviceId, schema.GetEthInterfaceInput{
		InterfaceID: interfaceID,
	})
	if err != nil {
		return nil, err
	}
	return &ethInterface, nil
}

func GetEthInterfaceList(mafDeviceId string) ([]netdl.EthInterfaceSettings, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	ethInterfaceList, _, err := dmManager.GetEthInterfaceList(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return ethInterfaceList, nil
}

func GetPcpSetting(mafDeviceId string) (*netdl.PcpSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	pcpSetting, _, err := dmManager.GetPcpSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &pcpSetting, nil
}

func GetSNMPTrapInfo(mafDeviceId string) ([]netdl.SNMPTrapSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	snmpTrapInfo, _, err := dmManager.GetSNMPTrap(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return snmpTrapInfo, nil
}

func GetPerStreamPriority(mafDeviceId string) (*netdl.PerStreamPrioritySetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	perStreamPriority, _, err := dmManager.GetPerStreamPriority(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &perStreamPriority, nil
}

func GetSyslogSetting(mafDeviceId string) (*netdl.SyslogSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}
	syslogSetting, _, err := dmManager.GetSyslog(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &syslogSetting, nil
}

func GetStreamIdentificationSetting(mafDeviceId string) (*netdl.StreamIdentificationSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	streaIdentificationSetting, _, err := dmManager.GetStreamIdentificationSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &streaIdentificationSetting, nil
}

func GetFrerSetting(mafDeviceId string) (*netdl.FrerSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	frerSetting, _, err := dmManager.GetFrerSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}
	return &frerSetting, nil
}

func GetTimeAwareShaperSetting(mafDeviceId string) (*netdl.TimeAwareShaperSetting, error) {
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	timeAwareShaperSetting, _, err := dmManager.GetTimeAwareShaperSetting(mafDeviceId)
	if err != nil {
		return nil, err
	}

	return &timeAwareShaperSetting, nil
}
