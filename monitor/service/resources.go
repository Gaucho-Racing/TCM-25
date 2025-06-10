package service

import (
	"encoding/json"
	"monitor/model"
	"monitor/utils"
	"os/exec"
	"time"
)

/*
Initializes necessary setup and threaded functions for querying and parsing Jetson resource information.
*/
func InitializeResourceQuery() {
	go func() {
		for {
			metrics, err := QueryResourceMetrics()
			if err != nil {
				utils.SugarLogger.Errorf("Error querying resource metrics: %v", err)
			} else {
				utils.SugarLogger.Infof("Resource metrics: %v", metrics)
				PublishResources(metrics)
			}
			time.Sleep(10 * time.Second)
		}
	}()
}

func QueryResourceMetrics() (model.ResourceMetrics, error) {
	out, err := exec.Command("python3", "jetson-stats.py").Output()
	if err != nil {
		utils.SugarLogger.Errorf("Failed to run jetson-stats.py: %v", err)
	}

	var metrics model.ResourceMetrics
	if err := json.Unmarshal(out, &metrics); err != nil {
		utils.SugarLogger.Errorf("Failed to parse JSON: %v", err)
	}
	return metrics, nil
}

func PublishResources(metrics model.ResourceMetrics) {
	return
}
