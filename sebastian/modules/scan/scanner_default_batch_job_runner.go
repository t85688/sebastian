package scan

import (
	"context"
	"fmt"
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/semaphore"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/workerpool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/configmapper"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/topomanager"
)

type DefaultScannerBatchJobRunner struct {
	MaxConcurrencyTotalQuery     int
	MaxConcurrencyPerDeviceQuery int
	jobChan                      chan *DefaultScannerQueryJob
	semaphores                   map[int64]*semaphore.Semaphore
	actDeviceMap                 map[int64]*domain.Device
	deviceConfigMutex            sync.Mutex
	instanceMutex                sync.Mutex
	deviceConfigMap
}

type deviceConfigMap struct {
	UnicastStaticForwardTableMap   map[int64]*domain.StaticForwardTable
	ManagementInterfaceTableMap    map[int64]*domain.ManagementInterfaceTable
	TimeSyncTableMap               map[int64]*domain.TimeSyncTable
	TimeSettingTableMap            map[int64]*domain.TimeSettingTable
	VlanTableMap                   map[int64]*domain.VlanTable
	RstpTableMap                   map[int64]*domain.RstpTable
	LoopProtectionTableMap         map[int64]*domain.LoopProtection
	PortSettingTableMap            map[int64]*domain.PortSettingTable
	MulticastStaticForwardTableMap map[int64]*domain.StaticForwardTable
	NetworkSettingTableMap         map[int64]*domain.NetworkSettingTable
	PortDefaultPCPTableMap         map[int64]*domain.PortDefaultPCPTable
	SnmpTrapSettingTableMap        map[int64]*domain.SnmpTrapSettingTable
	StreamPriorityEgressTableMap   map[int64]*domain.StreamPriorityEgressTable
	StreamPriorityIngressTableMap  map[int64]*domain.StreamPriorityIngressTable
	InformationSettingTableMap     map[int64]*domain.InformationSettingTable
	LoginPolicyTableMap            map[int64]*domain.LoginPolicyTable
	UserAccountTableMap            map[int64]*domain.UserAccountTable
	SyslogSettingTableMap          map[int64]*domain.SyslogSettingTable
	GCLTableMap                    map[int64]*domain.GCLTable
	CBTableMap                     map[int64]*domain.CbTable
}

type DefaultScannerQueryJob struct {
	ActDeviceID int64
	MafDeviceID string
	JobFunc     func() error
}

func NewDefaultScannerBatchJobRunner(maxConcurrencyTotalQuery int, maxConcurrencyPerDeviceQuery int, actDeviceList map[int64]*domain.Device) *DefaultScannerBatchJobRunner {
	instance := &DefaultScannerBatchJobRunner{
		actDeviceMap:                 actDeviceList,
		MaxConcurrencyTotalQuery:     maxConcurrencyTotalQuery,
		MaxConcurrencyPerDeviceQuery: maxConcurrencyPerDeviceQuery,
		jobChan:                      make(chan *DefaultScannerQueryJob),
		semaphores:                   make(map[int64]*semaphore.Semaphore),
		deviceConfigMap:              deviceConfigMap{},
	}
	return instance
}

