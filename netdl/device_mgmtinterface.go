package netdl

type MgmtInterfaceSetting struct {
	Http              *Http        `json:"http,omitempty"`
	Https             *Https       `json:"https,omitempty"`
	Telnet            *Telnet      `json:"telnet,omitempty"`
	Ssh               *Ssh         `json:"ssh,omitempty"`
	Snmp              *Snmp        `json:"snmp,omitempty"`
	MoxaService       *MoxaService `json:"moxaService,omitempty"`
	MaxLoginHttpHttps *int         `json:"maxLoginHttpHttps,omitempty"`
	MaxLoginTelnetSsh *int         `json:"maxLoginTelnetSsh,omitempty"`
}
type Http struct {
	Enable *bool `json:"enable,omitempty"`
	Port   *int  `json:"port,omitempty"`
}
type Https struct {
	Enable *bool `json:"enable,omitempty"`
	Port   *int  `json:"port,omitempty"`
}
type Telnet struct {
	Enable *bool `json:"enable,omitempty"`
	Port   *int  `json:"port,omitempty"`
}
type Ssh struct {
	Enable *bool `json:"enable,omitempty"`
	Port   *int  `json:"port,omitempty"`
}
type Snmp struct {
	Enable   *bool   `json:"enable,omitempty"`
	Port     *int    `json:"port,omitempty"`
	Protocol *string `json:"protocol" validate:"oneof=udp tcp,omitempty"`
}
type MoxaService struct {
	Enable *bool `json:"enable,omitempty"`
}
