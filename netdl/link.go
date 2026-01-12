package netdl

import (
	"fmt"
	"sync"
)

type Link struct {
	LinkInner

	lock sync.Mutex
}

type LinkInner struct {
	ID string `json:"id,omitempty"`
	LinkIdentity

	Speed    uint64          `json:"speed,omitempty"`
	Status   string          `json:"status,omitempty"`
	LinkType map[string]bool `json:"linkType,omitempty"` // [trunk] true wireless true

	VPN        *LinkVPN        `json:"vpn,omitempty"`        // no use, to be removed
	SFP        *LinkSFP        `json:"sfp,omitempty"`        // no use, to be removed
	Redundancy *LinkRedundancy `json:"redundancy,omitempty"` // no use, to be removed

	Traffic      *LinkTraffic         `json:"traffic,omitempty"`
	PktErrorRate *LinkPacketErrorRate `json:"pktErrorRate,omitempty"`
}

func (link *Link) GetLinkTypes() []LinkType {
	link.lock.Lock()
	defer link.lock.Unlock()

	var types []LinkType
	for t := range link.LinkType {
		types = append(types, NewLinkType(t))
	}

	return types
}

func (link *Link) HasLinkType(linkType LinkType) bool {
	link.lock.Lock()
	defer link.lock.Unlock()

	if link.LinkType == nil {
		return false
	}

	return link.LinkType[linkType.String()]
}

func (link *Link) AddLinkType(linkType LinkType) {
	link.lock.Lock()
	defer link.lock.Unlock()

	if link.LinkType == nil {
		link.LinkType = make(map[string]bool)
	}

	link.LinkType[linkType.String()] = true
}

func (link *Link) RemoveLinkType(linkType LinkType) {
	link.lock.Lock()
	defer link.lock.Unlock()
	delete(link.LinkType, linkType.String())
}

type LinkIdentity struct {
	// TempID         string      `json:"-"`
	FromDevice     LinkDeviceBasic `json:"fromDevice,omitempty"`
	FromDeviceID   string          `json:"fromDeviceID,omitempty"`
	ToDevice       LinkDeviceBasic `json:"toDevice,omitempty"`
	ToDeviceID     string          `json:"toDeviceID,omitempty"`
	FromPort       int             `json:"fromPort,omitempty"`
	ToPort         int             `json:"toPort,omitempty"`
	UseCustomLabel bool            `json:"useCustomLabel,omitempty"`
	FromPortLabel  string          `json:"fromPortLabel,omitempty"`
	ToPortLabel    string          `json:"toPortLabel,omitempty"`
}

type LinkDeviceBasic struct {
	DeviceId string `json:"deviceId" gorm:"primary_key;column:device_id;type:text"`
	IP       string `json:"ip" gorm:"column:ip;type:varchar(15)"`
	MAC      string `json:"mac" gorm:"column:mac;type:varchar(12)"`
}

func (l *LinkDeviceBasic) GetIdentity() string {
	return l.IP
}

func (l *LinkIdentity) DeepCopy(mutex *sync.RWMutex) *LinkIdentity {
	if l == nil {
		return nil
	}
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	return &LinkIdentity{
		FromDevice: LinkDeviceBasic{
			DeviceId: l.FromDevice.DeviceId,
			IP:       l.FromDevice.IP,
			MAC:      l.FromDevice.MAC,
		},
		FromDeviceID: l.FromDeviceID,
		ToDevice: LinkDeviceBasic{
			DeviceId: l.ToDevice.DeviceId,
			IP:       l.ToDevice.IP,
			MAC:      l.ToDevice.MAC,
		},
		ToDeviceID: l.ToDeviceID,
		FromPort:   l.FromPort,
		ToPort:     l.ToPort,
	}
}

func (l *LinkIdentity) Swap() {
	l.FromDevice, l.ToDevice = l.ToDevice, l.FromDevice
	l.FromDeviceID, l.ToDeviceID = l.ToDeviceID, l.FromDeviceID
	l.FromPort, l.ToPort = l.ToPort, l.FromPort
}

func (l *LinkIdentity) GetExactIdentity() string {
	from := l.FromDevice.GetIdentity()
	to := l.ToDevice.GetIdentity()
	fromPort := l.FromPort
	toPort := l.ToPort
	if from > to {
		from, to = to, from
		fromPort, toPort = toPort, fromPort
	}

	if fromPort != 0 {
		from = fmt.Sprintf("%s:%d", from, fromPort)
	}
	if toPort != 0 {
		to = fmt.Sprintf("%s:%d", to, toPort)
	}

	return fmt.Sprintf("%s-%s", from, to)
}

func (l *LinkIdentity) GetIdentities() []string {
	fromNode := l.FromDevice.GetIdentity()
	toNode := l.ToDevice.GetIdentity()
	fromPort := l.FromPort
	toPort := l.ToPort
	if fromNode > toNode {
		fromNode, toNode = toNode, fromNode
		fromPort, toPort = toPort, fromPort
	}

	output := []string{}

	from := fromNode
	to := toNode

	if fromPort != 0 {
		from = fmt.Sprintf("%s:%d", fromNode, fromPort)
		output = append(output, fmt.Sprintf("%s-%s", from, toNode))
	}
	if toPort != 0 {
		to = fmt.Sprintf("%s:%d", toNode, toPort)
		output = append(output, fmt.Sprintf("%s-%s", fromNode, to))
	}
	if fromPort != 0 && toPort != 0 {
		output = append(output, fmt.Sprintf("%s-%s", from, to))
	}

	// Handle case where both ports are 0 (device-to-device link)
	if fromPort == 0 && toPort == 0 {
		// Use exact identity logic for 0-port links
		output = append(output, fmt.Sprintf("%s-%s", fromNode, toNode))
	}

	return output
}

type LinkVPN struct {
	LocalVPNConnectionName  string `json:"localVPNConnectionName,omitempty"`
	RemoteVPNConnectionName string `json:"remoteVPNConnectionName,omitempty"`
	VPNFromIP               string `json:"vpnFromIP,omitempty"`
	VPNToIP                 string `json:"vpnToIP,omitempty"`
}

type LinkWirelessData struct {
	WifiChannel int `json:"wifiChannel,omitempty"`
	WifiSNR     int `json:"wifiSNR,omitempty"`
	Band        int `json:"band,omitempty"`
	Bandwidth   int `json:"bandwidth,omitempty"`
}

type LinkSFP struct {
	FromTxPower     string `json:"fromTxPower,omitempty"`
	FromRxPower     string `json:"fromRxPower,omitempty"`
	FromTemperature string `json:"fromTemperature,omitempty"`
	FromVoltage     string `json:"fromVoltage,omitempty"`
	ToTxPower       string `json:"toTxPower,omitempty"`
	ToRxPower       string `json:"toRxPower,omitempty"`
	ToTemperature   string `json:"toTemperature,omitempty"`
	ToVoltage       string `json:"toVoltage,omitempty"`
}

type LinkRedundancy struct {
	Status string `json:"status,omitempty" enums:"blocking,forwarding"`
	Type   string `json:"type,omitempty" enums:"turboRing,turboRingV2,turboChain,dualHoming"`
}

type LinkTraffic struct {
	RecordTime     uint64 `json:"recordTime"`
	InUtilization  uint64 `json:"inUtilization"`
	OutUtilization uint64 `json:"outUtilization"`
}

type LinkPacketErrorRate struct {
	RecordTime uint64 `json:"recordTime"`
	InError    uint64 `json:"inError"`
	OutError   uint64 `json:"outError"`
}
