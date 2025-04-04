package service

import (
	"fmt"
	"os/exec"
	"regexp"
	"strings"
	"time"
)

var CPU_util int
var GPU_util int
var memory_util int
var storage_util int
var power_usage float32
var CPU_temp float32
var GPU_temp float32
var stats_output

func InitializeResourceQuery() {

}

func QueryResourceStatistics(error) {
	tegrastats := exec.Command("tegrastats", "--interval", "1000", "--count", "1")
	stats_output, err := tegrastats.Output()
	if err != nil {
		return err
	}
}

func GetGPUUtil(error) {
	re := regexp.MustCompile(`GR3D_FREQ (\d+)%`)
	GPU_util_match := re.FindStringSubmatch(string(stats_output))
	if len(matches) < 2 {
		return fmt.Errorf("GPU utilization (GR3D_FREQ) not found.")
	}

	fmt.Sscanf(matches[1], "%d", &GPU_util)

	return nil
}

func GetCPUUtil(error) {
	re := regexp.MustCompile(`CPU \[(.*?)\]`)
	CPU_block_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_block_match) < 2 {
		return fmt.Errorf("CPU utilization block not found.")
	}

	CPU_block = CPU_block_match[1]
	CPU_re := regexp.MustCompile(`(\d+)%`)
	CPU_matches := GPU_re.FindAllStringSubmatch(CPU_block, -1)

	// var CPU_percents []int
	var count int = 0
	for _, match := range CPU_matches {
		var value int
		fmt.Sscanf(match[1], "%d", &value)
		count += 1
		CPU_util := ((count * CPU_util) + value) / (count + 1)
		// CPU_percents = append(CPU_percents, value)
	}

	return nil
}