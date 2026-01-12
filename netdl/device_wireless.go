package netdl

type WirelessConfiguration struct {
	WifiRfSetting  *WifiRfSetting  `json:"wifiRfSetting,omitempty"`
	WifiAclSetting *WifiACLSetting `json:"wifiAclSetting,omitempty"`
}

type WifiACLSetting struct {
	Setting string `json:"setting,omitempty"`
	// todo
}

type WifiRfSetting struct {
	// todo
}
