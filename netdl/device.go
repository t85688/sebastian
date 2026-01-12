// Package entity package entity
package netdl

import (
	"database/sql/driver"
	"encoding/json"
	"sync"
	"time"

	afstr "gitlab.com/moxa/sw/maf/moxa-app-framework/internal/utils/string"
	"gorm.io/datatypes"
)

type Device struct {
	// Device Basic Data
	DeviceBasic

	// Device Communication Status
	Communication *Communication `json:"communication"`

	// Device SystemStatus
	SystemStatus *SystemStatus     `json:"systemStatus,omitempty"`
	Interfaces   []*Interface      `json:"interfaces,omitempty"`
	Ports        []*Port           `json:"ports,omitempty"`
	Redundancy   *RedundancyStatus `json:"redundancy,omitempty"`
	Modules      *Modules          `json:"modules,omitempty"`

	// Device Configurations
	Configuration *Configuration `json:"configuration,omitempty"`

	LastUpdated time.Time `json:"lastUpdated,omitempty"`
}

type MilBase struct {
	MilVersion   string `json:"milVersion,omitempty" gorm:"column:mil_version;type:varchar(50)"`
	SecurityType string `json:"securityType,omitempty" gorm:"column:security_type;type:varchar(50)"`
}

type DeviceBasic struct {
	// Device Basic Information
	DeviceId        string            `json:"deviceId" gorm:"primary_key;column:device_id;type:text"`
	SerialNumber    string            `json:"serialNumber" gorm:"column:serial_number;type:varchar(15)"`
	ModelName       string            `json:"modelName" gorm:"column:model_name;type:varchar(50)"`
	FirmwareVersion string            `json:"firmwareVersion" gorm:"column:firmware_version;type:varchar(50)"`
	ImageVersion    string            `json:"imageVersion,omitempty" gorm:"column:image_version;type:varchar(50)"`
	Protocol        afstr.StringArray `json:"protocol,omitempty" gorm:"column:protocol;type:text"`
	ProtocolData    ProtocolData      `json:"protocolData,omitempty" gorm:"column:protocol_data;type:text"`
	ProductRevision string            `json:"productRevision,omitempty" gorm:"column:product_revision;type:varchar(50)"`
	Uptime          string            `json:"uptime,omitempty" gorm:"column:uptime;type:varchar(50)"`

	// Device System Information
	DeviceName  *string `json:"deviceName,omitempty" gorm:"column:device_name;type:varchar(255)"`
	Location    *string `json:"location,omitempty" gorm:"column:location;type:varchar(255)"`
	Description *string `json:"description,omitempty" gorm:"column:description;type:text"`
	ContactInfo *string `json:"contactInfo,omitempty" gorm:"column:contact_info;type:text"`

	// Device Connection Information
	IP             string   `json:"ip" gorm:"column:ip;type:varchar(15)"`
	MAC            string   `json:"mac" gorm:"column:mac;type:varchar(12)"`
	SubnetMask     *string  `json:"subnetMask,omitempty" gorm:"-"`
	DefaultGateway *string  `json:"defaultGateway,omitempty" gorm:"-"`
	DnsServers     []string `json:"dnsServers,omitempty" gorm:"-"`

	// IPC Device features
	MilBase

	// TBD
	DeviceType string `json:"deviceType,omitempty" gorm:"-"`
	DeviceMe
}

func (db *DeviceBasic) DeepCopy(mutex *sync.RWMutex) *DeviceBasic {
	if db == nil {
		return nil
	}
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	clone := &DeviceBasic{
		DeviceId:        db.DeviceId,
		DeviceName:      db.DeviceName,
		SerialNumber:    db.SerialNumber,
		ModelName:       db.ModelName,
		FirmwareVersion: db.FirmwareVersion,
		ImageVersion:    db.ImageVersion,
		Location:        db.Location,
		Protocol:        nil,
		Description:     db.Description,
		ContactInfo:     db.ContactInfo,
		ProductRevision: db.ProductRevision,
		Uptime:          db.Uptime,
		IP:              db.IP,
		MAC:             db.MAC,
		SubnetMask:      nil,
		DefaultGateway:  nil,
		DnsServers:      nil,
		DeviceType:      db.DeviceType,
		DeviceMe:        *db.DeviceMe.DeepCopy(mutex),
		ProtocolData:    db.ProtocolData,
	}

	if db.Protocol != nil {
		clone.Protocol = make(afstr.StringArray, len(db.Protocol))
		copy(clone.Protocol, db.Protocol)
	}
	if db.SubnetMask != nil {
		subnetMask := *db.SubnetMask
		clone.SubnetMask = &subnetMask
	}
	if db.DefaultGateway != nil {
		defaultGateway := *db.DefaultGateway
		clone.DefaultGateway = &defaultGateway
	}
	if db.DnsServers != nil {
		clone.DnsServers = make([]string, len(db.DnsServers))
		copy(clone.DnsServers, db.DnsServers)
	}

	return clone
}

