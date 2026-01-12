package monitor

import (
	"context"
	"encoding/json"
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
)

func (m *DefaultMonitor) onCreateDevice(ctx context.Context, actDevice *domain.Device) {
	if ctx.Err() != nil {
		logger.Warnf("Context is cancelled, skip onCreateDevice")
		return
	}

	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		logger.Warnf("Project ID not found in context, skip onCreateDevice")
		return
	}

	currentState, _ := m.getInternalState(projectId)
	if currentState != InternalStateRunning {
		logger.Warnf("Monitor is not in running state, skip onCreateDevice")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandPatchUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionCreate.String(),
		actDevice,
	)

	wsResp.Path = fmt.Sprintf("Projects/%d/Devices/%d", projectId, actDevice.Id)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal wsResp: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) onUpdateDevice(ctx context.Context, actDevice *domain.Device) {
	if ctx.Err() != nil {
		logger.Warnf("Context is cancelled, skip onUpdateDevice")
		return
	}

	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		logger.Warnf("Project ID not found in context, skip onUpdateDevice")
		return
	}

	currentState, _ := m.getInternalState(projectId)
	if currentState != InternalStateRunning {
		logger.Warnf("Monitor is not in running state, skip onUpdateDevice")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandPatchUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionUpdate.String(),
		actDevice,
	)

	wsResp.Path = fmt.Sprintf("Projects/%d/Devices/%d", projectId, actDevice.Id)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal wsResp: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) onDeleteDevice(ctx context.Context, actDevice *domain.Device) {
	if ctx.Err() != nil {
		logger.Warnf("Context is cancelled, skip onDeleteDevice")
		return
	}

	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		logger.Warnf("Project ID not found in context, skip onDeleteDevice")
		return
	}

	currentState, _ := m.getInternalState(projectId)

	if currentState != InternalStateRunning {
		logger.Warnf("Monitor is not in running state, skip onDeleteDevice")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandPatchUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionDelete.String(),
		actDevice,
	)

	wsResp.Path = fmt.Sprintf("Projects/%d/Devices/%d", projectId, actDevice.Id)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal wsResp: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) onCreateLink(ctx context.Context, actLink *domain.Link) {
	if ctx.Err() != nil {
		logger.Warnf("Context is cancelled, skip onCreateLink")
		return
	}

	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		logger.Warnf("Project ID not found in context, skip onCreateLink")
		return
	}

	currentState, _ := m.getInternalState(projectId)

	if currentState != InternalStateRunning {
		logger.Warnf("Monitor is not in running state, skip onCreateLink")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandPatchUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionCreate.String(),
		actLink,
	)

	wsResp.Path = fmt.Sprintf("Projects/%d/Links/%d", projectId, actLink.Id)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal wsResp: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

func (m *DefaultMonitor) onDeleteLink(ctx context.Context, actLink *domain.Link) {
	if ctx.Err() != nil {
		logger.Warnf("Context is cancelled, skip onDeleteLink")
		return
	}

	projectId, projectIdExists := getProjectId(ctx)
	if !projectIdExists {
		logger.Warnf("Project ID not found in context, skip onDeleteLink")
		return
	}

	currentState, _ := m.getInternalState(projectId)

	if currentState != InternalStateRunning {
		logger.Warnf("Monitor is not in running state, skip onDeleteLink")
		return
	}

	wsResp := wscommand.NewBaseWsCommandResponse(
		wscommand.ActWSCommandPatchUpdate.Int64(),
		0,
		wscommand.PatchUpdateActionDelete.String(),
		actLink,
	)

	wsResp.Path = fmt.Sprintf("Projects/%d/Links/%d", projectId, actLink.Id)

	jsonBytes, err := json.Marshal(wsResp)
	if err != nil {
		logger.Errorf("Failed to marshal wsResp: %v", err)
		return
	}

	ws.MulticastToSpecifiedProjectId(jsonBytes, projectId)
}

