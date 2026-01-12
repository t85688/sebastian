import json
import asyncio
from asyncua import Client
import method

# assign directory
configuration_path = './offline_single_tsn_device.json'

async def main():
    server_url = input('server url (default: opc.tcp://localhost:48010) = ') or 'opc.tcp://localhost:48010'
    client = Client(url=f'{server_url}')

    # Authentication type
    authentication = input('authentication type (1: username, 2: certificate) (default: username)= ')
    if(authentication == '2' or authentication == "certificate"):
        cert = input('certificate path = ')
        key = input('private key path = ')
        try:
            await client.load_client_certificate(cert)
            await client.load_private_key(key)
        except:
            print("Load certificate failed")
            return
    else:
        username = input('username (default: admin)= ') or 'admin'
        password = input('password (default: moxa)= ') or 'moxa'
        client.set_user(username)
        client.set_password(password)

    try:
        await client.connect()
    except:
        print(f"Connect {server_url} failed")
        return

    try:
        await client.load_type_definitions()
    except:
        print("Load type definitions failed")
        return


    # get server namespace information
    namespace = {'OpcFoundation': await client.get_namespace_index('http://opcfoundation.org/UA/'),\
                'OpcuaServer': await client.get_namespace_index('urn:Moxa:OpcUaServer'),\
                'OpcuaBnm': await client.get_namespace_index('http://opcfoundation.org/UA/BNM/'),\
                'ClassBased': await client.get_namespace_index('http://www.moxa.com/auto-configuration-tool-class-based-model/'),\
                'MoxaServer': await client.get_namespace_index('http://www.moxa.com/UA/')}

    tsn_profile_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:TsnDeviceProfiles'])

    tsn_project_folder = await client.get_root_node().get_child([f'{namespace["OpcFoundation"]}:Objects',
                                                        f'{namespace["OpcFoundation"]}:Server',
                                                        f'{namespace["OpcuaBnm"]}:Resources',
                                                        f'{namespace["OpcuaBnm"]}:Communication',
                                                        f'{namespace["ClassBased"]}:TsnProjects'])

    # open configuration file
    try:
        with open(configuration_path, 'r') as file:
            configuration = json.loads(file.read())


            tsn_project_name = configuration['TsnProjectSetting']['TsnProjectName']

            # Remove project v1.0.13-8.20.2
            print(f"\nStep 1 [v1.0.13-8.20.2]: Remove {tsn_project_name} project...")
            ret = await method.remove_tsn_project(namespace, tsn_project_name, tsn_project_folder)
            if (ret.OutputArguments[0] != 0):
                print(ret.OutputArguments[1])

            # Import the device profiles
            import_device_profiles = configuration['ImportDeviceProfiles']
            for device_profile in import_device_profiles:
                # Get Filename and NodeName from configuration file
                device_profile_filename = device_profile['Filename']
                device_profile_nodename = device_profile['NodeName']
                                
                # Check profile exist or not v1.0.13-8.39.1
                print(f"\nStep 2 [v1.0.13-8.39.1]: Check customized device profile {device_profile_nodename} exist or not...")
                ret = await method.check_device_profile(namespace, device_profile_nodename, tsn_profile_folder)
                if (ret.OutputArguments[0] == True):
                    # Remove device profile v1.0.13-8.39.3
                    print(f"\nStep 2.1 [v1.0.13-8.39.3]: Remove device profile node {device_profile_nodename}...")
                    
                    # Get the children of the tsn_profile_folder node
                    children = await tsn_profile_folder.get_children()
                    for child in children:
                        # Get the display name of the node
                        child_display_name = await child.read_display_name()

                        # Check if the display name matches the device profile node name
                        if child_display_name.Text == device_profile_nodename:
                            ret = await method.remove_device_profile(namespace, child.nodeid, tsn_profile_folder)
                            if (ret.OutputArguments[0] != 0):
                                return print(ret.OutputArguments[1])
                            break

                try:
                    # Import device profile v1.0.13-8.39.2
                    print(f"\nStep 3 [v1.0.13-8.39.2]: Import device profile {device_profile_filename}...")
                    ret = await method.import_device_profile(namespace, device_profile_filename, tsn_profile_folder)
                    if (ret.OutputArguments[1] != 0):
                        return print(ret.OutputArguments[2])
                except FileNotFoundError:
                    return print(f'Cannot open device profile {tsn_device_profile_filename}')

            # Check the builtin device profiles exist
            builtin_device_profiles = configuration['BuiltinDeviceProfiles']
            for device_profile in builtin_device_profiles:
                # Get NodeName from configuration file
                device_profile_nodename = device_profile['NodeName']
                                
                # Check profile exist or not v1.0.13-8.39.1
                print(f"\nStep 4 [v1.0.13-8.39.1]: Check device profile {device_profile_nodename} exist or not...")
                ret = await method.check_device_profile(namespace, device_profile_nodename, tsn_profile_folder)
                if (ret.OutputArguments[0] == False):
                    # Print that the built device profile is not exist
                    return print(f"\nDevice profile {device_profile_nodename} does not exist.")


            # Add project v1.0.13-8.20.1
            print(f"\nStep 5 [v1.0.13-8.20.1]: Add {tsn_project_name} project...")
            ret = await method.add_tsn_project(namespace, tsn_project_name, tsn_project_folder)
            if (ret.OutputArguments[1] != 0):
                return print(ret.OutputArguments[2])
            print(ret.OutputArguments[0])

            # Get new project node
            tsn_project_node = client.get_node(ret.OutputArguments[0])

            # Set project setting v1.0.13-8.21.3
            print("\nStep 6 [v1.0.13-8.21.3]: Set project setting...")
            ret = await method.set_tsn_project_setting(namespace, configuration['TsnProjectSetting'], tsn_project_node)
            if (ret.OutputArguments[0] != 0):
                return print(ret.OutputArguments[1])

            # Add bridges v1.0.13-8.32.1
            print("\nStep 7 [v1.0.13-8.32.1]: Add bridges...")
            tsn_bridge_folder = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:TsnBridges')
            for tsnBridge in configuration['TsnBridges']:
                ret = await method.add_tsn_bridges(namespace, tsnBridge, tsn_bridge_folder)
                if (ret.OutputArguments[1] != 0):
                    return print(ret.OutputArguments[2])
                print(ret.OutputArguments[0])

            # Add end stations v1.0.13-8.30.1
            print("\nStep 8 [v1.0.13-8.30.1]: Add end stations...")
            tsn_end_station_folder = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:TsnEndStations')
            for tsnEndStation in configuration['TsnEndStations']:
                ret = await method.add_tsn_end_stations(namespace, tsnEndStation, tsn_end_station_folder)
                if (ret.OutputArguments[1] != 0):
                    return print(ret.OutputArguments[2])
                print(ret.OutputArguments[0])

    #         # Add bridged end stations v1.0.13-8.34.1
    #         print("\nAdd bridged end stations...")
    #         tsn_bridged_end_station_folder = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:TsnBridgedEndStations')
    #         for tsnBridgedEndStation in configuration['TsnBridgedEndStations']:
    #             ret = await method.add_tsn_bridged_end_stations(namespace, tsnBridgedEndStation, tsn_bridged_end_station_folder)
    #             if (ret.OutputArguments[1] != 0):
    #                 return print(ret.OutputArguments[2])

            # Add links v1.0.13-8.37.1
            print("\nStep 9 [v1.0.13-8.37.1]: Add links...")
            tsn_link_folder = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:TsnLinks')
            for tsnLink in configuration['TsnLinks']:
                ret = await method.add_tsn_links(namespace, tsnLink, tsn_link_folder)
                if (ret.OutputArguments[1] != 0):
                    return print(ret.OutputArguments[2])
                print(ret.OutputArguments[0])

            # Set scheduling v1.0.13-8.3.1
            print("\nStep 10 [v1.0.13-8.3.1]: Set scheduling...")
            scheduling_node = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:ClassBasedScheduling')
            time_slot_folder = await scheduling_node.get_child(f'{namespace["ClassBased"]}:TimeSlots')
            ret = await method.set_time_slot(namespace, configuration['TimeSlots'], time_slot_folder)
            if (ret.OutputArguments[0] != 0):
                return print(ret.OutputArguments[1])

            # Add streams v1.0.13-8.5.1
            print("\nStep 11 [v1.0.13-8.5.1]: Add streams...")
            for classBasedStream in configuration['TsnStreams']:
                timeSlotIndex = classBasedStream['TimeSlotIndex']
                time_slot_node = await time_slot_folder.get_child(f'{namespace["MoxaServer"]}:TimeSlot_{timeSlotIndex}')
                class_based_stream_folder = await time_slot_node.get_child(f'{namespace["ClassBased"]}:ClassBasedStreams')
                ret = await method.add_class_based_stream(namespace, classBasedStream, class_based_stream_folder)
                if (ret.OutputArguments[1] != 0):
                    return print(ret.OutputArguments[2])
                print(ret.OutputArguments[0])

            # Compute v1.0.13-8.18.2
            print("\nStep 12 [v1.0.13-8.18.2]: Computing...")
            compute_node = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:Compute')
            error_code = await method.compute_tsn_project(namespace, client, compute_node)
            if (error_code != 0):
                error_message = await client.get_node(f'{compute_node}.ErrorMessage').get_value()
                return print(error_message)

            # Get all stream compute result v1.0.13-8.21.2
            print("\nStep 13 [v1.0.13-8.21.2]: Get all stream compute results...")
            ret = await method.get_all_stream_computed_result(namespace, tsn_project_node)
            if (ret.OutputArguments[1] != 0):
                    return print(ret.OutputArguments[2])
            print(ret.OutputArguments[0])

            # Get stream compute result v1.0.13-8.21.1
            for stream_name in configuration['ComputedResult']:
                print("\nStep 14 [v1.0.13-8.21.1]: Get stream \"", stream_name ,"\" compute result...")
                ret = await method.get_stream_computed_result(namespace, stream_name, tsn_project_node)
                if (ret.OutputArguments[1] != 0):
                        return print(ret.OutputArguments[2])
                print(ret.OutputArguments[0])

            # Compare v1.0.13-8.18.3
            print("\nStep 15 [v1.0.13-8.18.3]: Comparing...")
    #         compare_node = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:Compare')
    #         error_code = await method.compare_tsn_project(namespace, client, compare_node)
    #         if (error_code != 0):
    #             error_message = await client.get_node(f'{compare_node}.ErrorMessage').get_value()
    #             return print(error_message)
            
            # Deploy v1.0.13-8.18.4
            print("\nStep 16[v1.0.13-8.18.4]: Deploying...")
            deploy_node = await tsn_project_node.get_child(f'{namespace["ClassBased"]}:Deploy')
            error_code = await method.deploy_tsn_project(namespace, client, deploy_node)
            if (error_code != 0):
                error_message = await client.get_node(f'{deploy_node}.ErrorMessage').get_value()
                return print(error_message)


    except FileNotFoundError:
        print(f'Cannot open file {configuration_path}')
    except Exception as ex:
        print(ex)

    finally:
        # Disconnect from the OPC UA server
        await client.disconnect()
        print("Disconnected from the OPC UA server.")

if __name__ == "__main__":
    asyncio.run(main())
