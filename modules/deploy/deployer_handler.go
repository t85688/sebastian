package deploy

import (
	"encoding/json"
	"fmt"
	"net"
	"sort"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	devicemanager "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager/job/model"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	dmschema "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

func (deployer *Deployer) clearMAFCache() error {
	netCache, err := dipool.GetNetCache()
	if err != nil {
		return err
	}

	netCache.DeleteAllDevices()
	netCache.DeleteAllLinks()

	return nil
}

func (deployer *Deployer) broadcastDeviceSearch(project domain.Project, deviceIds []int64, projectWithdeviceConfig domain.ProjectWithDeviceConfig) (map[int64]string, error) {
	deviceMappingMap := make(map[int64]string) // Map(ActDeviceId(int64), MafDeviceId(string))

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return deviceMappingMap, err
	}

	deployDeviceIdMap := make(map[int64]struct{}) // Map(DeviceId, null)
	for _, id := range deviceIds {
		deployDeviceIdMap[id] = struct{}{}
	}

	// BroadcastDiscovery
	_, err = dmManager.Search(dmschema.DeviceSearchInput{
		Method: devicemanager.BroadcastDiscovery,
	})

	// Mapping MAF devices & Chamberlain device
	// Generate the deviceMappingMap
	deviceMacIdMap := make(map[string]int64) // Map(Mac, DeviceId)

	logger.Info(fmt.Sprintf("DeviceConfig.MappingDeviceIpSettingTables size:%d", len(projectWithdeviceConfig.DeviceConfig.DeviceIpSettingTables)))

	// Get change IP device by MappingDeviceIpSettingTables from TopologyMapping
	changeDeviceIdMap := make(map[int64]struct{}) // Map(DeviceId, null)
	for id, table := range projectWithdeviceConfig.DeviceConfig.DeviceIpSettingTables {
		if _, ok := deployDeviceIdMap[id]; ok {
			mac := normalizeMAC(table.MacAddress)
			changeDeviceIdMap[id] = struct{}{}
			deviceMacIdMap[mac] = id
			logger.Info(fmt.Sprintf("MappingDeviceIpSettingTable: MAC: %v, ID: %v", table.MacAddress, id))
		}
	}

	mafDevices, err := dmManager.GetDevices(dmschema.GetDeviceListInput{})
	if err != nil {
		logger.Error("Error getting devices:", err)
		return deviceMappingMap, err
	}

	// Add the change Device to result
	uesdMafDeviceIdMap := make(map[string]struct{}) // Used MafDeviceIdMap(MafDeviceId, null)
	for _, mafDevice := range mafDevices {
		// call Qt api to create device
		deviceInfo := fmt.Sprintf("ID: %v, IP: %v, ModelName: %v, SerialNumber: %v, FirmwareVersion: %v, Location: %v, MAC: %v", mafDevice.DeviceId, mafDevice.IP, mafDevice.ModelName, mafDevice.SerialNumber, mafDevice.FirmwareVersion, mafDevice.Location, mafDevice.MAC)
		logger.Info(deviceInfo)

		if deviceId, ok := deviceMacIdMap[mafDevice.MAC]; ok {
			deviceMappingMap[deviceId] = mafDevice.DeviceId
			uesdMafDeviceIdMap[mafDevice.DeviceId] = struct{}{}
		}
	}

	// Add the not change Device to result
	noChangeDeviceIpIdMap := make(map[string]int64) // Map(Ip, DeviceId) for not changed IP device
	noChangeDeviceIdMap := make(map[int64]struct{}) // Map(DeviceId, null)

	for _, id := range deviceIds {
		if _, ok := changeDeviceIdMap[id]; !ok {
			noChangeDeviceIdMap[id] = struct{}{}
		}
	}

	logger.Info(fmt.Sprintf("changeDeviceIdMap size:%d", len(changeDeviceIdMap)))
	logger.Info(fmt.Sprintf("noChangeDeviceIdMap size:%d", len(noChangeDeviceIdMap)))

	for _, device := range project.Devices {
		if _, ok := noChangeDeviceIdMap[device.Id]; ok {
			noChangeDeviceIpIdMap[device.Ipv4.IpAddress] = device.Id
		}
	}

	logger.Info(fmt.Sprintf("noChangeDeviceIpIdMap size:%d", len(noChangeDeviceIpIdMap)))

	for _, mafDevice := range mafDevices {
		if _, ok := uesdMafDeviceIdMap[mafDevice.DeviceId]; !ok { // skip the used Maf DeviceId
			if deviceId, ok := noChangeDeviceIpIdMap[mafDevice.IP]; ok {
				deviceMappingMap[deviceId] = mafDevice.DeviceId
			}
		}
	}

	return deviceMappingMap, err
}

