package netdl

type Configuration struct {
	RedundancyProtocols     []string                       `json:"redundancyProtocols"`
	MgmtInterfaceSetting    *MgmtInterfaceSetting          `json:"mgmtInterfaceSetting,omitempty"`
	VlanSetting             *VlanSetting                   `json:"vlanSetting,omitempty"`
	RstpSetting             *RedundancySpanningTreeSetting `json:"rstpSetting,omitempty"`
	PRPHSRSetting           *RedundancyPRPHSRSetting       `json:"prphsrSetting,omitempty"`
	SupervisionFrameSetting *SupervisionFrameSetting       `json:"supervisionFrameSetting,omitempty"`
	SyslogSetting           *SyslogSetting                 `json:"syslogServerSetting,omitempty"`
	SNMPTrapSetting         *[]SNMPTrapSetting             `json:"snmpTrapSetting,omitempty"`
	SystemTimeConfig        *SystemTimeConfig              `json:"systemTimeConfig,omitempty"`
	PortSetting             *[]PortSetting                 `json:"portSetting,omitempty"`
	LoginPolicySetting      *LoginPolicySetting            `json:"loginPolicySetting,omitempty"`
	LoopProtectionSetting   *LoopProtectionSetting         `json:"loopProtectionSetting,omitempty"`
	PcpSetting              *PcpSetting                    `json:"pcpSetting,omitempty"`
	DaylightSavingSetting   *SetSystemDaylightSavingInput  `json:"daylightSavingSetting,omitempty"`
	PerStreamPriority       *PerStreamPrioritySetting      `json:"perStreamPriority,omitempty"`
	TimeAwareShaper         *TimeAwareShaperSetting        `json:"timeAwareShaper,omitempty"`
	EthInterfaceSettings    *[]EthInterfaceSettings        `json:"ethInterfaceSettings,omitempty"`
	LinkupDelaySetting      *LinkupDelaySetting            `json:"linkupDelaySetting,omitempty"`

	WirelessConfiguration
}
