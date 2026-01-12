from asyncua.common.methods import call_method_full, call_method
from asyncua.common.type_dictionary_builder import get_ua_class
from asyncua import Client
from asyncua import ua
import time
import json
import inspect

async def get_namespace(client):
    namespace = {'OpcFoundation': await client.get_namespace_index('http://opcfoundation.org/UA/'),                'OpcuaServer': await client.get_namespace_index('urn:Moxa:OpcUaServer'),                'OpcuaBnm': await client.get_namespace_index('http://opcfoundation.org/UA/BNM/'),                'ClassBased': await client.get_namespace_index('http://www.moxa.com/auto-configuration-tool-class-based-model/'),                'MoxaServer': await client.get_namespace_index('http://www.moxa.com/UA/')}
    return namespace

async def login(server_url, username, password):
    client = Client(url=f'{server_url}')
    client.set_user(username)
    client.set_password(password)

    await client.connect()
    await client.load_type_definitions()

    return client

async def import_device_profile(client, device_profile_config):
    namespace = await get_namespace(client)
    profile_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:DeviceProfiles'])
    with open(device_profile_config, 'r') as file:
        ret = await call_method_full(profile_folder,
                                    await profile_folder.get_child(f'{namespace["ClassBased"]}:ImportDeviceProfile'),
                                    file.read())
        if (ret.OutputArguments[1] != 0):
            raise Exception("Error: " + ret.OutputArguments[2])

async def remove_device_profile(client, device_profile_name):
    namespace = await get_namespace(client)
    profile_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:DeviceProfiles'])
    device_profile_node = await profile_folder.get_child(f'{namespace["MoxaServer"]}:{device_profile_name}')
    ret = await call_method_full(profile_folder,
                                await profile_folder.get_child(f'{namespace["ClassBased"]}:RemoveDeviceProfile'),
                                device_profile_node.nodeid)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def check_device_profile(client, device_profile_name):
    namespace = await get_namespace(client)
    profile_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:DeviceProfiles'])
    ret = await call_method_full(profile_folder,
                                await profile_folder.get_child(f'{namespace["ClassBased"]}:CheckDeviceProfile'),
                                device_profile_name)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))
    return ret.OutputArguments[0]

async def add_project(client, project_config):
    namespace = await get_namespace(client)
    project_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:Projects'])
    project_name = project_config['ProjectSetting']['ProjectName']
    try:
        return await project_folder.get_child(f'{namespace["MoxaServer"]}:{project_name}')
    except:
        ret = await call_method_full(project_folder,
                                    await project_folder.get_child(f'{namespace["ClassBased"]}:AddProject'),
                                    project_name)
        if (ret.OutputArguments[1] != 0):
            if "Warning" in str(ret.OutputArguments[1]):
                print(ret.OutputArguments[1])
            else:
                raise Exception("Error: " + str(ret.OutputArguments[1]))

        return client.get_node(ret.OutputArguments[0])