func (deployer *Deployer) ipListDeviceSearch(project domain.Project, deviceIds []int64) (map[int64]string, error) {
	deviceMappingMap := make(map[int64]string) // Map(ActDeviceId(int64), MafDeviceId(string))

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return deviceMappingMap, err
	}

	// Get device_ip
	var ipList []string
	deviceIdMap := make(map[int64]struct{}) // Map(DeviceId, null)
	deviceIpIdMap := make(map[string]int64) // Map(Ip, DeviceId)

	for _, id := range deviceIds {
		deviceIdMap[id] = struct{}{}
	}
	for _, device := range project.Devices {
		if _, ok := deviceIdMap[device.Id]; ok {
			ipList = append(ipList, device.Ipv4.IpAddress)
			deviceIpIdMap[device.Ipv4.IpAddress] = device.Id
		}
	}

	// IpListDiscovery
	productLines := []string{"switch"}
	_, err = dmManager.Search(dmschema.DeviceSearchInput{
		Method:      devicemanager.IpListDiscovery,
		ProductLine: &productLines,
		IPListOption: dmschema.IPListOption{
			IPList: &ipList,
		},
	})

	// Mapping MAF devices & Chamberlain device
	// Generate the deviceMappingMap
	mafDevices, err := dmManager.GetDevices(dmschema.GetDeviceListInput{})
	if err != nil {
		logger.Error("Error getting devices:", err)
		return deviceMappingMap, err
	}

	logger.Info("Device List:")
	for _, mafDevice := range mafDevices {
		// call Qt api to create device
		deviceInfo := fmt.Sprintf("ID: %v, IP: %v, ModelName: %v, SerialNumber: %v, FirmwareVersion: %v, Location: %v, MAC: %v", mafDevice.DeviceId, mafDevice.IP, mafDevice.ModelName, mafDevice.SerialNumber, mafDevice.FirmwareVersion, mafDevice.Location, mafDevice.MAC)
		logger.Info(deviceInfo)

		if deviceId, ok := deviceIpIdMap[mafDevice.IP]; ok {
			deviceMappingMap[deviceId] = mafDevice.DeviceId
		}
	}

	return deviceMappingMap, err
}

func (deployer *Deployer) checkDevicesExists(deviceIds []int64, deviceMappingMap map[int64]string, progress int64) {
	for _, id := range deviceIds {
		if _, ok := deviceMappingMap[id]; !ok {
			deployer.onDeviceResultProgressResponse(deployer.wsConnId, progress, DeployDeviceResult{
				DeviceId:     id,
				Status:       "Failed",
				ErrorMessage: "Not found",
			})
		}
	}
}

func (deployer *Deployer) setMAFDevicesSecret(project domain.Project, deviceIds []int64, deviceMappingMap map[int64]string) error {
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	for _, id := range deviceIds {
		if mafDeviceId, ok := deviceMappingMap[id]; ok {
			// SetDeviceSecrets(id string, secrets schema.DeviceSecrets) error

			device, err := getProjectDeviceById(project, id)
			if err != nil {
				return err
			}

			restfulPort := uint16(device.RestfulConfiguration.Port)
			dmManager.UpdateDeviceSecret(mafDeviceId, schema.DeviceSecrets{
				GlobalUsername: device.Account.Username,
				GlobalPassword: device.Account.Password,
				// Snmp: &schema.SNMP{
				// 	Version:         device.SnmpConfiguration.Version,
				// 	ReadCommunity:   device.SnmpConfiguration.ReadCommunity,
				// 	WriteCommunity:  device.SnmpConfiguration.WriteCommunity,
				// 	Username:        device.SnmpConfiguration.Username,
				// 	AuthType:        device.SnmpConfiguration.AuthenticationType,
				// 	AuthPassword:    device.SnmpConfiguration.DataEncryptionKey,
				// 	DataEncryptType: device.SnmpConfiguration.DataEncryptionType,
				// 	DataEncryptKey:  device.SnmpConfiguration.DataEncryptionKey,
				// 	Port:            device.SnmpConfiguration.Port,
				// 	// TransportProtocol: "TCP",
				// },
				Http: &schema.HTTP{
					// Username:
					// Password:
					Port: &restfulPort,
				},
				Https: &schema.HTTPS{
					Port: &restfulPort,
				},
				MoxaCmd: &schema.MOXACMD{
					// Username:
					// Password:
					Port: &restfulPort,
				},
			})
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}
	}

	return err
}

