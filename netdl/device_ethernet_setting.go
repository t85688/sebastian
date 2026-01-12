package netdl

const (
	Ipv4NetworkModeStatic = "static"
	Ipv4NetworkModeDhcp   = "dhcp"
	Ipv4NetworkModeBootp  = "bootp"
)

const (
	Ipv6NetworkModeStatic    = "static"
	Ipv6NetworkModeDhcp      = "dhcp"
	Ipv6NetworkModeLinkLocal = "link-local"
)

type Ipv4NetworkSettings struct {
	Mode      string `json:"mode" validate:"required,oneof=static dhcp bootp"`
	IpAddress string `json:"ipAddress" validate:"required_if=Mode static,ip"`
	Netmask   string `json:"netmask" validate:"required_if=Mode static,ip"`
	Gateway   string `json:"gateway" validate:"ip"`
	Dns1      string `json:"dns1" validate:"ip"`
	Dns2      string `json:"dns2" validate:"ip"`
}

type Ipv6NetworkSettings struct {
	Mode      string `json:"mode" validate:"required,oneof=static dhcp link-local"`
	IpAddress string `json:"ipAddress" validate:"required_if=Mode static,ipv6"`
	Prefix    string `json:"prefix" validate:"required_if=Mode static,regexp=^(128|(?:[0-9]{1,2}|1[01][0-9]|12[0-7]))$"`
	Gateway   string `json:"gateway" validate:"ipv6"`
	Dns1      string `json:"dns1" validate:"ipv6"`
	Dns2      string `json:"dns2" validate:"ipv6"`
}

type EthInterfaceSettings struct {
	InterfaceID   string               `json:"interfaceId" validate:"required"`
	Enable        bool                 `json:"enable" validate:"required"`
	Mode          string               `json:"mode" validate:"required,oneof=lan wan bridge cellular"`
	Ipv4NSettings *Ipv4NetworkSettings `json:"ipv4,omitempty" validate:"omitempty,dive"`
	Ipv6NSettings *Ipv6NetworkSettings `json:"ipv6,omitempty" validate:"omitempty,dive"`
}