async def remove_project(client, project_config):
    namespace = await get_namespace(client)
    project_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:Projects'])
    project_name = project_config['ProjectSetting']['ProjectName']
    ret = await call_method_full(project_folder,
                                  await project_folder.get_child(f'{namespace["ClassBased"]}:RemoveProject'),
                                  project_name)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def set_project_setting(client, project_node, project_config):
    if ('ProjectSetting' in project_config):
        project_setting_config = project_config['ProjectSetting']
        ProjectSettingDataType = get_ua_class('ProjectSettingDataType')()

        if ('ProjectName' in project_setting_config):
            ProjectSettingDataType.ProjectName = project_setting_config['ProjectName']

        if ('ConnectionAccount' in project_setting_config):
            connection_account_config = project_setting_config['ConnectionAccount']
            ProjectSettingDataType.ConnectionAccount = get_ua_class('ConnectionAccountDataType')()
            if ('UserName' in connection_account_config):
                ProjectSettingDataType.ConnectionAccount.UserName = connection_account_config['UserName']
            if ('Password' in connection_account_config):
                ProjectSettingDataType.ConnectionAccount.Password = connection_account_config['Password']

        if ('BaseIpSetting' in project_setting_config):
            base_ip_setting_config = project_setting_config['BaseIpSetting']
            ProjectSettingDataType.BaseIpSetting = get_ua_class('IpSettingDataType')()
            if ('IpAddress' in base_ip_setting_config):
                ProjectSettingDataType.BaseIpSetting.IpAddress = base_ip_setting_config['IpAddress']
            if ('SubnetMask' in base_ip_setting_config):
                ProjectSettingDataType.BaseIpSetting.SubnetMask = base_ip_setting_config['SubnetMask']
            if ('Gateway' in base_ip_setting_config):
                ProjectSettingDataType.BaseIpSetting.Gateway = base_ip_setting_config['Gateway']
            if ('DNS1' in base_ip_setting_config):
                ProjectSettingDataType.BaseIpSetting.DNS1 = base_ip_setting_config['DNS1']
            if ('DNS2' in base_ip_setting_config):
                ProjectSettingDataType.BaseIpSetting.DNS2 = base_ip_setting_config['DNS2']

        if ('NETCONF' in project_setting_config):
            NETCONF_config = project_setting_config['NETCONF']
            ProjectSettingDataType.NETCONF = get_ua_class('NETCONFDataType')()
            if ('SSHPort' in NETCONF_config):
                ProjectSettingDataType.NETCONF.SSHPort = NETCONF_config['SSHPort']

        if ('SNMP' in project_setting_config):
            SNMP_config = project_setting_config['SNMP']
            ProjectSettingDataType.SNMP = get_ua_class('SNMPDataType')()
            if ('WriteCommunity' in SNMP_config):
                ProjectSettingDataType.SNMP.WriteCommunity = SNMP_config['WriteCommunity']
            if ('ReadCommunity' in SNMP_config):
                ProjectSettingDataType.SNMP.ReadCommunity = SNMP_config['ReadCommunity']
            if ('Port' in SNMP_config):
                ProjectSettingDataType.SNMP.Port = SNMP_config['Port']
            if ('Version' in SNMP_config):
                match SNMP_config['Version']:
                    case 'v2c':
                        ProjectSettingDataType.SNMP.Version = get_ua_class('SNMPVersion').v2c
                    case _:
                        raise Exception("Invalid SNMP version value: " + SNMP_config['Version'])

        if ('RESTful' in project_setting_config):
            RESTful_config = project_setting_config['RESTful']
            ProjectSettingDataType.RESTful = get_ua_class('RESTfulDataType')()
            if ('Port' in RESTful_config):
                ProjectSettingDataType.RESTful.Port = RESTful_config['Port']

    ret = await call_method_full(client.get_node(f'{project_node}.ProjectSetting'),
                                client.get_node(f'{project_node}.ProjectSetting.SetProjectSetting'),
                                  ProjectSettingDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def add_bridge(client, project_node, bridge_config):
    BridgeDataType = get_ua_class('BridgeDataType')()
    if ('Alias' in bridge_config):
        BridgeDataType.Alias = bridge_config['Alias']

    if ('DeviceProfileName' in bridge_config):
        BridgeDataType.DeviceProfileName = bridge_config['DeviceProfileName']

    if ('ConnectionAccount' in bridge_config):
        connection_account_config = bridge_config['ConnectionAccount']
        BridgeDataType.ConnectionAccount = get_ua_class('ConnectionAccountDataType')()
        if ('UserName' in connection_account_config):
            BridgeDataType.ConnectionAccount.UserName = connection_account_config['UserName']
        if ('Password' in connection_account_config):
            BridgeDataType.ConnectionAccount.Password = connection_account_config['Password']

    if ('NETCONF' in bridge_config):
        NETCONF_config = bridge_config['NETCONF']
        BridgeDataType.NETCONF = get_ua_class('NETCONFDataType')()
        if ('SSHPort' in NETCONF_config):
            BridgeDataType.NETCONF.SSHPort = NETCONF_config['SSHPort']
            
    if ('SNMP' in bridge_config):
        SNMP_config = bridge_config['SNMP']
        BridgeDataType.SNMP = get_ua_class('SNMPDataType')()
        if ('Port' in SNMP_config):
            BridgeDataType.SNMP.Port = SNMP_config['Port']
        if ('Version' in SNMP_config):
            BridgeDataType.SNMP.Version = SNMP_config['Version']
        if ('WriteCommunity' in SNMP_config):
            BridgeDataType.SNMP.WriteCommunity = SNMP_config['WriteCommunity']
        if ('ReadCommunity' in SNMP_config):
            BridgeDataType.SNMP.ReadCommunity = SNMP_config['ReadCommunity']

    if ('RESTful' in bridge_config):
        RESTful_config = bridge_config['RESTful']
        BridgeDataType.RESTful = get_ua_class('RESTfulDataType')()
        if ('Port' in RESTful_config):
            BridgeDataType.RESTful.Port = RESTful_config['Port']
            
    if ('IpSetting' in bridge_config):
        ip_setting_config = bridge_config['IpSetting']
        BridgeDataType.IpSetting = get_ua_class('IpSettingDataType')()
        if ('IpAddress' in ip_setting_config):
            BridgeDataType.IpSetting.IpAddress = ip_setting_config['IpAddress']
        if ('SubnetMask' in ip_setting_config):
            BridgeDataType.IpSetting.SubnetMask = ip_setting_config['SubnetMask']
        if ('Gateway' in ip_setting_config):
            BridgeDataType.IpSetting.Gateway = ip_setting_config['Gateway']
        if ('DNS1' in ip_setting_config):
            BridgeDataType.IpSetting.DNS1 = ip_setting_config['DNS1']
        if ('DNS2' in ip_setting_config):
            BridgeDataType.IpSetting.DNS2 = ip_setting_config['DNS2']

    try:
        ret = await call_method_full(client.get_node(f'{project_node}.Devices'),
                                    client.get_node(f'{project_node}.Devices.AddBridge'),
                                    BridgeDataType)
        if (ret.OutputArguments[1] != 0):
            raise Exception("Error: " + ret.OutputArguments[2])

        return client.get_node(ret.OutputArguments[0])
    except:
        browse_name = bridge_config['IpSetting']['IpAddress']
        namespace = await get_namespace(client)
        bridge_node = await project_node.get_child([f'{namespace["ClassBased"]}:Devices',
                                                    f'{namespace["MoxaServer"]}:{browse_name}'])
        await set_bridge(client, bridge_node, bridge_config)
        return bridge_node

async def set_bridge(client, bridge_node, bridge_config):
    BridgeDataType = get_ua_class('BridgeDataType')()
    if ('Alias' in bridge_config):
        BridgeDataType.Alias = bridge_config['Alias']

    if ('DeviceProfileName' in bridge_config):
        BridgeDataType.DeviceProfileName = bridge_config['DeviceProfileName']
        
    if ('FirmwareVersion' in bridge_config):
        BridgeDataType.FirmwareVersion = bridge_config['FirmwareVersion']

    if ('ConnectionAccount' in bridge_config):
        connection_account_config = bridge_config['ConnectionAccount']
        BridgeDataType.ConnectionAccount = get_ua_class('ConnectionAccountDataType')()
        if ('UserName' in connection_account_config):
            BridgeDataType.ConnectionAccount.UserName = connection_account_config['UserName']
        if ('Password' in connection_account_config):
            BridgeDataType.ConnectionAccount.Password = connection_account_config['Password']

    if ('NETCONF' in bridge_config):
        NETCONF_config = bridge_config['NETCONF']
        BridgeDataType.NETCONF = get_ua_class('NETCONFDataType')()
        if ('SSHPort' in NETCONF_config):
            BridgeDataType.NETCONF.SSHPort = NETCONF_config['SSHPort']
            
    if ('SNMP' in bridge_config):
        SNMP_config = bridge_config['SNMP']
        BridgeDataType.SNMP = get_ua_class('SNMPDataType')()
        if ('Port' in SNMP_config):
            BridgeDataType.SNMP.Port = SNMP_config['Port']
        if ('Version' in SNMP_config):
            BridgeDataType.SNMP.Version = SNMP_config['Version']
        if ('WriteCommunity' in SNMP_config):
            BridgeDataType.SNMP.WriteCommunity = SNMP_config['WriteCommunity']
        if ('ReadCommunity' in SNMP_config):
            BridgeDataType.SNMP.ReadCommunity = SNMP_config['ReadCommunity']

    if ('RESTful' in bridge_config):
        RESTful_config = bridge_config['RESTful']
        BridgeDataType.RESTful = get_ua_class('RESTfulDataType')()
        if ('Port' in RESTful_config):
            BridgeDataType.RESTful.Port = RESTful_config['Port']
            
    if ('IpSetting' in bridge_config):
        ip_setting_config = bridge_config['IpSetting']
        BridgeDataType.IpSetting = get_ua_class('IpSettingDataType')()
        if ('IpAddress' in ip_setting_config):
            BridgeDataType.IpSetting.IpAddress = ip_setting_config['IpAddress']
        if ('SubnetMask' in ip_setting_config):
            BridgeDataType.IpSetting.SubnetMask = ip_setting_config['SubnetMask']
        if ('Gateway' in ip_setting_config):
            BridgeDataType.IpSetting.Gateway = ip_setting_config['Gateway']
        if ('DNS1' in ip_setting_config):
            BridgeDataType.IpSetting.DNS1 = ip_setting_config['DNS1']
        if ('DNS2' in ip_setting_config):
            BridgeDataType.IpSetting.DNS2 = ip_setting_config['DNS2']

    ret = await call_method_full(bridge_node,
                                client.get_node(f'{bridge_node}.SetBridgeSetting'),
                                BridgeDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def add_end_station(client, project_node, end_station_config):
    EndStationDataType = get_ua_class('EndStationDataType')()
    if ('Alias' in end_station_config):
        EndStationDataType.Alias = end_station_config['Alias']

    if ('DeviceProfileName' in end_station_config):
        EndStationDataType.DeviceProfileName = end_station_config['DeviceProfileName']

    if ('ConnectionAccount' in end_station_config):
        connection_account_config = end_station_config['ConnectionAccount']
        EndStationDataType.ConnectionAccount = get_ua_class('ConnectionAccountDataType')()
        if ('UserName' in connection_account_config):
            EndStationDataType.ConnectionAccount.UserName = connection_account_config['UserName']
        if ('Password' in connection_account_config):
            EndStationDataType.ConnectionAccount.Password = connection_account_config['Password']

    if ('NETCONF' in end_station_config):
        NETCONF_config = end_station_config['NETCONF']
        EndStationDataType.NETCONF = get_ua_class('NETCONFDataType')()
        if ('SSHPort' in NETCONF_config):
            EndStationDataType.NETCONF.SSHPort = NETCONF_config['SSHPort']
            
    if ('SNMP' in end_station_config):
        SNMP_config = end_station_config['SNMP']
        EndStationDataType.SNMP = get_ua_class('SNMPDataType')()
        if ('Port' in SNMP_config):
            EndStationDataType.SNMP.Port = SNMP_config['Port']
        if ('Version' in SNMP_config):
            EndStationDataType.SNMP.Version = SNMP_config['Version']
        if ('WriteCommunity' in SNMP_config):
            EndStationDataType.SNMP.WriteCommunity = SNMP_config['WriteCommunity']
        if ('ReadCommunity' in SNMP_config):
            EndStationDataType.SNMP.ReadCommunity = SNMP_config['ReadCommunity']

    if ('RESTful' in end_station_config):
        RESTful_config = end_station_config['RESTful']
        EndStationDataType.RESTful = get_ua_class('RESTfulDataType')()
        if ('Port' in RESTful_config):
            EndStationDataType.RESTful.Port = RESTful_config['Port']

    if ('IpSetting' in end_station_config):
        ip_setting_config = end_station_config['IpSetting']
        EndStationDataType.IpSetting = get_ua_class('IpSettingDataType')()
        if ('IpAddress' in ip_setting_config):
            EndStationDataType.IpSetting.IpAddress = ip_setting_config['IpAddress']
        if ('SubnetMask' in ip_setting_config):
            EndStationDataType.IpSetting.SubnetMask = ip_setting_config['SubnetMask']
        if ('Gateway' in ip_setting_config):
            EndStationDataType.IpSetting.Gateway = ip_setting_config['Gateway']
        if ('DNS1' in ip_setting_config):
            EndStationDataType.IpSetting.DNS1 = ip_setting_config['DNS1']
        if ('DNS2' in ip_setting_config):
            EndStationDataType.IpSetting.DNS2 = ip_setting_config['DNS2']

    if ('MacAddress' in end_station_config):
        EndStationDataType.MacAddress = end_station_config['MacAddress']

    if ('EthernetInterfaces' in end_station_config):
        for ethernet_interface_config in end_station_config['EthernetInterfaces']:
            EthernetInterfaceDataType = get_ua_class('EthernetInterfaceDataType')()
            if ('PhysAddress' in ethernet_interface_config):
                EthernetInterfaceDataType.PhysAddress = ethernet_interface_config['PhysAddress']
            EthernetInterfaceDataType.InterfaceName = ethernet_interface_config['InterfaceName']
            EndStationDataType.EthernetInterfaces.append(EthernetInterfaceDataType)

    try:
        ret = await call_method_full(client.get_node(f'{project_node}.Devices'),
                                    client.get_node(f'{project_node}.Devices.AddEndStation'),
                                    EndStationDataType)
        if (ret.OutputArguments[1] != 0):
            raise Exception("Error: " + ret.OutputArguments[2])

        return client.get_node(ret.OutputArguments[0])
    except:
        browse_name = end_station_config['IpSetting']['IpAddress']
        namespace = await get_namespace(client)
        end_station_node = await project_node.get_child([f'{namespace["ClassBased"]}:Devices',
                                                    f'{namespace["MoxaServer"]}:{browse_name}'])
        await set_end_station(client, end_station_node, end_station_config)
        return end_station_node

async def set_end_station(client, end_station_node, end_station_config):
    EndStationDataType = get_ua_class('EndStationDataType')()
    if ('Alias' in end_station_config):
        EndStationDataType.Alias = end_station_config['Alias']

    if ('DeviceProfileName' in end_station_config):
        EndStationDataType.DeviceProfileName = end_station_config['DeviceProfileName']

    if ('ConnectionAccount' in end_station_config):
        connection_account_config = end_station_config['ConnectionAccount']
        EndStationDataType.ConnectionAccount = get_ua_class('ConnectionAccountDataType')()
        if ('UserName' in connection_account_config):
            EndStationDataType.ConnectionAccount.UserName = connection_account_config['UserName']
        if ('Password' in connection_account_config):
            EndStationDataType.ConnectionAccount.Password = connection_account_config['Password']

    if ('NETCONF' in end_station_config):
        NETCONF_config = end_station_config['NETCONF']
        EndStationDataType.NETCONF = get_ua_class('NETCONFDataType')()
        if ('SSHPort' in NETCONF_config):
            EndStationDataType.NETCONF.SSHPort = NETCONF_config['SSHPort']
            
    if ('SNMP' in end_station_config):
        SNMP_config = end_station_config['SNMP']
        EndStationDataType.SNMP = get_ua_class('SNMPDataType')()
        if ('Port' in SNMP_config):
            EndStationDataType.SNMP.Port = SNMP_config['Port']
        if ('Version' in SNMP_config):
            EndStationDataType.SNMP.Version = SNMP_config['Version']
        if ('WriteCommunity' in SNMP_config):
            EndStationDataType.SNMP.WriteCommunity = SNMP_config['WriteCommunity']
        if ('ReadCommunity' in SNMP_config):
            EndStationDataType.SNMP.ReadCommunity = SNMP_config['ReadCommunity']

    if ('RESTful' in end_station_config):
        RESTful_config = end_station_config['RESTful']
        EndStationDataType.RESTful = get_ua_class('RESTfulDataType')()
        if ('Port' in RESTful_config):
            EndStationDataType.RESTful.Port = RESTful_config['Port']

    if ('IpSetting' in end_station_config):
        ip_setting_config = end_station_config['IpSetting']
        EndStationDataType.IpSetting = get_ua_class('IpSettingDataType')()
        if ('IpAddress' in ip_setting_config):
            EndStationDataType.IpSetting.IpAddress = ip_setting_config['IpAddress']
        if ('SubnetMask' in ip_setting_config):
            EndStationDataType.IpSetting.SubnetMask = ip_setting_config['SubnetMask']
        if ('Gateway' in ip_setting_config):
            EndStationDataType.IpSetting.Gateway = ip_setting_config['Gateway']
        if ('DNS1' in ip_setting_config):
            EndStationDataType.IpSetting.DNS1 = ip_setting_config['DNS1']
        if ('DNS2' in ip_setting_config):
            EndStationDataType.IpSetting.DNS2 = ip_setting_config['DNS2']

    if ('MacAddress' in end_station_config):
        EndStationDataType.MacAddress = end_station_config['MacAddress']

    if ('EthernetInterfaces' in end_station_config):
        for ethernet_interface_config in end_station_config['EthernetInterfaces']:
            EthernetInterfaceDataType = get_ua_class('EthernetInterfaceDataType')()
            if ('PhysAddress' in ethernet_interface_config):
                EthernetInterfaceDataType.PhysAddress = ethernet_interface_config['PhysAddress']
            EthernetInterfaceDataType.InterfaceName = ethernet_interface_config['InterfaceName']
            EndStationDataType.EthernetInterfaces.append(EthernetInterfaceDataType)

    ret = await call_method_full(end_station_node,
                                client.get_node(f'{end_station_node}.SetEndStationSetting'),
                                EndStationDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def clear_devices(client, project_node):
    devices_node = client.get_node(f'{project_node}.Devices')
    children = await devices_node.get_children(refs=ua.ObjectIds.HasComponent, nodeclassmask=ua.NodeClass.Object)
    for device in children:
        ret = await call_method_full(client.get_node(f'{project_node}.Devices'),
                                    client.get_node(f'{project_node}.Devices.RemoveDevice'),
                                    device.nodeid)
        if (ret.OutputArguments[0] != 0):
            raise Exception("Error: " + ret.OutputArguments[1])

async def add_link(client, links_node, link_config):
    LinkDataType = get_ua_class('LinkDataType')()
    try:
        LinkDataType.SourceIpAddress = link_config['SourceIpAddress']
        LinkDataType.SourceInterfaceName = link_config['SourceInterfaceName']
        LinkDataType.DestinationIpAddress = link_config['DestinationIpAddress']
        LinkDataType.DestinationInterfaceName = link_config['DestinationInterfaceName']
    except Exception as e:
        print("Parsing", e, "Invalid:", link_config)
        return

    ret = await call_method_full(links_node,
                            client.get_node(f'{links_node}.AddLink'),
                            LinkDataType)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))

    return client.get_node(ret.OutputArguments[0])


async def get_interface_node(client, bridge_node, interface_config):
    interface_id = interface_config['InterfaceId']
    return client.get_node(f'{bridge_node}.{interface_id}')

async def add_per_stream_priority(client, per_stream_priorities_node, per_stream_priority_config):
    PerStreamPriorityDataType = get_ua_class('PerStreamPriorityDataType')()
    PerStreamPriorityDataType.PerStreamPriorityMode = 0
    PerStreamPriorityDataType.EtherType = per_stream_priority_config['EtherType']
    if ('SubType' in per_stream_priority_config):
        PerStreamPriorityDataType.SubType = per_stream_priority_config['SubType']
    
    IeeeTsnVlanTagDataType = get_ua_class('IeeeTsnVlanTagDataType')()
    IeeeTsnVlanTagDataType.VlanId = per_stream_priority_config['VlanId']
    if ('PriorityCodePoint' in per_stream_priority_config):
        IeeeTsnVlanTagDataType.PriorityCodePoint = per_stream_priority_config['PriorityCodePoint']
    PerStreamPriorityDataType.VlanTag = IeeeTsnVlanTagDataType

    namespace = await get_namespace(client)
    ret = await call_method_full(per_stream_priorities_node,
                                await per_stream_priorities_node.get_child(f'{namespace["ClassBased"]}:AddPerStreamPriority'),
                                PerStreamPriorityDataType)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))

