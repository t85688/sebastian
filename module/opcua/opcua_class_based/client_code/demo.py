import json
import asyncio
import method
import inspect

# assign directory
configuration_path = './demo.json'

async def main():
    
    server_url = input('server url (default: opc.tcp://localhost:48010) = ') or 'opc.tcp://localhost:48010'
    username = input('username (default: admin)= ') or 'admin'
    password = input('password (default: moxa)= ') or 'moxa'
    try:
        client = await method.login(server_url, username, password)
    except Exception as e:
        print(e)
        return

    try:
        with open(configuration_path, 'r') as file:
            project_config = json.loads(file.read())

            # Cleanup existing project if any
            print("\nCleaning up existing project (if any)...")
            try:
                await method.remove_project(client, project_config)
            except Exception as e:
                print(f"Cleanup note: {e}")

            # Import device profile v1.0.14-7.40.2
            if ('DeviceProfiles' in project_config):
                print("\nImport device profile...")
                for device_profile_config in  project_config['DeviceProfiles']:
                    print(device_profile_config)
                    await method.import_device_profile(client, device_profile_config)

            if ('ProjectSetting' not in project_config or 'ProjectName' not in project_config['ProjectSetting']):
                print("Need to add \"ProjectName\" in \"ProjectSetting\"")
                return

            # Add project v1.0.14-7.2.1
            print("\nAdd new project...")
            print(project_config['ProjectSetting']['ProjectName'])
            project_node = await method.add_project(client, project_config)

            # Set project setting v1.0.14-7.4.1
            if ('ProjectSetting' in project_config):
                print("\nSet project setting...")
                print(project_config['ProjectSetting'])
                await method.set_project_setting(client, project_node, project_config)

            # Add bridge v1.0.14-7.8.1
            if ('Bridges' in project_config):
                for bridge_config in project_config['Bridges']:
                    print("\nAdd bridge device...")
                    print(bridge_config)
                    bridge_node = await method.add_bridge(client, project_node, bridge_config)

                    if ('EthernetInterfaces' in bridge_config):
                        for interface_config in bridge_config['EthernetInterfaces']:
                            interface_node = await method.get_interface_node(client, bridge_node, interface_config)

                            # Add per-stream priority v1.0.14-7.13.1
                            if ('PerStreamPriorities' in interface_config):
                                per_stream_priorities_node = client.get_node(f'{interface_node}.PerStreamPriorities')
                                await method.clear_per_stream_priorities(client, per_stream_priorities_node)
                                print("\nadd per-stream priority configs...")
                                for per_stream_priority_config in interface_config['PerStreamPriorities']:
                                    print(per_stream_priority_config)
                                    await method.add_per_stream_priority(client, per_stream_priorities_node, per_stream_priority_config)

                            # Set spanning tree port setting v1.0.14-7.31.1
                            if ('SpanningTreePort' in interface_config):
                                print("\nSet spanning tree port setting...")
                                print(interface_config['SpanningTreePort'])
                                spanning_tree_port_node = client.get_node(f'{interface_node}.SpanningTreePort')
                                await method.set_spanning_tree_port(client, spanning_tree_port_node, interface_config['SpanningTreePort'])

                            # Insert time aware shaper v1.0.14-7.15.1
                            if ('TimeAwareShaper' in interface_config):
                                time_aware_shaper_node = client.get_node(f'{interface_node}.TimeAwareShaper')
                                await method.clear_time_aware_shaper(client, time_aware_shaper_node)
                                
                                print("\nInsert time aware shaper...")
                                for gate_control_config in interface_config['TimeAwareShaper']:
                                    print(gate_control_config)
                                    await method.insert_time_aware_shaper(client, time_aware_shaper_node, gate_control_config)

                            # Set time sync port setting v1.0.14-7.23.1
                            if ('TimeSyncPort' in interface_config):
                                print("\nSet time sync port setting...")
                                print(interface_config['TimeSyncPort'])
                                time_sync_port_node = client.get_node(f'{interface_node}.TimeSyncPort')
                                await method.set_ieee_dot1as_2011_port(client, time_sync_port_node, interface_config['TimeSyncPort'])

                    if ('DeviceConfig' in bridge_config):
                        device_config = bridge_config['DeviceConfig']

                        # Add device account v1.0.14-7.28.1
                        if ('DeviceAccounts' in device_config):
                            device_accounts_node = client.get_node(f'{bridge_node}.DeviceConfig.DeviceAccounts')
                            await method.clear_device_accounts(client, device_accounts_node)

                            print("\nAdd device accounts...")
                            for device_account_config in device_config['DeviceAccounts']:
                                print(device_account_config)
                                await method.add_device_account(client, device_accounts_node, device_account_config)

                        # Set management interface v1.0.14-7.32.1
                        if ('ManagementInterface' in device_config):
                            print("\nSet management interface setting...")
                            print(device_config['ManagementInterface'])
                            management_interface_node = client.get_node(f'{bridge_node}.DeviceConfig.ManagementInterface')
                            await method.set_management_interface(client, management_interface_node, device_config['ManagementInterface'])

                        # Configure Ports and VLANs for Static Forward Tables
                        static_forward_tables = []
                        if 'MulticastStaticForwardTable' in device_config:
                            static_forward_tables.extend(device_config['MulticastStaticForwardTable'])
                        if 'UnicastStaticForwardTable' in device_config:
                            static_forward_tables.extend(device_config['UnicastStaticForwardTable'])

                        if static_forward_tables:
                            try:
                                # 1. Configure Ports
                                ports_to_config = set()
                                for config in static_forward_tables:
                                    if 'EgressPorts' in config:
                                        ports_to_config.update(config['EgressPorts'])
                                    if 'EgressPort' in config:
                                        ports_to_config.add(config['EgressPort'])
                                
                                for port_id in ports_to_config:
                                    print(f"\nConfiguring Port {port_id} to Trunk mode...")
                                    try:
                                        vlan_port_node = client.get_node(f'{bridge_node}.{port_id}.VlanPort')
                                        vlan_port_config = {'Mode': 'Trunk'}
                                        await method.set_vlan_port(client, vlan_port_node, vlan_port_config)
                                    except Exception as e:
                                        print(f"Warning: Failed to configure port {port_id}: {e}")

                                # 2. Add/Update VLANs
                                vlan_table_node = client.get_node(f'{bridge_node}.DeviceConfig.VlanSetting.VlanTable')
                                unique_vids = set(cfg['VlanId'] for cfg in static_forward_tables if 'VlanId' in cfg)
                                
                                for vid in unique_vids:
                                    # Collect all egress ports for this VLAN from all tables
                                    vlan_ports = set()
                                    for config in static_forward_tables:
                                        if config.get('VlanId') == vid:
                                            if 'EgressPorts' in config:
                                                vlan_ports.update(config['EgressPorts'])
                                            if 'EgressPort' in config:
                                                vlan_ports.add(config['EgressPort'])
                                    
                                    print(f"\nAdd/Update VLAN {vid} with ports {list(vlan_ports)}...")
                                    vlan_config = {'VlanId': vid, 'VlanName': f'vlan{vid}', 'MemberPort': list(vlan_ports)}
                                    try:
                                        await method.add_vlan(client, vlan_table_node, vlan_config)
                                    except Exception as e:
                                        if "is exist" in str(e):
                                            print(f"VLAN {vid} exists, updating...")
                                            await method.set_vlan(client, vlan_table_node, vlan_config)
                                        else:
                                            raise e

                            except Exception as e:
                                import traceback
                                traceback.print_exc()
                                print(f"Warning: Failed to configure VLANs/Ports: {e}")

                        # Add multicast static forward v1.0.14-7.26.1
                        if ('MulticastStaticForwardTable' in device_config):
                            multicast_static_forward_table_node = client.get_node(f'{bridge_node}.DeviceConfig.MulticastStaticForwardTable')
                            await method.clear_multicast_static_forward_table(client, multicast_static_forward_table_node)
                            print("\nAdd multicast static forward table...")
                            for multicast_static_forward_config in device_config['MulticastStaticForwardTable']:
                                print(multicast_static_forward_config)
                                await method.add_multicast_static_forward_table(client, multicast_static_forward_table_node, multicast_static_forward_config)

                        # Set spanning tree v1.0.14-7.30.1
                        if ('SpanningTree' in device_config):
                            print("\nSet spanning tree...")
                            print(device_config['SpanningTree'])
                            spanning_tree_node = client.get_node(f'{bridge_node}.DeviceConfig.SpanningTree')
                            await method.set_spanning_tree(client, spanning_tree_node, device_config['SpanningTree'])

                        # Set time sync v1.0.14-7.21.1
                        if ('TimeSync' in device_config):
                            print("\nSet time sync setting...")
                            print(device_config['TimeSync'])
                            time_sync_node = client.get_node(f'{bridge_node}.DeviceConfig.TimeSync')
                            await method.set_time_sync(client, time_sync_node, device_config['TimeSync'])

                        # Add unicast static forward v1.0.14-7.24.1
                        if ('UnicastStaticForwardTable' in device_config):
                            unicast_static_forward_table_node = client.get_node(f'{bridge_node}.DeviceConfig.UnicastStaticForwardTable')
                            await method.clear_unicast_static_forward_table(client, unicast_static_forward_table_node)
                            print("\nAdd unicast static forward table...")
                            for unicast_static_forward_config in device_config['UnicastStaticForwardTable']:
                                print(unicast_static_forward_config)
                                await method.add_unicast_static_forward_table(client, unicast_static_forward_table_node, unicast_static_forward_config)

            # Add end-station devices v1.0.14-7.8.3
            if ('EndStations' in project_config):
                for end_station_config in project_config['EndStations']:
                    print("\nAdd end-station devices...")
                    print(end_station_config)
                    await method.add_end_station(client, project_node, end_station_config)

            # Add links v1.0.14-7.36.1
            if ('Links' in project_config):
                print("\nAdd links...")
                links_node = client.get_node(f'{project_node}.Links')
                for link_config in project_config['Links']:
                    print(link_config)
                    await method.add_link(client, links_node, link_config)

            # Remove project
            print("\nRemove project...")
            await method.remove_project(client, project_config)

    except Exception as e:
        import traceback
        traceback.print_exc()
        print(e, '\n')

if __name__ == "__main__":
    asyncio.run(main())
