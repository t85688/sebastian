package common

import (
	"encoding/json"
	"fmt"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	devicemanager "gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/manager/job/model"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/netdl"
)

type MAFJobWorker struct {
	stopFlag bool
}

type IMAFJobWorker interface {
	EnableMAFDevicesSNMPByJob(mafDeviceIdList []string, arpEnable bool) error
	TriggerMAFDeviceLocatorByJob(mafDeviceIdList []string, duration int, arpEnable bool) error
	Stop()
}

func NewMAFJobWorker() IMAFJobWorker {
	return &MAFJobWorker{
		stopFlag: false,
	}
}

func (mafJobWorker *MAFJobWorker) Stop() {
	Logger.Info("Stop MAFJobWorker module")
	mafJobWorker.stopFlag = true
	return
}

func (mafJobWorker *MAFJobWorker) waitForJobCompletion(jobId string, intervalSeconds int, timeoutSeconds int) error {
	Logger.Info("Start waitForJobCompletion")

	timeout := time.After(time.Duration(timeoutSeconds) * time.Second)
	ticker := time.NewTicker(time.Duration(intervalSeconds) * time.Second)
	defer ticker.Stop()
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// local funct (topologyMapper *TopologyMapper)ion: check if all actions are completed, return stats
	allActionsCompleted := func(job *model.Job) (bool, int, int) {
		total := 0
		done := 0

		for _, tasks := range job.Pipeline {
			for _, task := range tasks {
				for _, action := range task.Actions {
					total++
					if action.CompletedAt != nil {
						done++

						// jsonBytes, _ := json.MarshalIndent(action, "", "  ")
						Logger.Info(fmt.Sprintf("MAF Complete Action: %+v\n", action))
						if action.Status != "success" {

						}
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
			Logger.Error(err)
			return err
		case <-ticker.C:
			job, code, err := dmManager.GetJobById(jobId)
			if code >= MafGenericErrorBase || err != nil {
				err := fmt.Errorf("Failed to get job: %w", err)
				Logger.Error(err)
				return err
			}
			allDone, done, pending := allActionsCompleted(job)
			Logger.Info(fmt.Sprintf("Job(%s) Checking: %d/%d actions completed (%d pending)", jobId, done, done+pending, pending))
			if allDone {
				Logger.Info(fmt.Sprintf("Job(%s) All actions completed", jobId))
				return nil
			}
		}
		if mafJobWorker.stopFlag {
			return nil
		}
	}
}

func (mafJobWorker *MAFJobWorker) EnableMAFDevicesSNMPByJob(mafDeviceIdList []string, arpEnable bool) error {
	Logger.Info("Start EnableMAFDevicesSNMPByJob")

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
	for _, id := range mafDeviceIdList {
		task := devicemanager.TaskInput{
			DeviceId: id,
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
	}

	jobInput := devicemanager.JobInput{
		AutoParallel: true,
		ArpEnable:    arpEnable,
		Tasks:        tasks,
	}
	jsonBytes, _ := json.MarshalIndent(jobInput, "", "  ")
	Logger.Info(fmt.Sprintf("jobInput: %+v\n", string(jsonBytes)))

	// Create Job
	job, code, err := dmManager.CreateJob(jobInput)
	if err != nil {
		Logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
		return err
	}
	Logger.Info(fmt.Sprintf("Job created: %+v\n", job))

	// Wait Job Complete
	err = mafJobWorker.waitForJobCompletion(job.JobId, 1, 180) // interval(1 sec), timeout(180 sec)
	if err != nil {
		return err
	}

	return nil
}

func (mafJobWorker *MAFJobWorker) TriggerMAFDeviceLocatorByJob(mafDeviceIdList []string, duration int, arpEnable bool) error {
	Logger.Info("Start TriggerMAFDeviceLocatorByJob")

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	// Create the config payload
	cfg := schema.SetLocatorInput{
		Enable:   true,
		Duration: duration,
	}
	payload, _ := json.Marshal(cfg)

	// Create Tasks
	var tasks []devicemanager.TaskInput
	for _, id := range mafDeviceIdList {
		task := devicemanager.TaskInput{
			DeviceId: id,
			Actions: []devicemanager.ActionInput{
				{
					Action:  "setLocator",
					Payload: payload,
					RetryPolicy: &model.RetryPolicy{
						MaxAttempt: 2,
						Interval:   1,
					},
				},
			},
		}
		tasks = append(tasks, task)
	}

	jobInput := devicemanager.JobInput{
		AutoParallel: true,
		ArpEnable:    arpEnable,
		Tasks:        tasks,
	}
	jsonBytes, _ := json.MarshalIndent(jobInput, "", "  ")
	Logger.Info(fmt.Sprintf("jobInput: %+v\n", string(jsonBytes)))

	// Create Job
	job, code, err := dmManager.CreateJob(jobInput)
	if err != nil {
		Logger.Error(fmt.Sprintf("CreateJob failed: %v (code=%d)\n", err, code))
		return err
	}
	Logger.Info(fmt.Sprintf("Job created: %+v\n", job))

	// Wait Job Complete
	err = mafJobWorker.waitForJobCompletion(job.JobId, 1, 180) // interval(1 sec), timeout(180 sec)
	if err != nil {
		return err
	}

	return nil
}
