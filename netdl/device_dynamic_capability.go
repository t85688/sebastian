package netdl

type DynamicCapability struct {
	Support *SupportCapability `json:"support,omitempty"`
}

type SupportCapability struct {
	Console       *ConsoleCapability       `json:"console,omitempty"`
	Locate        *LocateCapability        `json:"locate,omitempty"`
	AssignIp      *AssignIpCapability      `json:"assignIp,omitempty"`
	AllowList     *AllowListCapability     `json:"allowList,omitempty"`
	Configuration *ConfigurationCapability `json:"configuration,omitempty"`
	Certificate   *CertificateCapability   `json:"certificate,omitempty"`
	DsciType      *DsciTypeCapability      `json:"dsciType,omitempty"`
}

type ConsoleCapability struct {
	Http      *int `json:"http,omitempty"`
	HttpIpv6  *int `json:"http_ipv6,omitempty"`
	Https     *int `json:"https,omitempty"`
	HttpsIpv6 *int `json:"https_ipv6,omitempty"`
	Ssh       *int `json:"ssh,omitempty"`
	SshIpv6   *int `json:"ssh_ipv6,omitempty"`
	Telnet    *int `json:"telnet,omitempty"`
}

type LocateCapability struct {
	Ipv4Support *bool `json:"ipv4Support,omitempty"`
	Ipv6Support *bool `json:"ipv6Support,omitempty"`
}

type AssignIpCapability struct {
	Ipv6Support       *bool `json:"ipv6Support,omitempty"`
	Ipv6SupportOption *int  `json:"ipv6SupportOption,omitempty"`
}

type AllowListCapability struct {
	AllowListSupport *bool `json:"allowListSupport,omitempty"`
	MaxRulesNumber   *int  `json:"maxRulesNumber,omitempty"`
}

type ConfigurationCapability struct {
	ConfigurationSupport *bool `json:"configurationSupport,omitempty"`
	GsdFileSupport       *bool `json:"gsdFileSupport,omitempty"`
}

type CertificateCapability struct {
	CertificateSupport *bool `json:"certificateSupport,omitempty"`
}

type DsciTypeCapability struct {
	DsciTypeSupport *int `json:"dsciTypeSupport,omitempty"`
}