/*

{
  "OpCode": 4097,
  "Path": "Projects/1757835731/Devices/50003",
  "Action": "Delete",
  "Data": {
    "Account": {
      "DefaultSetting": true,
      "Password": "",
      "Username": "admin"
    },
    "Coordinate": {
      "X": 1004,
      "Y": 404
    },
    "DeviceAlias": "",
    "DeviceInfo": {
      "Location": "LLL",
      "ProductRevision": "V1.0.0",
      "RedundantProtocol": "STP/RSTP",
      "SerialNumber": "TBBJB1099685",
      "SystemUptime": "3d6h49m14s"
    },
    "DeviceName": "moxa",
    "DeviceProfileId": 502,
    "DeviceProperty": {
      "Certificate": false,
      "Description": "",
      "DeviceCluster": "Default",
      "FeatureGroup": {
        "AutoScan": {
          "BroadcastSearch": true,
          "DeviceInformation": {
            "DeviceName": true,
            "IPConfiguration": true,
            "InterfaceMAC": true,
            "InterfaceName": true,
            "Location": true,
            "MACTable": true,
            "ModularInfo": true,
            "PortInfo": true,
            "PortSpeed": true,
            "ProductRevision": true,
            "RedundantProtocol": true,
            "SerialNumber": true,
            "SystemUptime": true
          },
          "Identify": {
            "FirmwareVersion": true,
            "ModelName": true,
            "VendorID": true
          },
          "LLDP": true
        },
        "Configuration": {
          "CheckConfigSynchronization": false,
          "InformationSetting": true,
          "LoginPolicy": true,
          "LoopProtection": true,
          "ManagementInterface": true,
          "NetworkSetting": true,
          "PortSetting": {
            "AdminStatus": true
          },
          "SNMPTrapSetting": true,
          "STPRSTP": {
            "BPDUFilter": true,
            "BPDUGuard": true,
            "ErrorRecoveryTime": true,
            "LinkType": true,
            "LoopGuard": true,
            "PortRSTPEnable": true,
            "RSTP": true,
            "RootGuard": true,
            "Swift": false
          },
          "StaticForwardSetting": {
            "Multicast": true,
            "Unicast": true
          },
          "SyslogSetting": true,
          "TSN": {
            "IEEE802Dot1CB": false,
            "IEEE802Dot1Qbv": false
          },
          "TimeSetting": {
            "PTP": false,
            "SystemTime": true
          },
          "TimeSyncSetting": {
            "IEC61850_2016": false,
            "IEEE1588_2008": false,
            "IEEE1588_2008_ClockMode": false,
            "IEEE1588_2008_ClockType": false,
            "IEEE1588_2008_MaximumStepsRemoved": false,
            "IEEE802Dot1AS_2011": false,
            "IEEEC37Dot238_2017": false
          },
          "UserAccount": true,
          "VLANSetting": {
            "AccessTrunkMode": true,
            "DefaultPCP": true,
            "DefaultPVID": true,
            "HybridMode": true,
            "ManagementVLAN": true,
            "PerStreamPriority": false,
            "PerStreamPriorityV2": false,
            "TEMSTID": false
          }
        },
        "Monitor": {
          "BasicStatus": {
            "FiberCheck": true,
            "PortStatus": true,
            "SystemUtilization": true
          },
          "Redundancy": {
            "RSTP": true
          },
          "TimeSynchronization": {
            "IEEE1588_2008": false,
            "IEEE802Dot1AS_2011": false
          },
          "Traffic": {
            "TrafficUtilization": true,
            "TxTotalOctets": true,
            "TxTotalPackets": true
          }
        },
        "Operation": {
          "CLI": true,
          "EnableSNMPService": true,
          "EventLog": true,
          "FactoryDefault": true,
          "FirmwareUpgrade": true,
          "ImportExport": true,
          "Locator": true,
          "Reboot": true
        }
      },
      "GCLOffsetMaxDuration": 0,
      "GCLOffsetMinDuration": 0,
      "GateControlListLength": 0,
      "Ieee802Dot1cb": {
        "SequenceGeneration": {
          "Max": 1023,
          "Min": 0
        },
        "SequenceIdentification": {
          "Max": 1023,
          "Min": 0
        },
        "SequenceRecovery": {
          "Max": 1535,
          "MaxHistoryLength": 15,
          "Min": 0,
          "MinHistoryLength": 2,
          "ResetTimeout": 4096
        },
        "StreamIdentity": {
          "Max": 1535,
          "MaxHandle": 2147483647,
          "Min": 0,
          "MinHandle": 0
        }
      },
      "Ieee802VlanTag": {
        "PriorityCodePoint": 0,
        "VlanId": 0
      },
      "L2Family": "",
      "L3Series": "",
      "L4Series": "",
      "MaxVlanCfgSize": 256,
      "ModelName": "MDS-G4020",
      "MountType": [
        "DinRail",
        "WallMount",
        "RackMount"
      ],
      "NumberOfQueue": 0,
      "OperatingTemperatureC": {
        "Max": 60,
        "Min": -10
      },
      "PerQueueSize": 0,
      "PhysicalModelName": "MDS-G4020",
      "ProcessingDelayMap": {},
      "PtpQueueId": 0,
      "ReservedVlan": [
        1
      ],
      "StreamPriorityConfigIngressIndexMax": 0,
      "TickGranularity": 0,
      "TrafficSpecification": {
        "EnableMaxBytesPerInterval": false,
        "Interval": {
          "Denominator": 1000000000,
          "Numerator": 30000
        },
        "MaxBytesPerInterval": 0,
        "MaxFrameSize": 46,
        "MaxFramesPerInterval": 1,
        "TimeAware": {
          "EarliestTransmitOffset": 0,
          "Jitter": 0,
          "LatestTransmitOffset": 999999999
        }
      },
      "Vendor": "MOXA"
    },
    "DeviceRole": "Unknown",
    "DeviceStatus": {
      "ICMPStatus": false,
      "NETCONFStatus": false,
      "NewMOXACommandStatus": false,
      "RESTfulStatus": true,
      "SNMPStatus": true
    },
    "DeviceType": "Switch",
    "Distance": 0,
    "EnableSnmpSetting": true,
    "FirmwareFeatureProfileId": -1,
    "FirmwareVersion": "v4.0 Build 2023_0830_0002",
    "Id": 50003,
    "Interfaces": [
      {
        "Active": true,
        "CableTypes": [
          "Copper"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 1,
        "InterfaceName": "Eth1/1",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          10,
          100,
          1000
        ],
        "Used": false
      },
      {
        "Active": true,
        "CableTypes": [
          "Copper"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 2,
        "InterfaceName": "Eth1/2",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          10,
          100,
          1000
        ],
        "Used": true
      },
      {
        "Active": true,
        "CableTypes": [
          "Copper"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 3,
        "InterfaceName": "Eth1/3",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          10,
          100,
          1000
        ],
        "Used": false
      },
      {
        "Active": true,
        "CableTypes": [
          "Copper"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 4,
        "InterfaceName": "Eth1/4",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          10,
          100,
          1000
        ],
        "Used": false
      },
      {
        "Active": true,
        "CableTypes": [
          "Fiber"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 5,
        "InterfaceName": "Eth2/1",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          100,
          1000
        ],
        "Used": false
      },
      {
        "Active": true,
        "CableTypes": [
          "Fiber"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 6,
        "InterfaceName": "Eth2/2",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          100,
          1000
        ],
        "Used": true
      },
      {
        "Active": true,
        "CableTypes": [
          "Fiber"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 7,
        "InterfaceName": "Eth2/3",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          100,
          1000
        ],
        "Used": false
      },
      {
        "Active": true,
        "CableTypes": [
          "Fiber"
        ],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 8,
        "InterfaceName": "Eth2/4",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [
          100,
          1000
        ],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 9,
        "InterfaceName": "Eth3/1",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 10,
        "InterfaceName": "Eth3/2",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 11,
        "InterfaceName": "Eth3/3",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 12,
        "InterfaceName": "Eth3/4",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 13,
        "InterfaceName": "Eth4/1",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 14,
        "InterfaceName": "Eth4/2",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 15,
        "InterfaceName": "Eth4/3",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 16,
        "InterfaceName": "Eth4/4",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 17,
        "InterfaceName": "Eth5/1",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 18,
        "InterfaceName": "Eth5/2",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 19,
        "InterfaceName": "Eth5/3",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      },
      {
        "Active": false,
        "CableTypes": [],
        "Description": "",
        "DeviceId": 50003,
        "InterfaceId": 20,
        "InterfaceName": "Eth5/4",
        "IpAddress": "192.168.127.20",
        "MacAddress": "",
        "Management": false,
        "RootGuard": false,
        "SupportSpeeds": [],
        "Used": false
      }
    ],
    "Ipv4": {
      "DNS1": "",
      "DNS2": "",
      "Gateway": "",
      "IpAddress": "192.168.127.20",
      "SubnetMask": "255.255.252.0"
    },
    "MacAddress": "00-90-E8-AF-F6-41",
    "ModularConfiguration": {
      "Ethernet": {
        "2": 103
      },
      "Power": {
        "2": 203
      }
    },
    "NetconfConfiguration": {
      "DefaultSetting": true,
      "NetconfOverSSH": {
        "SSHPort": 830
      },
      "NetconfOverTLS": {
        "CaCerts": "",
        "CertFile": "",
        "KeyFile": "",
        "TLSPort": 6513
      },
      "TLS": false
    },
    "RestfulConfiguration": {
      "DefaultSetting": true,
      "Port": 443,
      "Protocol": "HTTPS"
    },
    "SSHPort": 22,
    "SnmpConfiguration": {
      "AuthenticationPassword": "",
      "AuthenticationType": "None",
      "DataEncryptionKey": "",
      "DataEncryptionType": "None",
      "DefaultSetting": true,
      "Port": 161,
      "ReadCommunity": "",
      "Username": "moxa",
      "Version": "v2c",
      "WriteCommunity": ""
    },
    "Tier": 0
  }
}




*/

/*

{
  "OpCode": 4097,
  "Path": "Projects/1757835731/Links/50001",
  "Action": "Delete",
  "Data": {
    "Alive": true,
    "CableLength": 1,
    "CableType": "Copper",
    "DestinationDeviceId": 50001,
    "DestinationDeviceIp": "192.168.127.1",
    "DestinationInterfaceId": 8,
    "Id": 50001,
    "PropagationDelay": 5,
    "Redundant": false,
    "SourceDeviceId": 50003,
    "SourceDeviceIp": "192.168.127.20",
    "SourceInterfaceId": 2,
    "Speed": 1000
  }
}
*/