func (DeviceBasic) TableName() string {
	return "device"
}

type DeviceMe struct {
	MeMacs      []string `json:"meMacs,omitempty" gorm:"-"`
	MeIpAliases []string `json:"meIpAliases,omitempty" gorm:"-"`
}

func (d *DeviceMe) DeepCopy(mutex *sync.RWMutex) *DeviceMe {
	if d == nil {
		return nil
	}
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	clone := &DeviceMe{
		MeMacs:      make([]string, len(d.MeMacs)),
		MeIpAliases: make([]string, len(d.MeIpAliases)),
	}

	copy(clone.MeMacs, d.MeMacs)
	copy(clone.MeIpAliases, d.MeIpAliases)

	return clone
}

func (d *DeviceBasic) GetIdentity() string {
	return d.IP
}

type DeviceLocalModel struct {
	ProductLine string  `json:"productLine" gorm:"column:product_line;type:varchar(20)"`
	ARPMac      *string `json:"arpMac,omitempty" gorm:"column:arp_mac;"`
	FromNIC     *string `json:"fromNIC,omitempty" gorm:"column:from_nic;"`
	FromLib     *string `json:"fromLib,omitempty" gorm:"column:from_lib;"`
	DeviceNIC   int     `json:"deviceNIC,omitempty" gorm:"column:device_nic;"`     // specific for nport/mgate productline
	SdsciPrefix string  `json:"sdsciPrefix,omitempty" gorm:"column:sdsci_prefix;"` // specific for nport/mgate productline
}

type DeviceSparkplugBModel struct {
	// DeviceDLMModel
	RawProperties datatypes.JSON `json:"rawProperties" gorm:"column:raw_properties;type:jsonb"`
	EnrollCert    string         `json:"enrollCert" gorm:"column:enroll_cert;type:varchar(4096)"`
	EnrollPK      string         `json:"enrollPK" gorm:"column:enroll_pk;type:varchar(4096)"`
}

type DeviceDLMModel struct {
	ProjectId        string    `json:"projectId" gorm:"column:project_id;type:type:uuid"`
	Hostname         string    `json:"hostname" gorm:"column:hostname;type:type:varchar(255)"`
	OSType           string    `json:"osType" gorm:"column:os_type;type:type:varchar(10)"`
	ConnectionStatus string    `json:"connectionStatus" gorm:"column:connection_status;type:varchar(20)"`
	ConfigStatus     string    `json:"configStatus" gorm:"column:config_status;type:varchar(20)"`
	ConfigUpdatedAt  time.Time `json:"configUpdatedAt" gorm:"column:config_updated_at;type:timestamptz"`
	LastBootTime     time.Time `json:"lastBootTime" gorm:"column:last_boot_time;type:timestamptz"`
	ConnectedAt      time.Time `json:"connectedAt" gorm:"column:connected_at;type:timestamptz"`
	DisconnectedAt   time.Time `json:"disconnectedAt" gorm:"column:disconnected_at;type:timestamptz"`
	CreatedAt        time.Time `json:"createdAt" gorm:"column:created_at;type:timestamptz"`
	UpdatedAt        time.Time `json:"updatedAt" gorm:"column:updated_at;type:timestamptz"`
}

type DeviceLwM2MModel struct {
	DeviceDLMModel
	ICCID string `json:"iccid" gorm:"column:iccid;type:varchar(20)"`
	PSK   string `json:"psk" gorm:"column:psk;type:varchar(255)"`
}

type DeviceZeusModel struct {
	Hostname       *string        `json:"hostname,omitempty"`
	Product        *string        `json:"product,omitempty"`
	PSK            *string        `json:"psk,omitempty"`
	Location       *string        `json:"location,omitempty"`
	Profile        datatypes.JSON `json:"profile,omitempty"`
	Online         *bool          `json:"online,omitempty"`
	ConnectedAt    *time.Time     `json:"connectedAt,omitempty"`
	DisconnectedAt *time.Time     `json:"disconnectedAt,omitempty"`
	LastBootTime   *time.Time     `json:"lastBootTime,omitempty"`
	CreatedAt      *time.Time     `json:"createdAt,omitempty"`
	UpdatedAt      *time.Time     `json:"updatedAt,omitempty"`
}

type ProtocolData struct {
	Local      *DeviceLocalModel      `json:"local,omitempty"`
	SparkplugB *DeviceSparkplugBModel `json:"sparkplugb,omitempty"`
	LwM2M      *DeviceLwM2MModel      `json:"lwm2m,omitempty"`
	Zeus       *DeviceZeusModel       `json:"zeus,omitempty"`
}

func (d ProtocolData) String() string {
	bytes, _ := json.Marshal(d)
	return string(bytes)
}

func (d ProtocolData) Value() (driver.Value, error) {
	return json.Marshal(d)
}

func (d *ProtocolData) Scan(value interface{}) error {
	return json.Unmarshal(value.([]byte), &d)
}