func (deployer *Deployer) enableDevicesSNMP(deviceIds []int64, deviceMappingMap map[int64]string) error {
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))
	logger.Info("Start enableDevicesSNMP")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	for _, id := range deviceIds {
		if mafDeviceId, ok := deviceMappingMap[id]; ok {
			enable := true
			_, code, err := dmManager.SetMgmtInterface(mafDeviceId, netdl.MgmtInterfaceSetting{
				Snmp: &netdl.Snmp{
					Enable: &enable,
				},
			})
			if code >= MafGenericErrorBase || err != nil {
				logger.Warn(fmt.Sprintf("Enable SNMP failed. Device: %d\n", id))
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}
	}

	return err
}

func (deployer *Deployer) waitForJobCompletion(jobId string, intervalSeconds int, timeoutSeconds int) error {
	logger.Info("Start waitForJobCompletion")

	timeout := time.After(time.Duration(timeoutSeconds) * time.Second)
	ticker := time.NewTicker(time.Duration(intervalSeconds) * time.Second)
	defer ticker.Stop()
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// local funct (deployer *Deployer)ion: check if all actions are completed, return stats
	allActionsCompleted := func(job *model.Job) (bool, int, int) {
		total := 0
		done := 0

		for _, tasks := range job.Pipeline {
			for _, task := range tasks {
				for _, action := range task.Actions {
					total++
					if action.CompletedAt != nil {
						done++
					}
				}
			}
		}
		return done == total, done, total - done
	}
	for {
		select {
		case <-timeout:
			err := fmt.Errorf("Job(%s) did not complete within timeout(%d sec)", jobId, timeoutSeconds)
			logger.Error(err)
			return err
		case <-ticker.C:
			job, code, err := dmManager.GetJobById(jobId)
			if code >= MafGenericErrorBase || err != nil {
				err := fmt.Errorf("Failed to get job: %w", err)
				logger.Error(err)
				return err
			}
			allDone, done, pending := allActionsCompleted(job)
			logger.Info(fmt.Sprintf("Job(%s) Checking: %d/%d actions completed (%d pending)", jobId, done, done+pending, pending))
			if allDone {
				logger.Info(fmt.Sprintf("Job(%s) All actions completed", jobId))
				return nil
			}
		}
		if deployer.stopFlag {
			return nil
		}
	}
}

func (deployer *Deployer) enableDevicesSNMPByJob(deviceIds []int64, deviceMappingMap map[int64]string) error {
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))
	logger.Info("Start enableDevicesSNMP")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// Create the config payload
	enable := true
	cfg := netdl.MgmtInterfaceSetting{
		Snmp: &netdl.Snmp{
			Enable: &enable,
		}}
	payload, _ := json.Marshal(cfg)

	// Create Tasks
	var tasks []devicemanager.TaskInput
	var index int
	for _, id := range deviceIds {
		if mafDeviceId, ok := deviceMappingMap[id]; ok {
			task := devicemanager.TaskInput{
				DeviceId: mafDeviceId,
				Actions: []devicemanager.ActionInput{
					{
						Action:  "setMgmtInterface",
						Payload: payload,
						RetryPolicy: &model.RetryPolicy{
							MaxAttempt: 2,
							Interval:   1,
						},
					},
				},
			}
			tasks = append(tasks, task)
			index = index + 1
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}
	}

	jobInput := devicemanager.JobInput{
		AutoParallel: true,
		ArpEnable:    deployer.deployArpEnable,
		Tasks:        tasks,
	}
	logger.Info(fmt.Sprintf("jobInput: %+v\n", jobInput))

	// Create Job
	job, code, err := dmManager.CreateJob(jobInput)
	if err != nil {
		logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
		return err
	}
	logger.Info(fmt.Sprintf("Job created: %+v\n", job))

	// Wait Job Complete
	err = deployer.waitForJobCompletion(job.JobId, 1, 180) // interval(1 sec), timeout(180 sec)
	if err != nil {
		return err
	}

	return nil
}