func (runner *DefaultScannerBatchJobRunner) BatchQuery(
	ctx context.Context,
	projectId int64,
	onProgress func(ctx context.Context, projectId int64, progress int),
) (*domain.DeviceConfig, error) {
	runner.instanceMutex.Lock()
	defer runner.instanceMutex.Unlock()

	result := &domain.DeviceConfig{}

	if len(runner.actDeviceMap) == 0 {
		return result, nil
	}

	if runner.MaxConcurrencyTotalQuery <= 0 {
		return nil, fmt.Errorf("MaxConcurrencyTotalQuery should be greater than 0")
	}

	if runner.MaxConcurrencyPerDeviceQuery <= 0 {
		return nil, fmt.Errorf("MaxConcurrencyPerDeviceQuery should be greater than 0")
	}

	actDeviceManager, err := getActDeviceManager()
	if err != nil {
		return nil, err
	}

	// Per device semaphore
	semaphores := make(map[int64]*semaphore.Semaphore)
	for actDeviceID, _ := range runner.actDeviceMap {
		semaphores[actDeviceID] = semaphore.NewSemaphore(runner.MaxConcurrencyPerDeviceQuery)
	}
	runner.semaphores = semaphores

	// Setup DeviceConfigMap
	runner.UnicastStaticForwardTableMap = make(map[int64]*domain.StaticForwardTable)
	runner.ManagementInterfaceTableMap = make(map[int64]*domain.ManagementInterfaceTable)
	runner.TimeSyncTableMap = make(map[int64]*domain.TimeSyncTable)
	runner.TimeSettingTableMap = make(map[int64]*domain.TimeSettingTable)
	runner.VlanTableMap = make(map[int64]*domain.VlanTable)
	runner.RstpTableMap = make(map[int64]*domain.RstpTable)
	runner.LoopProtectionTableMap = make(map[int64]*domain.LoopProtection)
	runner.PortSettingTableMap = make(map[int64]*domain.PortSettingTable)
	runner.MulticastStaticForwardTableMap = make(map[int64]*domain.StaticForwardTable)
	runner.NetworkSettingTableMap = make(map[int64]*domain.NetworkSettingTable)
	runner.PortDefaultPCPTableMap = make(map[int64]*domain.PortDefaultPCPTable)
	runner.SnmpTrapSettingTableMap = make(map[int64]*domain.SnmpTrapSettingTable)
	runner.StreamPriorityEgressTableMap = make(map[int64]*domain.StreamPriorityEgressTable)
	runner.StreamPriorityIngressTableMap = make(map[int64]*domain.StreamPriorityIngressTable)
	runner.InformationSettingTableMap = make(map[int64]*domain.InformationSettingTable)
	runner.LoginPolicyTableMap = make(map[int64]*domain.LoginPolicyTable)
	runner.UserAccountTableMap = make(map[int64]*domain.UserAccountTable)
	runner.SyslogSettingTableMap = make(map[int64]*domain.SyslogSettingTable)
	runner.GCLTableMap = make(map[int64]*domain.GCLTable)
	runner.CBTableMap = make(map[int64]*domain.CbTable)

	// setup DeviceConfigMap
	runner.jobChan = make(chan *DefaultScannerQueryJob, runner.MaxConcurrencyTotalQuery*runner.MaxConcurrencyPerDeviceQuery*2)

	go runner.produceQueryJobs(ctx, actDeviceManager)
	onProgress(ctx, projectId, 75)

	// Add job in worker pool
	workerPool := workerpool.NewWorkerPoolWaitable(
		runner.MaxConcurrencyTotalQuery,
		runner.MaxConcurrencyTotalQuery*runner.MaxConcurrencyPerDeviceQuery,
	)

	for job := range runner.jobChan {
		sema, ok := runner.semaphores[job.ActDeviceID]
		if ok {
			sema.Acquire()
		} else {
			logger.Warnf("No semaphore found for device ID %d", job.ActDeviceID)
			continue
		}

		workerPool.AddJob(func() {
			defer sema.Release()
			err := job.JobFunc()
			if err != nil {
				logger.Warnf("Job for device ID %d failed: %v", job.ActDeviceID, err)
			}
		})

	}
	workerPool.WaitAndClose()
	onProgress(ctx, projectId, 85)

	result.UnicastStaticForwardTables = make(map[int64]domain.StaticForwardTable)
	for deviceID, val := range runner.UnicastStaticForwardTableMap {
		if val != nil {
			result.UnicastStaticForwardTables[deviceID] = *val
		}
	}

	result.ManagementInterfaceTables = make(map[int64]domain.ManagementInterfaceTable)
	for deviceID, val := range runner.ManagementInterfaceTableMap {
		if val != nil {
			result.ManagementInterfaceTables[deviceID] = *val
		}
	}

	result.TimeSyncTables = make(map[int64]domain.TimeSyncTable)
	for deviceID, val := range runner.TimeSyncTableMap {
		if val != nil {
			result.TimeSyncTables[deviceID] = *val
		}
	}

	result.TimeSettingTables = make(map[int64]domain.TimeSettingTable)
	for deviceID, val := range runner.TimeSettingTableMap {
		if val != nil {
			result.TimeSettingTables[deviceID] = *val
		}
	}

	result.VlanTables = make(map[int64]domain.VlanTable)
	for deviceID, val := range runner.VlanTableMap {
		if val != nil {
			result.VlanTables[deviceID] = *val
		}
	}

	result.RstpTables = make(map[int64]domain.RstpTable)
	for deviceID, val := range runner.RstpTableMap {
		if val != nil {
			result.RstpTables[deviceID] = *val
		}
	}

	result.LoopProtectionTables = make(map[int64]domain.LoopProtection)
	for deviceID, val := range runner.LoopProtectionTableMap {
		if val != nil {
			result.LoopProtectionTables[deviceID] = *val
		}
	}

	result.PortSettingTables = make(map[int64]domain.PortSettingTable)
	for deviceID, val := range runner.PortSettingTableMap {
		if val != nil {
			result.PortSettingTables[deviceID] = *val
		}
	}

	result.MulticastStaticForwardTables = make(map[int64]domain.StaticForwardTable)
	for deviceID, val := range runner.MulticastStaticForwardTableMap {
		if val != nil {
			result.MulticastStaticForwardTables[deviceID] = *val
		}
	}

	result.NetworkSettingTables = make(map[int64]domain.NetworkSettingTable)
	for deviceID, val := range runner.NetworkSettingTableMap {
		if val != nil {
			result.NetworkSettingTables[deviceID] = *val
		}
	}

	result.PortDefaultPCPTables = make(map[int64]domain.PortDefaultPCPTable)
	for deviceID, val := range runner.PortDefaultPCPTableMap {
		if val != nil {
			result.PortDefaultPCPTables[deviceID] = *val
		}
	}

	result.SnmpTrapSettingTables = make(map[int64]domain.SnmpTrapSettingTable)
	for deviceID, val := range runner.SnmpTrapSettingTableMap {
		if val != nil {
			result.SnmpTrapSettingTables[deviceID] = *val
		}
	}

	result.StreamPriorityEgressTables = make(map[int64]domain.StreamPriorityEgressTable)
	for deviceID, val := range runner.StreamPriorityEgressTableMap {
		if val != nil {
			result.StreamPriorityEgressTables[deviceID] = *val
		}
	}

	result.StreamPriorityIngressTables = make(map[int64]domain.StreamPriorityIngressTable)
	for deviceID, val := range runner.StreamPriorityIngressTableMap {
		if val != nil {
			result.StreamPriorityIngressTables[deviceID] = *val
		}
	}

	// InformationSettingTables
	result.InformationSettingTables = make(map[int64]domain.InformationSettingTable)
	for deviceID, val := range runner.InformationSettingTableMap {
		if val != nil {
			result.InformationSettingTables[deviceID] = *val
		}
	}

	// LoginPolicyTables
	result.LoginPolicyTables = make(map[int64]domain.LoginPolicyTable)
	for deviceID, val := range runner.LoginPolicyTableMap {
		if val != nil {
			result.LoginPolicyTables[deviceID] = *val
		}
	}

	// UserAccountTables
	result.UserAccountTables = make(map[int64]domain.UserAccountTable)
	for deviceID, val := range runner.UserAccountTableMap {
		if val != nil {
			result.UserAccountTables[deviceID] = *val
		}
	}

	// SyslogSettingTables
	result.SyslogSettingTables = make(map[int64]domain.SyslogSettingTable)
	for deviceID, val := range runner.SyslogSettingTableMap {
		if val != nil {
			result.SyslogSettingTables[deviceID] = *val
		}
	}

	// GCLTables
	result.GCLTables = make(map[int64]domain.GCLTable)
	for deviceID, val := range runner.GCLTableMap {
		if val != nil {
			result.GCLTables[deviceID] = *val
		}
	}

	// CBTables
	result.CBTables = make(map[int64]domain.CbTable)
	for deviceID, val := range runner.CBTableMap {
		if val != nil {
			result.CBTables[deviceID] = *val
		}
	}

	return result, nil
}

