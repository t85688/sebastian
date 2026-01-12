from asyncua.common.methods import call_method_full
from asyncua.common.type_dictionary_builder import get_ua_class
from asyncua import ua
import time
import json

async def check_device_profile(namespace, device_profile_path, tsn_profile_folder):
    return await call_method_full(tsn_profile_folder,
                                    await tsn_profile_folder.get_child(f'{namespace["ClassBased"]}:CheckTsnDeviceProfile'),
                                    device_profile_path)


async def import_device_profile(namespace, device_profile_path, tsn_profile_folder):
    with open(device_profile_path, 'r') as file:
        return await call_method_full(tsn_profile_folder,
                                      await tsn_profile_folder.get_child(f'{namespace["ClassBased"]}:ImportTsnDeviceProfile'),
                                      file.read())


async def remove_device_profile(namespace, node_id, tsn_profile_folder):
    return await call_method_full(tsn_profile_folder,
                                    await tsn_profile_folder.get_child(f'{namespace["ClassBased"]}:RemoveTsnDeviceProfile'),
                                    node_id)


async def add_tsn_project(namespace, TsnProjectName, tsn_project_folder):
    return await call_method_full(tsn_project_folder,
                                  await tsn_project_folder.get_child(f'{namespace["ClassBased"]}:AddTsnProject'),
                                  TsnProjectName)

async def remove_tsn_project(namespace, TsnProjectName, tsn_project_folder):
    return await call_method_full(tsn_project_folder,
                                  await tsn_project_folder.get_child(f'{namespace["ClassBased"]}:RemoveTsnProject'),
                                  TsnProjectName)

async def set_tsn_project_setting(namespace, json_configuration, tsn_project_node):
    TsnProjectSettingDataType = get_ua_class('TsnProjectSettingDataType')()
    TsnProjectSettingDataType.TsnProjectName = json_configuration['TsnProjectName']
    TsnProjectSettingDataType.KeepPreviousResult = json_configuration['KeepPreviousResult']
    TsnProjectSettingDataType.MinTsnVlan = json_configuration['MinTsnVlan']
    TsnProjectSettingDataType.MaxTsnVlan = json_configuration['MaxTsnVlan']
    # TsnProjectPcpSetting
    TsnProjectPcpSettingDataType = get_ua_class('TsnProjectPcpSettingDataType')()
    TsnProjectPcpSettingDataType.Queue0 = json_configuration['TsnProjectPcpSetting']['Queue0']
    TsnProjectPcpSettingDataType.Queue1 = json_configuration['TsnProjectPcpSetting']['Queue1']
    TsnProjectPcpSettingDataType.Queue2 = json_configuration['TsnProjectPcpSetting']['Queue2']
    TsnProjectPcpSettingDataType.Queue3 = json_configuration['TsnProjectPcpSetting']['Queue3']
    TsnProjectPcpSettingDataType.Queue4 = json_configuration['TsnProjectPcpSetting']['Queue4']
    TsnProjectPcpSettingDataType.Queue5 = json_configuration['TsnProjectPcpSetting']['Queue5']
    TsnProjectPcpSettingDataType.Queue6 = json_configuration['TsnProjectPcpSetting']['Queue6']
    TsnProjectPcpSettingDataType.Queue7 = json_configuration['TsnProjectPcpSetting']['Queue7']
    TsnProjectSettingDataType.TsnProjectPcpSetting = TsnProjectPcpSettingDataType
    # NETCONF
    NETCONFDataType = get_ua_class('NETCONFDataType')()
    NETCONFDataType.Username = json_configuration['NETCONF']['Username']
    NETCONFDataType.Password = json_configuration['NETCONF']['Password']
    NETCONFDataType.SSHPort = json_configuration['NETCONF']['SSHPort']
    TsnProjectSettingDataType.NETCONF = NETCONFDataType
    # SNMP
    SNMPDataType = get_ua_class('SNMPDataType')()
    SNMPDataType.ReadCommunity = json_configuration['SNMP']['ReadCommunity']
    SNMPDataType.WriteCommunity = json_configuration['SNMP']['WriteCommunity']
    SNMPDataType.Port = json_configuration['SNMP']['Port']
    SNMPDataType.Version = json_configuration['SNMP']['Version']
    TsnProjectSettingDataType.SNMP = SNMPDataType
    # RESTful
    RESTfulDataType = get_ua_class('RESTfulDataType')()
    RESTfulDataType.Username = json_configuration['RESTful']['Username']
    RESTfulDataType.Password = json_configuration['RESTful']['Password']
    RESTfulDataType.Port = json_configuration['RESTful']['Port']
    TsnProjectSettingDataType.RESTful = RESTfulDataType

    # call method full for the setting
    


    return await call_method_full(tsn_project_node,
                                  await tsn_project_node.get_child(f'{namespace["ClassBased"]}:SetTsnProjectSetting'),
                                  TsnProjectSettingDataType)