func (deployer *Deployer) rebootDevices(deviceIds []int64, deviceMappingMap map[int64]string) error {
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))
	logger.Info("Start rebootDevices")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// Create Tasks
	var tasks []devicemanager.TaskInput
	// var pipelineDeviceIds []string
	var index int
	for _, id := range deviceIds {
		if mafDeviceId, ok := deviceMappingMap[id]; ok {
			task := devicemanager.TaskInput{
				DeviceId: mafDeviceId,
				Actions: []devicemanager.ActionInput{
					{
						Action:  "reboot",
						Payload: []byte(`{}`),
						RetryPolicy: &model.RetryPolicy{
							MaxAttempt: 2,
							Interval:   1,
						},
					},
				},
			}

			tasks = append(tasks, task)
			index = index + 1
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}
	}

	jobInput := devicemanager.JobInput{
		AutoParallel: true,
		ArpEnable:    deployer.deployArpEnable,
		Tasks:        tasks,
	}
	logger.Info(fmt.Sprintf("jobInput: %+v\n", jobInput))
	job, code, err := dmManager.CreateJob(jobInput)
	if err != nil {
		logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
		return err
	}
	logger.Info(fmt.Sprintf("Job created: %+v\n", job))

	// Wait Job Complete
	err = deployer.waitForJobCompletion(job.JobId, 1, 60) // interval(1 sec), timeout(60 sec)
	if err != nil {
		return err
	}
	return err
}

func (deployer *Deployer) sortDevicesByDistance(deviceMappingMap map[int64]string) ([]int64, error) {
	logger.Info("Start sortDevicesByDistance")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return nil, err
	}

	var mafDevIdList []string
	for _, mafId := range deviceMappingMap {
		mafDevIdList = append(mafDevIdList, mafId)
	}
	reqDevices := model.SortRequest{
		Devices:   mafDevIdList,
		EnableArp: deployer.deployArpEnable,
	}

	// Sort by MAF DM
	sortResult, code, err := dmManager.GetSortDistance(reqDevices)
	if code >= MafGenericErrorBase || err != nil {
		return nil, err
	}
	logger.Info(fmt.Sprintf("MAF Sort Result: %v", sortResult))

	mergedMafDevicePath, err := layerMergeFarToNear(sortResult)
	if err != nil {
		return nil, err
	}

	// Reverse lookup from mafDevice to deviceId
	valToKey := make(map[string]int64)
	for k, v := range deviceMappingMap {
		valToKey[v] = k
	}
	var resultDeviceIdPath []int64
	for _, node := range mergedMafDevicePath {
		if deviceId, ok := valToKey[node]; ok {
			resultDeviceIdPath = append(resultDeviceIdPath, deviceId)
		}
	}

	return resultDeviceIdPath, err
}

func (deployer *Deployer) sortDevicesByIp(project domain.Project, deviceMappingMap map[int64]string) ([]int64, error) {
	logger.Info("Start sortDevicesByIp")

	type kv struct {
		deviceId int64
		ip       net.IP
	}
	var pairs []kv

	for deviceId, _ := range deviceMappingMap {
		device, err := getProjectDeviceById(project, deviceId)
		if err != nil {
			return nil, err
		}

		pairs = append(pairs, kv{deviceId, net.ParseIP(device.Ipv4.IpAddress)})
	}

	// Sort slice by IP (convert string to net.IP for correct numeric comparison)
	sort.Slice(pairs, func(i, j int) bool {
		return bytesCompare(pairs[i].ip, pairs[j].ip) < 0
	})

	// Extract sorted deviceIds into a list
	var sorted []int64
	for _, p := range pairs {
		sorted = append(sorted, p.deviceId)
	}

	return sorted, nil
}

func (deployer *Deployer) uploadDevicesOfflineConfig(deploySequenceByDeviceIds []int64, deviceMappingMap map[int64]string, deviceOfflineConfigMap map[int64]string) error {
	logger.Info("Start uploadDevicesOfflineConfig")
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))

	if len(deploySequenceByDeviceIds) == 0 {
		return fmt.Errorf("Deploy Sequence set is empty")
	}

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	if deployer.deployArpEnable {
		dmManager.AutoSetArp(true)
		defer dmManager.AutoSetArp(false)
	}

	minProgress := int64(30)
	maxProgress := int64(90)
	step := (maxProgress - minProgress) / int64(len(deploySequenceByDeviceIds))

	logger.Info(fmt.Sprintf("Deploy sequence: %v", deploySequenceByDeviceIds))
	for index, id := range deploySequenceByDeviceIds {
		if mafDeviceId, ok := deviceMappingMap[id]; ok {
			if deviceConfigFileId, ok := deviceOfflineConfigMap[id]; ok {
				progress := minProgress + step*int64(index+1)

				cfg := dmschema.ImportOfflineConfigInput{
					FileId: deviceConfigFileId,
				}
				_, code, err := dmManager.UploadOfflineConfig(mafDeviceId, cfg)
				if code >= MafGenericErrorBase || err != nil {
					deployer.onDeviceResultProgressResponse(deployer.wsConnId, progress, DeployDeviceResult{
						DeviceId:     id,
						Status:       "Failed",
						ErrorMessage: "Import Config failed",
					})
					errMsg := ""
					if err != nil {
						errMsg = err.Error()
					}
					logger.Info(fmt.Sprintf("Upload %v offline config failed(code:%v, err:%v)\n", id, code, errMsg))

				} else {
					deployer.onDeviceResultProgressResponse(deployer.wsConnId, progress, DeployDeviceResult{
						DeviceId:     id,
						Status:       "Success",
						ErrorMessage: "",
					})
					logger.Info(fmt.Sprintf("Upload %v offline config success\n", id))
				}
			} else {
				return fmt.Errorf("The Device(%v) not exists at the DeviceOfflineConfigMap", id)
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}

		if deployer.stopFlag {
			return nil
		}
	}

	return err
}