async def clear_per_stream_priorities(client, per_stream_priorities_node):
    namespace = await get_namespace(client)
    children = await per_stream_priorities_node.get_children(refs=ua.ObjectIds.HasComponent, nodeclassmask=ua.NodeClass.Object)
    for per_stream_priority_node in children:
        ret = await call_method_full(per_stream_priorities_node,
                                    await per_stream_priorities_node.get_child(f'{namespace["ClassBased"]}:RemovePerStreamPriority'),
                                    per_stream_priority_node.nodeid)
        if (ret.OutputArguments[0] != 0):
            raise Exception("Error: " + ret.OutputArguments[1])

async def set_ieee_dot1as_2011_port(client, time_sync_port_node, time_sync_port_config):
    IeeeDot1AS2011PortDataType = get_ua_class('IeeeDot1AS2011PortDataType')()
    if ('Active' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.Active = time_sync_port_config['Active']
    if ('AnnounceInterval' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.AnnounceInterval = time_sync_port_config['AnnounceInterval']
    if ('AnnounceReceiptTimeout' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.AnnounceReceiptTimeout = time_sync_port_config['AnnounceReceiptTimeout']
    if ('SyncInterval' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.SyncInterval = time_sync_port_config['SyncInterval']
    if ('SyncReceiptTimeout' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.SyncReceiptTimeout = time_sync_port_config['SyncReceiptTimeout']
    if ('PdelayRequestInterval' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.PdelayRequestInterval = time_sync_port_config['PdelayRequestInterval']
    if ('NeighborPropagationDelayThreshold' in time_sync_port_config):
        IeeeDot1AS2011PortDataType.NeighborPropagationDelayThreshold = time_sync_port_config['NeighborPropagationDelayThreshold']

    ret = await call_method_full(time_sync_port_node,
                                client.get_node(f'{time_sync_port_node}.SetIeeeDot1AS2011Port'),
                                IeeeDot1AS2011PortDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in ret.OutputArguments[1]:
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + ret.OutputArguments[1])

    return client.get_node(ret.OutputArguments[0])

async def insert_time_aware_shaper(client, time_aware_shaper_node, gate_control_config):
    GateControlDataType = get_ua_class('GateControlDataType')()
    GateControlDataType.Index = gate_control_config['Index']
    if ('Interval' in gate_control_config):
        GateControlDataType.Interval = gate_control_config['Interval']
    GateControlDataType.Queue0 = gate_control_config['Queue0']
    GateControlDataType.Queue1 = gate_control_config['Queue1']
    GateControlDataType.Queue2 = gate_control_config['Queue2']
    GateControlDataType.Queue3 = gate_control_config['Queue3']
    GateControlDataType.Queue4 = gate_control_config['Queue4']
    GateControlDataType.Queue5 = gate_control_config['Queue5']
    GateControlDataType.Queue6 = gate_control_config['Queue6']
    GateControlDataType.Queue7 = gate_control_config['Queue7']

    namespace = await get_namespace(client)
    ret = await call_method_full(time_aware_shaper_node,
                                await time_aware_shaper_node.get_child(f'{namespace["ClassBased"]}:InsertGateControl'),
                                GateControlDataType)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))

async def clear_time_aware_shaper(client, time_aware_shaper_node):
    namespace = await get_namespace(client)
    children = await time_aware_shaper_node.get_children(refs=ua.ObjectIds.HasComponent, nodeclassmask=ua.NodeClass.Object)
    for _ in children:
        ret = await call_method_full(time_aware_shaper_node,
                                    await time_aware_shaper_node.get_child(f'{namespace["ClassBased"]}:RemoveGateControl'),
                                    0)
        if (ret.OutputArguments[0] != 0):
            raise Exception("Error: " + ret.OutputArguments[1])

async def set_spanning_tree(client, spanning_tree_node, spanning_tree_config):
    SpanningTreeDataType = get_ua_class('SpanningTreeDataType')()
    if ('Active' in spanning_tree_config):
        SpanningTreeDataType.Active = spanning_tree_config['Active']
    if ('Compatibility' in spanning_tree_config):
        match spanning_tree_config['Compatibility']:
            case 'STP':
                SpanningTreeDataType.Compatibility = get_ua_class('SpanningTreeCompatibilityEnumType').STP
            case 'RSTP':
                SpanningTreeDataType.Compatibility = get_ua_class('SpanningTreeCompatibilityEnumType').RSTP
            case 'MSTP':
                SpanningTreeDataType.Compatibility = get_ua_class('SpanningTreeCompatibilityEnumType').MSTP
            case _:
                raise Exception("Invalid Compatibility Value: " + spanning_tree_config['Compatibility'])
    if ('BridgePriority' in spanning_tree_config):
        SpanningTreeDataType.BridgePriority = spanning_tree_config['BridgePriority']
    if ('ForwardDelayTime' in spanning_tree_config):
        SpanningTreeDataType.ForwardDelayTime = spanning_tree_config['ForwardDelayTime']
    if ('HelloTime' in spanning_tree_config):
        SpanningTreeDataType.HelloTime = spanning_tree_config['HelloTime']
    if ('MaxAge' in spanning_tree_config):
        SpanningTreeDataType.MaxAge = spanning_tree_config['MaxAge']

    ret = await call_method_full(spanning_tree_node,
                                client.get_node(f'{spanning_tree_node}.SetSpanningTree'),
                                SpanningTreeDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def set_spanning_tree_port(client, spanning_tree_port_node, spanning_tree_port_config):
    SpanningTreePortDataType = get_ua_class('SpanningTreePortDataType')()
    if ('BPDUFilter' in spanning_tree_port_config):
        SpanningTreePortDataType.BPDUFilter = spanning_tree_port_config['BPDUFilter']
    if ('Edge' in spanning_tree_port_config):
        match spanning_tree_port_config['Edge']:
            case 'Auto':
                SpanningTreePortDataType.Edge = get_ua_class('EdgeEnumType').Auto
            case 'Yes':
                SpanningTreePortDataType.Edge = get_ua_class('EdgeEnumType').Yes
            case 'No':
                SpanningTreePortDataType.Edge = get_ua_class('EdgeEnumType').No
            case _:
                raise Exception("Invalid Edge Value (Auto|Yes|No): " + spanning_tree_port_config['Edge'])
    if ('LinkType' in spanning_tree_port_config):
        match spanning_tree_port_config['LinkType']:
            case 'PointToPoint':
                SpanningTreePortDataType.LinkType = get_ua_class('LinkTypeEnumType').PointToPoint
            case 'Shared':
                SpanningTreePortDataType.LinkType = get_ua_class('LinkTypeEnumType').Shared
            case 'Auto':
                SpanningTreePortDataType.LinkType = get_ua_class('LinkTypeEnumType').Auto
            case _:
                raise Exception("Invalid LinkType Value (PointToPoint|Shared|Auto): " + spanning_tree_port_config['LinkType'])
    if ('PathCost' in spanning_tree_port_config):
        SpanningTreePortDataType.PathCost = spanning_tree_port_config['PathCost']
    if ('Priority' in spanning_tree_port_config):
        SpanningTreePortDataType.Priority = spanning_tree_port_config['Priority']

    ret = await call_method_full(spanning_tree_port_node,
                                client.get_node(f'{spanning_tree_port_node}.SetSpanningTreePort'),
                                SpanningTreePortDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def add_device_account(client, device_accounts_node, device_account_config):
    DeviceAccountDataType = get_ua_class('DeviceAccountDataType')()
    try:
        DeviceAccountDataType.UserName = device_account_config['UserName']
    except:
        raise Exception("Need to add \"UserName\" in \"DeviceAccount\"")
    if ('Password' in device_account_config):
        DeviceAccountDataType.Password = device_account_config['Password']
    if ('Authority' in device_account_config):
        match device_account_config['Authority']:
            case 'Admin':
                DeviceAccountDataType.Authority = get_ua_class('AuthorityEnumType').Admin
            case 'Supervisor':
                DeviceAccountDataType.Authority = get_ua_class('AuthorityEnumType').Supervisor
            case 'User':
                DeviceAccountDataType.Authority = get_ua_class('AuthorityEnumType').User
            case _:
                raise Exception("Invalid Authority Value (Admin|Supervisor|User): " + device_account_config['Authority'])
    if ('Email' in device_account_config):
        DeviceAccountDataType.Email = device_account_config['Email']

    namespace = await get_namespace(client)
    ret = await call_method_full(device_accounts_node,
                                await device_accounts_node.get_child(f'{namespace["ClassBased"]}:AddDeviceAccount'),
                                DeviceAccountDataType)
    if (ret.OutputArguments[1] != 0):
        if "already in use" in str(ret.OutputArguments[2]).lower() or "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))

async def clear_device_accounts(client, device_accounts_node):
    namespace = await get_namespace(client)
    children = await device_accounts_node.get_children(refs=ua.ObjectIds.HasComponent, nodeclassmask=ua.NodeClass.Object)
    for device_account_node in children:
        try:
            ret = await call_method_full(device_accounts_node,
                                        await device_accounts_node.get_child(f'{namespace["ClassBased"]}:RemoveDeviceAccount'),
                                        device_account_node.nodeid)
            if (ret.OutputArguments[0] != 0):
                if "The device account table cannot be empty" in ret.OutputArguments[1]:
                    continue
                raise Exception("Error: " + ret.OutputArguments[1])
        except Exception as e:
            if "The device account table cannot be empty" in str(e):
                continue
            raise e

async def set_management_interface(client, management_interface_node, management_interface_config):
    ManagementInterfaceDataType = get_ua_class('ManagementInterfaceDataType')()
    if ('HttpActive' in management_interface_config):
        ManagementInterfaceDataType.HttpActive = management_interface_config['HttpActive']
    if ('HttpTcpPort' in management_interface_config):
        ManagementInterfaceDataType.HttpTcpPort = management_interface_config['HttpTcpPort']
    if ('HttpsActive' in management_interface_config):
        ManagementInterfaceDataType.HttpsActive = management_interface_config['HttpsActive']
    if ('HttpsTcpPort' in management_interface_config):
        ManagementInterfaceDataType.HttpsTcpPort = management_interface_config['HttpsTcpPort']
    if ('TelnetActive' in management_interface_config):
        ManagementInterfaceDataType.TelnetActive = management_interface_config['TelnetActive']
    if ('TelnetTcpPort' in management_interface_config):
        ManagementInterfaceDataType.TelnetTcpPort = management_interface_config['TelnetTcpPort']
    if ('SshActive' in management_interface_config):
        ManagementInterfaceDataType.SshActive = management_interface_config['SshActive']
    if ('SshTcpPort' in management_interface_config):
        ManagementInterfaceDataType.SshTcpPort = management_interface_config['SshTcpPort']
    if ('SnmpActive' in management_interface_config):
        match management_interface_config['SnmpActive']:
            case 'Enabled':
                ManagementInterfaceDataType.SnmpActive = get_ua_class('ActiveSnmpEnumType').Enabled
            case 'Disabled':
                ManagementInterfaceDataType.SnmpActive = get_ua_class('ActiveSnmpEnumType').Disabled
            case 'ReadOnly':
                ManagementInterfaceDataType.SnmpActive = get_ua_class('ActiveSnmpEnumType').ReadOnly
            case _:
                raise Exception("Invalid SNMPActive value: " + management_interface_config['SnmpActive'])
    if ('SnmpTransportProtocol' in management_interface_config):
        match management_interface_config['SnmpTransportProtocol']:
            case 'UDP':
                ManagementInterfaceDataType.SnmpTransportProtocol = get_ua_class('TransportProtocol').UDP
            case 'TCP':
                ManagementInterfaceDataType.SnmpTransportProtocol = get_ua_class('TransportProtocol').TCP
            case _:
                raise Exception("Invalid SnmpTransportProtocol value: " + management_interface_config['SnmpTransportProtocol'])
    if ('SnmpPort' in management_interface_config):
        ManagementInterfaceDataType.SnmpPort = management_interface_config['SnmpPort']
    if ('NumberOfHttpAndHttpsLoginSessions' in management_interface_config):
        ManagementInterfaceDataType.NumberOfHttpAndHttpsLoginSessions = management_interface_config['NumberOfHttpAndHttpsLoginSessions']
    if ('NumberOfTelnetAndSshLoginSessions' in management_interface_config):
        ManagementInterfaceDataType.NumberOfTelnetAndSshLoginSessions = management_interface_config['NumberOfTelnetAndSshLoginSessions']

    ret = await call_method_full(management_interface_node,
                        client.get_node(f'{management_interface_node}.SetManagementInterface'),
                        ManagementInterfaceDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def set_time_sync(client, time_sync_node, time_sync_config):
    TimeSyncDataType = get_ua_class('TimeSyncDataType')()
    if ('Active' in time_sync_config):
        TimeSyncDataType.Active = time_sync_config['Active']
    if ('Profile' in time_sync_config):
        match time_sync_config['Profile']:
            case 'IEEE_802Dot1AS_2011':
                TimeSyncDataType.Profile = get_ua_class('ProfileEnumType').IEEE_802Dot1AS_2011
            case 'IEEE_1588_2008':
                TimeSyncDataType.Profile = get_ua_class('ProfileEnumType').IEEE_1588_2008
            case 'IEC_61850_2016':
                TimeSyncDataType.Profile = get_ua_class('ProfileEnumType').IEC_61850_2016
            case 'IEEE_C37Dot238_2017':
                TimeSyncDataType.Profile = get_ua_class('ProfileEnumType').IEEE_C37Dot238_2017
            case _:
                raise Exception("Invalid time sync profile value: " + time_sync_config['Profile'])
    if ('IeeeDot1AS2011' in time_sync_config):
        ieee_dot1as_2011_config = time_sync_config['IeeeDot1AS2011']
        TimeSyncDataType.IeeeDot1AS2011 = get_ua_class('IeeeDot1AS2011DataType')()
        if ('Priority1' in ieee_dot1as_2011_config):
            TimeSyncDataType.IeeeDot1AS2011.Priority1 = ieee_dot1as_2011_config['Priority1']
        if ('Priority2' in ieee_dot1as_2011_config):
            TimeSyncDataType.IeeeDot1AS2011.Priority2 = ieee_dot1as_2011_config['Priority2']
        if ('AccuracyAlert' in ieee_dot1as_2011_config):
            TimeSyncDataType.IeeeDot1AS2011.AccuracyAlert = ieee_dot1as_2011_config['AccuracyAlert']

    ret = await call_method_full(time_sync_node,
                        client.get_node(f'{time_sync_node}.SetTimeSync'),
                        TimeSyncDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def set_vlan_port(client, vlan_port_node, vlan_port_config):
    VlanPortDataType = get_ua_class('VlanPortDataType')()
    if 'Mode' in vlan_port_config:
        match vlan_port_config['Mode']:
            case 'Access':
                VlanPortDataType.Mode = get_ua_class('PortModeEnumType').Access
            case 'Trunk':
                VlanPortDataType.Mode = get_ua_class('PortModeEnumType').Trunk
            case 'Hybrid':
                VlanPortDataType.Mode = get_ua_class('PortModeEnumType').Hybrid
            case _:
                raise Exception("Invalid Port Mode: " + vlan_port_config['Mode'])
    
    if 'Pvid' in vlan_port_config:
        VlanPortDataType.Pvid = vlan_port_config['Pvid']
    if 'UntaggedVlan' in vlan_port_config:
        VlanPortDataType.UntaggedVlan = vlan_port_config['UntaggedVlan']
    if 'TaggedVlan' in vlan_port_config:
        VlanPortDataType.TaggedVlan = vlan_port_config['TaggedVlan']

    namespace = await get_namespace(client)
    ret = await call_method_full(vlan_port_node,
                        await vlan_port_node.get_child(f'{namespace["ClassBased"]}:SetVlanPort'),
                        VlanPortDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def add_multicast_static_forward_table(client, multicast_static_forward_table_node, multicast_static_forward_config):
    MulticastStaticForwardDataType = get_ua_class('MulticastStaticForwardDataType')()
    MulticastStaticForwardDataType.VlanId = multicast_static_forward_config['VlanId']
    MulticastStaticForwardDataType.MacAddress = multicast_static_forward_config['MacAddress']
    if ('EgressPorts' in multicast_static_forward_config):
        MulticastStaticForwardDataType.EgressPorts = []
        for egress_port in multicast_static_forward_config['EgressPorts']:
            MulticastStaticForwardDataType.EgressPorts.append(egress_port)
    if ('ForbiddenEgressPorts' in multicast_static_forward_config):
        MulticastStaticForwardDataType.ForbiddenEgressPorts = []
        for forbidden_egress_port in multicast_static_forward_config['ForbiddenEgressPorts']:
            MulticastStaticForwardDataType.ForbiddenEgressPorts.append(forbidden_egress_port)

    namespace = await get_namespace(client)
    ret = await call_method_full(multicast_static_forward_table_node,
                        await multicast_static_forward_table_node.get_child(f'{namespace["ClassBased"]}:AddMulticastStaticForward'),
                        MulticastStaticForwardDataType)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))

async def clear_multicast_static_forward_table(client, multicast_static_forward_table_node):
    namespace = await get_namespace(client)
    children = await multicast_static_forward_table_node.get_children(refs=ua.ObjectIds.HasComponent, nodeclassmask=ua.NodeClass.Object)
    for multicast_static_forward_node in children:
        ret = await call_method_full(multicast_static_forward_table_node,
                                    await multicast_static_forward_table_node.get_child(f'{namespace["ClassBased"]}:RemoveMulticastStaticForward'),
                                    multicast_static_forward_node.nodeid)
        if (ret.OutputArguments[0] != 0):
            raise Exception("Error: " + ret.OutputArguments[1])

async def add_unicast_static_forward_table(client, unicast_static_forward_table_node, unicast_static_forward_config):
    UnicastStaticForwardDataType = get_ua_class('UnicastStaticForwardDataType')()
    UnicastStaticForwardDataType.VlanId = unicast_static_forward_config['VlanId']
    UnicastStaticForwardDataType.MacAddress = unicast_static_forward_config['MacAddress']
    if ('EgressPort' in unicast_static_forward_config):
        UnicastStaticForwardDataType.EgressPort = unicast_static_forward_config['EgressPort']

    namespace = await get_namespace(client)
    ret = await call_method_full(unicast_static_forward_table_node,
                        await unicast_static_forward_table_node.get_child(f'{namespace["ClassBased"]}:AddUnicastStaticForward'),
                        UnicastStaticForwardDataType)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))

async def clear_unicast_static_forward_table(client, unicast_static_forward_table_node):
    namespace = await get_namespace(client)
    children = await unicast_static_forward_table_node.get_children(refs=ua.ObjectIds.HasComponent, nodeclassmask=ua.NodeClass.Object)
    for unicast_static_forward_node in children:
        ret = await call_method_full(unicast_static_forward_table_node,
                                    await unicast_static_forward_table_node.get_child(f'{namespace["ClassBased"]}:RemoveUnicastStaticForward'),
                                    unicast_static_forward_node.nodeid)
        if (ret.OutputArguments[0] != 0):
            raise Exception("Error: " + ret.OutputArguments[1])

async def set_vlan(client, vlan_table_node, vlan_config):
    VlanDataType = get_ua_class('VlanDataType')()
    VlanDataType.VlanId = vlan_config['VlanId']
    if 'VlanName' in vlan_config:
        VlanDataType.VlanName = vlan_config['VlanName']
    if 'MemberPort' in vlan_config:
        VlanDataType.MemberPort = vlan_config['MemberPort']
    
    namespace = await get_namespace(client)
    ret = await call_method_full(vlan_table_node,
                        await vlan_table_node.get_child(f'{namespace["ClassBased"]}:SetVlan'),
                        VlanDataType)
    if (ret.OutputArguments[0] != 0):
        if "Warning" in str(ret.OutputArguments[1]):
            print(ret.OutputArguments[1])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[1]))

async def add_vlan(client, vlan_table_node, vlan_config):
    VlanDataType = get_ua_class('VlanDataType')()
    VlanDataType.VlanId = vlan_config['VlanId']
    if 'VlanName' in vlan_config:
        VlanDataType.VlanName = vlan_config['VlanName']
    if 'MemberPort' in vlan_config:
        VlanDataType.MemberPort = vlan_config['MemberPort']
    
    namespace = await get_namespace(client)
    ret = await call_method_full(vlan_table_node,
                        await vlan_table_node.get_child(f'{namespace["ClassBased"]}:AddVlan'),
                        VlanDataType)
    if (ret.OutputArguments[1] != 0):
        if "Warning" in str(ret.OutputArguments[2]):
            print(ret.OutputArguments[2])
        else:
            raise Exception("Error: " + str(ret.OutputArguments[2]))