async def get_all_stream_computed_result(namespace, tsn_project_node):
    return await call_method_full(tsn_project_node,
                                  await tsn_project_node.get_child(f'{namespace["ClassBased"]}:GetAllStreamComputedResult'))

async def get_stream_computed_result(namespace, stream_name, tsn_project_node):
    return await call_method_full(tsn_project_node,
                                  await tsn_project_node.get_child(f'{namespace["ClassBased"]}:GetStreamComputedResult'), stream_name)

async def compute_tsn_project(namespace, client, compute_node):
    await call_method_full(compute_node,
                           await compute_node.get_child(f'{namespace["ClassBased"]}:Start'))
    cnt = 0
    while True:
        current_state = await client.get_node(f'{compute_node}.CurrentState').get_value()
        if current_state.Text != "Running":
            print("\n")
            return await client.get_node(f'{compute_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def compare_tsn_project(namespace, client, compare_node):
    await call_method_full(compare_node,
                           await compare_node.get_child(f'{namespace["ClassBased"]}:Start'))
    cnt = 0
    while True:
        current_state = await client.get_node(f'{compare_node}.CurrentState').get_value()
        if current_state.Text != "Running":
            print("\n")
            return await client.get_node(f'{compare_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def deploy_tsn_project(namespace, client, deploy_node):
    await call_method_full(deploy_node,
                           await deploy_node.get_child(f'{namespace["ClassBased"]}:Start'))
    cnt = 0
    while True:
        current_state = await client.get_node(f'{deploy_node}.CurrentState').get_value()
        if current_state.Text != "Running":
            print("\n")
            return await client.get_node(f'{deploy_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def auto_scan_tsn_project(namespace, client, json_configuration, auto_scan_node):
    Configuration = []

    for auto_scan_json in json_configuration:
        TsnAutoScanDataType = get_ua_class('TsnAutoScanDataType')()
        TsnAutoScanDataType.FirstIpAddress = auto_scan_json['FirstIpAddress']
        TsnAutoScanDataType.LastIpAddress = auto_scan_json['LastIpAddress']
        # NETCONF
        try:
            NETCONFDataType = get_ua_class('NETCONFDataType')()
            NETCONFDataType.Username = auto_scan_json['NETCONF']['Username']
            NETCONFDataType.Password = auto_scan_json['NETCONF']['Password']
            NETCONFDataType.SSHPort = auto_scan_json['NETCONF']['SSHPort']
            TsnAutoScanDataType.NETCONF = NETCONFDataType
        except: pass
        # SNMP
        try:
            SNMPDataType = get_ua_class('SNMPDataType')()
            SNMPDataType.ReadCommunity = auto_scan_json['SNMP']['ReadCommunity']
            SNMPDataType.WriteCommunity = auto_scan_json['SNMP']['WriteCommunity']
            SNMPDataType.Port = auto_scan_json['SNMP']['Port']
            SNMPDataType.Version = auto_scan_json['SNMP']['Version']
            TsnAutoScanDataType.SNMP = SNMPDataType
        except: pass
        # RESTful
        try:
            RESTfulDataType = get_ua_class('RESTfulDataType')()
            RESTfulDataType.Username = auto_scan_json['RESTful']['Username']
            RESTfulDataType.Password = auto_scan_json['RESTful']['Password']
            RESTfulDataType.Port = auto_scan_json['RESTful']['Port']
            TsnAutoScanDataType.RESTful = RESTfulDataType
        except: pass
        Configuration.append(TsnAutoScanDataType)

    await call_method_full(auto_scan_node,
                           await auto_scan_node.get_child(f'{namespace["ClassBased"]}:Start'),
                           Configuration)
    cnt = 0
    while True:
        current_state = await client.get_node(f'{auto_scan_node}.CurrentState').get_value()
        if current_state.Text != "Running":
            print("\n")
            return await client.get_node(f'{auto_scan_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def add_tsn_bridges(namespace, json_configuration, tsn_bridge_folder):
    TsnBridgeDataType = get_ua_class('TsnBridgeDataType')()
    TsnBridgeDataType.IpAddress = json_configuration['IpAddress']
    TsnBridgeDataType.DeviceProfileName = json_configuration['DeviceProfileName']
    # NETCONF
    NETCONFDataType = get_ua_class('NETCONFDataType')()
    NETCONFDataType.Username = json_configuration['NETCONF']['Username']
    NETCONFDataType.Password = json_configuration['NETCONF']['Password']
    NETCONFDataType.SSHPort = json_configuration['NETCONF']['SSHPort']
    TsnBridgeDataType.NETCONF = NETCONFDataType
    # SNMP
    SNMPDataType = get_ua_class('SNMPDataType')()
    SNMPDataType.ReadCommunity = json_configuration['SNMP']['ReadCommunity']
    SNMPDataType.WriteCommunity = json_configuration['SNMP']['WriteCommunity']
    SNMPDataType.Port = json_configuration['SNMP']['Port']
    SNMPDataType.Version = json_configuration['SNMP']['Version']
    TsnBridgeDataType.SNMP = SNMPDataType
    # RESTful
    RESTfulDataType = get_ua_class('RESTfulDataType')()
    RESTfulDataType.Username = json_configuration['RESTful']['Username']
    RESTfulDataType.Password = json_configuration['RESTful']['Password']
    RESTfulDataType.Port = json_configuration['RESTful']['Port']
    TsnBridgeDataType.RESTful = RESTfulDataType

    return await call_method_full(tsn_bridge_folder,
                                  await tsn_bridge_folder.get_child(f'{namespace["ClassBased"]}:AddTsnBridge'),
                                  TsnBridgeDataType)

async def add_tsn_end_stations(namespace, json_configuration, tsn_end_station_folder):
    TsnEndStationDataType = get_ua_class('TsnEndStationDataType')()
    TsnEndStationDataType.IpAddress = json_configuration['IpAddress']
    TsnEndStationDataType.DeviceProfileName = json_configuration['DeviceProfileName']
    # Interfaces
    for json_interface in json_configuration['TsnEthernetInterfaces']:
        TsnEthernetInterfaceDataType = get_ua_class('TsnEthernetInterfaceDataType')()
        try:
            TsnEthernetInterfaceDataType.PhysAddress = json_interface['PhysAddress']
        except:
            pass
        TsnEthernetInterfaceDataType.InterfaceName = json_interface['InterfaceName']
        TsnEndStationDataType.TsnEthernetInterfaces.append(TsnEthernetInterfaceDataType)

    return await call_method_full(tsn_end_station_folder,
                                  await tsn_end_station_folder.get_child(f'{namespace["ClassBased"]}:AddTsnEndStation'),
                                  TsnEndStationDataType)

async def add_tsn_bridged_end_stations(namespace, json_configuration, tsn_bridged_end_station_folder):
    TsnBridgedEndStationDataType = get_ua_class('TsnBridgedEndStationDataType')()
    TsnBridgedEndStationDataType.IpAddress = json_configuration['IpAddress']
    TsnBridgedEndStationDataType.DeviceProfileName = json_configuration['DeviceProfileName']
    # NETCONF
    NETCONFDataType = get_ua_class('NETCONFDataType')()
    NETCONFDataType.Username = json_configuration['NETCONF']['Username']
    NETCONFDataType.Password = json_configuration['NETCONF']['Password']
    NETCONFDataType.SSHPort = json_configuration['NETCONF']['SSHPort']
    TsnBridgedEndStationDataType.NETCONF = NETCONFDataType
    # SNMP
    SNMPDataType = get_ua_class('SNMPDataType')()
    SNMPDataType.ReadCommunity = json_configuration['SNMP']['ReadCommunity']
    SNMPDataType.WriteCommunity = json_configuration['SNMP']['WriteCommunity']
    SNMPDataType.Port = json_configuration['SNMP']['Port']
    SNMPDataType.Version = json_configuration['SNMP']['Version']
    TsnBridgedEndStationDataType.SNMP = SNMPDataType
    # RESTful
    RESTfulDataType = get_ua_class('RESTfulDataType')()
    RESTfulDataType.Username = json_configuration['RESTful']['Username']
    RESTfulDataType.Password = json_configuration['RESTful']['Password']
    RESTfulDataType.Port = json_configuration['RESTful']['Port']
    TsnBridgedEndStationDataType.RESTful = RESTfulDataType
    # Interfaces
    for json_interface in json_configuration['TsnEthernetInterfaces']:
        TsnEthernetInterfaceDataType = get_ua_class('TsnEthernetInterfaceDataType')()
        try:
            TsnEthernetInterfaceDataType.PhysAddress = json_interface['PhysAddress']
        except:
            pass
        TsnEthernetInterfaceDataType.InterfaceName = json_interface['InterfaceName']
        TsnBridgedEndStationDataType.TsnEthernetInterfaces.append(TsnEthernetInterfaceDataType)

    return await call_method_full(tsn_bridged_end_station_folder,
                                  await tsn_bridged_end_station_folder.get_child(f'{namespace["ClassBased"]}:AddTsnBridgedEndStation'),
                                  TsnBridgedEndStationDataType)

async def add_tsn_links(namespace, json_configuration, tsn_link_folder):
    TsnLinkDataType = get_ua_class('TsnLinkDataType')()
    TsnLinkDataType.SourceIpAddress = json_configuration['SourceIpAddress']
    TsnLinkDataType.SourceInterfaceName = json_configuration['SourceInterfaceName']
    TsnLinkDataType.DestinationIpAddress = json_configuration['DestinationIpAddress']
    TsnLinkDataType.DestinationInterfaceName = json_configuration['DestinationInterfaceName']

    return await call_method_full(tsn_link_folder,
                                  await tsn_link_folder.get_child(f'{namespace["ClassBased"]}:AddTsnLink'),
                                  TsnLinkDataType)


async def set_time_slot(namespace, json_configuration, time_slot_folder):
    timeSlotDataTypes = []
    for idx, json_time_slot in enumerate(json_configuration):
        TimeSlotDataType = get_ua_class('TimeSlotDataType')()
        TimeSlotDataType.TimeSlotIndex = json_time_slot['TimeSlotIndex']
        TimeSlotDataType.TimeSlotPeriod = json_time_slot['TimeSlotPeriod']
        TimeSlotDataType.TimeSlotTrafficType = json_time_slot['TimeSlotTrafficType']
        timeSlotDataTypes.append(TimeSlotDataType)

    return await call_method_full(time_slot_folder,
                                  await time_slot_folder.get_child(f'{namespace["ClassBased"]}:SetTimeSlot'),
                                  timeSlotDataTypes)

async def add_class_based_stream(namespace, json_configuration, class_based_stream_folder):
    ClassBasedStreamDataType = get_ua_class('ClassBasedStreamDataType')()
    ClassBasedStreamDataType.StreamName = json_configuration['StreamName']

    # TsnVlanTag
    try:
        IeeeTsnVlanTagDataType = get_ua_class('IeeeTsnVlanTagDataType')()
        IeeeTsnVlanTagDataType.VlanId = json_configuration['TsnVlanTag']['VlanId']
        IeeeTsnVlanTagDataType.PriorityCodePoint = json_configuration['TsnVlanTag']['PriorityCodePoint']
        ClassBasedStreamDataType.TsnVlanTag = IeeeTsnVlanTagDataType
    except: pass

    # CCLinkIEStream
    try:
        CCLinkIEStreamDataType = get_ua_class('CCLinkIEStreamDataType')()
        CCLinkIEStreamDataType.EtherType = json_configuration['CCLinkIEStream']['EtherType']
        try:
            CCLinkIEStreamDataType.SubType = json_configuration['CCLinkIEStream']['SubType']
        except: pass
        ClassBasedStreamDataType.CCLinkIEStream = CCLinkIEStreamDataType
    except: pass

    # DestinationIpAddress
    try:
        ClassBasedStreamDataType.DestinationAddress = json_configuration['DestinationAddress']
    except: pass

    # TalkerInterface
    TsnInterfaceTalkerDataType = get_ua_class('TsnInterfaceTalkerDataType')()
    TsnInterfaceTalkerDataType.IpAddress = json_configuration['TalkerInterface']['IpAddress']
    TsnInterfaceTalkerDataType.InterfaceName = json_configuration['TalkerInterface']['InterfaceName']
    ClassBasedStreamDataType.TalkerInterface = TsnInterfaceTalkerDataType
    # ListenerInterface
    for json_listener_interface in json_configuration['ListenerInterface']:
        TsnInterfaceListenerDataType = get_ua_class('TsnInterfaceListenerDataType')()
        TsnInterfaceListenerDataType.IpAddress = json_listener_interface['IpAddress']
        TsnInterfaceListenerDataType.InterfaceName = json_listener_interface['InterfaceName']
        ClassBasedStreamDataType.ListenerInterface.append(TsnInterfaceListenerDataType)

    return await call_method_full(class_based_stream_folder,
                                  await class_based_stream_folder.get_child(f'{namespace["ClassBased"]}:AddClassBasedStream'),
                                  ClassBasedStreamDataType)

async def get_discovered_devices(namespace, broadcast_search_and_ip_setting_node):
    return await call_method_full(broadcast_search_and_ip_setting_node,
                                  await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:GetDiscoveredDevices'))

async def start_init(namespace, client, broadcast_search_and_ip_setting_node):
    return await call_method_full(broadcast_search_and_ip_setting_node,
                           await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:StartInit'))

async def start_device_discovery(namespace, client, json_configuration, broadcast_search_and_ip_setting_node):
    DeviceDiscoveryDataType = get_ua_class('DeviceDiscoveryDataType')()
    DeviceDiscoveryDataType.MoxaIndustrialEthernetSwitch = json_configuration['MoxaIndustrialEthernetSwitch']
    # SNMP
    try:
        SNMPDataType = get_ua_class('SNMPDataType')()
        SNMPDataType.ReadCommunity = json_configuration['SNMP']['ReadCommunity']
        SNMPDataType.WriteCommunity = json_configuration['SNMP']['WriteCommunity']
        SNMPDataType.Port = json_configuration['SNMP']['Port']
        SNMPDataType.Version = json_configuration['SNMP']['Version']
        DeviceDiscoveryDataType.SNMP = SNMPDataType
    except: pass
    # RESTful
    try:
        RESTfulDataType = get_ua_class('RESTfulDataType')()
        RESTfulDataType.Username = json_configuration['RESTful']['Username']
        RESTfulDataType.Password = json_configuration['RESTful']['Password']
        RESTfulDataType.Port = json_configuration['RESTful']['Port']
        DeviceDiscoveryDataType.RESTful = RESTfulDataType
    except: pass

    await call_method_full(broadcast_search_and_ip_setting_node,
                           await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:StartDeviceDiscovery'),
                           DeviceDiscoveryDataType)
    cnt = 0
    while True:
        current_state = await client.get_node(f'{broadcast_search_and_ip_setting_node}.CurrentState').get_value()
        if current_state.Text != "DeviceDiscovering":
            return await client.get_node(f'{broadcast_search_and_ip_setting_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def start_retry_connection(namespace, client, json_configuration, broadcast_search_and_ip_setting_node):
    # SNMP
    SNMPDataType = get_ua_class('SNMPDataType')()
    SNMPDataType.ReadCommunity = json_configuration['SNMP']['ReadCommunity']
    SNMPDataType.WriteCommunity = json_configuration['SNMP']['WriteCommunity']
    SNMPDataType.Port = json_configuration['SNMP']['Port']
    SNMPDataType.Version = json_configuration['SNMP']['Version']
    # RESTful
    RESTfulDataType = get_ua_class('RESTfulDataType')()
    RESTfulDataType.Username = json_configuration['RESTful']['Username']
    RESTfulDataType.Password = json_configuration['RESTful']['Password']
    RESTfulDataType.Port = json_configuration['RESTful']['Port']

    await call_method_full(broadcast_search_and_ip_setting_node,
                           await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:StartRetryConnection'),
                           SNMPDataType, RESTfulDataType)
    cnt = 0
    while True:
        current_state = await client.get_node(f'{broadcast_search_and_ip_setting_node}.CurrentState').get_value()
        if current_state.Text != "RetryConnection":
            return await client.get_node(f'{broadcast_search_and_ip_setting_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def start_link_sequence(namespace, client, json_configuration, broadcast_search_and_ip_setting_node):
    await call_method_full(broadcast_search_and_ip_setting_node,
                           await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:StartLinkSequenceDetect'))
    cnt = 0
    while True:
        current_state = await client.get_node(f'{broadcast_search_and_ip_setting_node}.CurrentState').get_value()
        if current_state.Text != "LinkSequenceDetecting":
            return await client.get_node(f'{broadcast_search_and_ip_setting_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)

async def start_ip_assign(namespace, client, json_configuration, broadcast_search_and_ip_setting_node):
    Configuration = []

    for ip_assign_json in json_configuration:
        TsnIpConfigureDataType = get_ua_class('TsnIpConfigureDataType')()
        TsnIpConfigureDataType.MacAddress = ip_assign_json['MacAddress']
        TsnIpConfigureDataType.AssignedIpAddress = ip_assign_json['AssignedIpAddress']
        Configuration.append(TsnIpConfigureDataType)

    await call_method_full(broadcast_search_and_ip_setting_node,
                           await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:StartIpAssign'), Configuration)

async def start_ip_configure(namespace, client, json_configuration, broadcast_search_and_ip_setting_node):
    Configuration = []

    for ip_configure_json in json_configuration:
        TsnIpConfigureDataType = get_ua_class('TsnIpConfigureDataType')()
        TsnIpConfigureDataType.MacAddress = ip_configure_json['MacAddress']
        TsnIpConfigureDataType.AssignedIpAddress = ip_configure_json['AssignedIpAddress']
        Configuration.append(TsnIpConfigureDataType)

    await call_method_full(broadcast_search_and_ip_setting_node,
                           await broadcast_search_and_ip_setting_node.get_child(f'{namespace["ClassBased"]}:StartIpConfigure'), Configuration)
    cnt = 0
    while True:
        current_state = await client.get_node(f'{broadcast_search_and_ip_setting_node}.CurrentState').get_value()
        if current_state.Text != "IpConfiguring":
            return await client.get_node(f'{broadcast_search_and_ip_setting_node}.ErrorCode').get_value()
        cnt += 1
        msg = "\r{}{}"
        progress = msg.format(current_state.Text, "." * cnt)
        print(progress, end="")
        time.sleep(1)