func (deployer *Deployer) uploadDevicesOfflineConfigByJob(deviceIds []int64, deviceMappingMap map[int64]string, deviceOfflineConfigMap map[int64]string) error {
	logger.Info("Start uploadDevicesOfflineConfig")
	// deviceMappingMap(ActDeviceId(int64), MafDeviceId(string))
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// Create Tasks
	var tasks []devicemanager.TaskInput
	var index int
	for _, id := range deviceIds {
		if mafDeviceId, ok := deviceMappingMap[id]; ok {
			if deviceConfigFileId, ok := deviceOfflineConfigMap[id]; ok {
				// Get offline config fileID (tag ID -> file ID)
				// Upload Offline Config
				cfg := dmschema.ImportOfflineConfigInput{
					FileId: deviceConfigFileId,
				}
				payload, _ := json.Marshal(cfg)
				task := devicemanager.TaskInput{
					DeviceId: mafDeviceId,
					Actions: []devicemanager.ActionInput{
						{
							Action:  "uploadOfflineConfig",
							Payload: payload,
							RetryPolicy: &model.RetryPolicy{
								MaxAttempt: 2,
								Interval:   1,
							},
						},
					},
				}
				tasks = append(tasks, task)
				index = index + 1
			} else {
				return fmt.Errorf("The Device(%v) not exists at the DeviceOfflineConfigMap", id)
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}
	}

	jobInput := devicemanager.JobInput{
		AutoParallel: true,
		ArpEnable:    deployer.deployArpEnable,
		Tasks:        tasks,
	}
	logger.Info(fmt.Sprintf("jobInput: %+v\n", jobInput))
	job, code, err := dmManager.CreateJob(jobInput)
	if code >= MafGenericErrorBase || err != nil {
		logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
		return err
	}

	logger.Info(fmt.Sprintf("Job created: %+v\n", job))
	// Get Job Status
	time.Sleep(3 * time.Second)
	job, _, err = dmManager.GetJobById(job.JobId) // checkout status and result fields inside Job structure.
	logger.Info(fmt.Sprintf("Job status(wait 3 second): %+v\n", job))

	return err
}

func (deployer *Deployer) syncDevicesUserAccount(project domain.Project, userAccountTables map[int64]domain.UserAccountTable, deviceIds []int64) error {
	var err error

	profiles_with_default_device_config, status := core.GetDeviceProfilesWithDefaultDeviceConfig()
	if !status.IsSuccess() {
		err := fmt.Errorf("Failed to get device profile with default device conig:%s", status.ErrorMessage)
		logger.Error(err)
		return err
	}

	for _, deviceId := range deviceIds {
		device, err := getProjectDeviceById(project, deviceId)
		if err != nil {
			return err
		}

		if userAccountTable, ok := userAccountTables[deviceId]; ok {
			// Sync the user-specified account to the device's connection
			syncUserName := userAccountTable.SyncConnectionAccount
			if syncUserAccount, ok := userAccountTable.Accounts[syncUserName]; ok {
				device.Account.DefaultSetting = false
				device.Account.Username = syncUserAccount.Username
				device.Account.Password = syncUserAccount.Password
			} else {
				err := fmt.Errorf("The specified connection account(%s) does not exist", syncUserName)
				logger.Error(err)
				return err
			}
		} else {
			var defaultConnectionAccount domain.Account
			deviceDefaultUserAccount, err := getDeviceDefaultAccount(profiles_with_default_device_config, device)
			if err != nil {
				logger.Error("Get the Device DefaultAccount(%s) failed", device.Id)
				return err
			}
			defaultConnectionAccount.Username = deviceDefaultUserAccount.Username
			defaultConnectionAccount.Password = deviceDefaultUserAccount.Password

			// Sync default account to the device's connection
			defaultConnectionAccount.DefaultSetting = false
			device.Account = defaultConnectionAccount
		}

		err = updateProjectDevice(&project, device)
		if err != nil {
			return err
		}
	}

	return err
}