func (runner *DefaultScannerBatchJobRunner) produceQueryJobs(ctx context.Context, actDeviceManager topomanager.IActDeviceManager) {
	defer func() {
		close(runner.jobChan)
	}()

	// Produce query jobs
	for actDeviceID, actDevice := range runner.actDeviceMap {
		mafDeviceID, exists := actDeviceManager.GetMAFDeviceID(actDeviceID)
		if !exists {
			logger.Errorf("MAF Device ID not found for act device ID %d", actDeviceID)
			continue
		}

		newDefaultScannerQueryJob := func(jobFunc func() error) *DefaultScannerQueryJob {
			return &DefaultScannerQueryJob{
				ActDeviceID: actDeviceID,
				MafDeviceID: mafDeviceID,
				JobFunc:     jobFunc,
			}
		}

		queryUnicastStaticForwardingJobFunc := runner.makeQueryUnicastStaticForwardingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryUnicastStaticForwardingJobFunc)

		queryMulticastStaticForwardingJobFunc := runner.makeQueryMulticastStaticForwardingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryMulticastStaticForwardingJobFunc)

		queryMgmtInterfaceJobFunc := runner.makeQueryMgmtInterfaceSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryMgmtInterfaceJobFunc)

		queryTimeSyncJobFunc := runner.makeQueryTimeSyncSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryTimeSyncJobFunc)

		// TimeSetting (SystemTime + DaylightSaving)
		queryTimeSettingJobFunc := runner.makeQueryTimeSettingTablesJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryTimeSettingJobFunc)

		queryVLANJobFunc := runner.makeQueryVLANSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryVLANJobFunc)

		queryRSTPJobFunc := runner.makeQueryRSTPSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryRSTPJobFunc)

		queryLoopProtectionJobFunc := runner.makeQueryLoopProtectionSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryLoopProtectionJobFunc)

		queryPortSettingJobFunc := runner.makeQueryPortSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryPortSettingJobFunc)

		// Network settings (IP, GW, DNS)
		queryNetworkSettingJobFunc := runner.makeQueryNetworkSettingTablesJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryNetworkSettingJobFunc)

		// PCP default per port
		// workaround begin: provide port id list
		portIdMap := make(map[int]bool)
		for _, iface := range actDevice.Interfaces {
			portIdMap[iface.InterfaceId] = true
		}
		// workaround end

		queryPortDefaultPCPJobFunc := runner.makeQueryPortDefaultPCPTableJobFunc(actDeviceID, mafDeviceID, portIdMap)
		runner.jobChan <- newDefaultScannerQueryJob(queryPortDefaultPCPJobFunc)

		// SNMP Trap settings
		querySnmpTrapJobFunc := runner.makeQuerySnmpTrapSettingTablesJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(querySnmpTrapJobFunc)

		// Stream Priority (Ingress/Egress)
		queryPerStreamPriorityJobFunc := runner.makeQueryPerStreamPriorityJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryPerStreamPriorityJobFunc)

		// Information settings
		queryInformationSettingJobFunc := runner.makeQueryInformationSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryInformationSettingJobFunc)

		// Login policy
		queryLoginPolicyJobFunc := runner.makeQueryLoginPolicyJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryLoginPolicyJobFunc)

		// User accounts
		queryUserAccountJobFunc := runner.makeQueryUserAccountJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryUserAccountJobFunc)

		// Syslog settings
		querySyslogSettingJobFunc := runner.makeQuerySyslogSettingJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(querySyslogSettingJobFunc)

		// GCL
		queryGCLJobFunc := runner.makeQueryGCLJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryGCLJobFunc)

		// CB
		queryCBJobFunc := runner.makeQueryCBJobFunc(actDeviceID, mafDeviceID)
		runner.jobChan <- newDefaultScannerQueryJob(queryCBJobFunc)
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryUnicastStaticForwardingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		unicastStaticForwarding, err := GetUnicastStaticForwarding(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get unicast static forwarding for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actUnicastStaticForwarding, err := configmapper.MappingDeviceConfigUnicastStaticForwarding(unicastStaticForwarding, actDeviceID)

		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.UnicastStaticForwardTableMap[actDeviceID] = actUnicastStaticForwarding
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryMulticastStaticForwardingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		multicastStaticForwarding, err := GetMulticastStaticForwarding(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get multicast static forwarding for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actMulticastStaticForwarding, err := configmapper.MappingDeviceConfigMulticastStaticForwarding(actDeviceID, multicastStaticForwarding)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.MulticastStaticForwardTableMap[actDeviceID] = actMulticastStaticForwarding
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryMgmtInterfaceSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		mgmtInterfaceSetting, err := GetMgmtInterface(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get management interface setting for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actMgmtInterface, err := configmapper.MappingDeviceConfigMgmtInterfaceSetting(mgmtInterfaceSetting, actDeviceID)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.ManagementInterfaceTableMap[actDeviceID] = actMgmtInterface
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryTimeSyncSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		timeSyncSetting, err := GetTimeSyncSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get time sync setting for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actTimeSync, err := configmapper.MappingDeviceConfigTimeSyncSetting(timeSyncSetting, actDeviceID)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.TimeSyncTableMap[actDeviceID] = actTimeSync
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryTimeSettingTablesJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		systemTime, err := GetSystemTime(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get system time for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		daylightSaving, err := GetDaylightSaving(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get daylight saving for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actTimeSetting, err := configmapper.MappingTimeSettingTables(actDeviceID, systemTime, daylightSaving)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.TimeSettingTableMap[actDeviceID] = actTimeSetting
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryVLANSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		vlanSetting, err := GetVlanSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get VLAN setting for device %s: %v", mafDeviceID, err)
			return err
		}

		actVlan, err := configmapper.MappingDeviceConfigVLANSetting(vlanSetting, actDeviceID)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.VlanTableMap[actDeviceID] = actVlan
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryRSTPSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		rstpSetting, err := GetStpRstpSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get STP/RSTP setting for device %s: %v", mafDeviceID, err)
			return err
		}

		actRstp, err := configmapper.MappingDeviceConfigRSTPSetting(rstpSetting, actDeviceID)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.RstpTableMap[actDeviceID] = actRstp
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryLoopProtectionSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		loopProtectionSetting, err := GetLoopProtectionSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get loop protection setting for device %s: %v", mafDeviceID, err)
			return err
		}

		actLoop := configmapper.MappingDeviceConfigLoopProtectionSetting(loopProtectionSetting, actDeviceID)

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.LoopProtectionTableMap[actDeviceID] = actLoop
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryPortSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		portSetting, err := GetPortSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get port setting for maf device %s: %v", mafDeviceID, err)
			return err
		}

		actPort := configmapper.MappingDeviceConfigPortSetting(portSetting, actDeviceID)

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.PortSettingTableMap[actDeviceID] = actPort
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryNetworkSettingTablesJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		ethInterfaces, err := GetEthInterfaceList(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get eth interface list for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actNetwork, err := configmapper.MappingNetworkSettingTables(actDeviceID, ethInterfaces)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.NetworkSettingTableMap[actDeviceID] = actNetwork
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryPortDefaultPCPTableJobFunc(actDeviceID int64, mafDeviceID string, portIdMap map[int]bool) func() error {
	return func() error {
		pcpSetting, err := GetPcpSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get PCP setting for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actPCP, err := configmapper.MappingPcpSettingTables(actDeviceID, pcpSetting, portIdMap)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.PortDefaultPCPTableMap[actDeviceID] = actPCP
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQuerySnmpTrapSettingTablesJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		snmpTrap, err := GetSNMPTrapInfo(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get SNMP Trap for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actTrap, err := configmapper.MappingSnmpTrapSettingTables(actDeviceID, snmpTrap)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.SnmpTrapSettingTableMap[actDeviceID] = actTrap
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryPerStreamPriorityJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		perStreamPriority, err := GetPerStreamPriority(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get Per-Stream Priority for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		hasEgress := true
		hasIngress := true
		actEgress, actEgressErr := configmapper.MappingStreamPriorityEgressTable(actDeviceID, perStreamPriority)
		actIngress, actIngressErr := configmapper.MappingStreamPriorityIngressTables(actDeviceID, perStreamPriority)

		if actEgressErr != nil {
			hasEgress = false
			logger.Warnf("Mapping Stream Priority Egress Table failed for device ID %d: %v", actDeviceID, actEgressErr)
		}

		if actIngressErr != nil {
			hasIngress = false
			logger.Warnf("Mapping Stream Priority Ingress Table failed for device ID %d: %v", actDeviceID, actIngressErr)
		}

		if !hasEgress && !hasIngress {
			return nil
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()

		if hasEgress {
			runner.StreamPriorityEgressTableMap[actDeviceID] = actEgress
		}

		if hasIngress {
			runner.StreamPriorityIngressTableMap[actDeviceID] = actIngress
		}

		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryInformationSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		info, err := GetDeviceInformation(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get device information for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actInfo, err := configmapper.MappingInformationSetting(actDeviceID, info)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.InformationSettingTableMap[actDeviceID] = actInfo
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryLoginPolicyJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		policy, err := GetLoginPolicy(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get login policy for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actPolicy, err := configmapper.MappingLoginPolicy(actDeviceID, policy)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.LoginPolicyTableMap[actDeviceID] = actPolicy
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryUserAccountJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		accounts, err := GetUserAccounts(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get user accounts for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actAccounts, err := configmapper.MappingUserAccounts(actDeviceID, accounts)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.UserAccountTableMap[actDeviceID] = actAccounts
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQuerySyslogSettingJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		syslog, err := GetSyslogSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get syslog setting for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actSyslog, err := configmapper.MappingSyslogSetting(actDeviceID, syslog)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()
		runner.SyslogSettingTableMap[actDeviceID] = actSyslog
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryGCLJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		timeAwareShaperSetting, err := GetTimeAwareShaperSetting(mafDeviceID)
		if err != nil {
			logger.Errorf("Failed to get GCL setting for MAF device ID %s: %v", mafDeviceID, err)
			return err
		}

		actGCLTable, err := configmapper.MappingGCLSetting(actDeviceID, timeAwareShaperSetting)
		if err != nil {
			return err
		}

		runner.deviceConfigMutex.Lock()
		defer runner.deviceConfigMutex.Unlock()

		runner.GCLTableMap[actDeviceID] = actGCLTable
		return nil
	}
}

func (runner *DefaultScannerBatchJobRunner) makeQueryCBJobFunc(actDeviceID int64, mafDeviceID string) func() error {
	return func() error {
		return fmt.Errorf("CB table query not implemented yet")
	}
}
