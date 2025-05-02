package main

import (
	"bytes"
	"bufio"
	"fmt"
	"os/exec"
	"regexp"
	"strings"
	"time"
	"strconv"
)

var CPU_util int        //Percent of CPU up-time in use
var GPU_util int        //Percent of GPU up-time in use
var memory_util int     //Percent of RAM space in use
var storage_util int    //Percent of hard drive space in use
var power_usage float32 //Average in mW (milliWatts), converted to centiWatts in MQTT compression
var CPU_temp float32    //CPU temp in Celsius
var GPU_temp float32    //GPU temp in Celsius
var stats_output string

/*

*/
func main() {
	InitializeResourceQuery()
}
 
/*
Initializes necessary setup and threaded functions for querying and parsing Jetson resource information.
*/
func InitializeResourceQuery() {
	go func() {
		for {
			err := QueryResourceStatistics()
			if err == nil {
				GetCPUUtil()
				GetGPUUtil()
				GetMemoryUtil()
				GetPowerUsage()
				GetCPUTemp()
				GetGPUTemp()
			} else {
				fmt.Println("Error running tegrastats:\n", err)
			}
			// PublishResources()
			TestPrintValues()
			time.Sleep(2 * time.Second)
		}
	}()
	for i := 0; i < 10; i++ {
		time.Sleep(3 * time.Second)
	}
}

/*
 */
func TestPrintValues() {
	fmt.Printf("CPU Usage: %d%%\nGPU Usage: %d%%\nMemory Usage: %d%%\nStorage Usage: %d%%\nPower Usage: %g centiWatts\nCPU Temp: %g C\nGPU Temp: %g C\n",
		CPU_util,
		GPU_util,
		memory_util,
		storage_util,
		power_usage,
		CPU_temp,
		GPU_temp)
}

/*
This is a helper function that keeps an input value between a min and max (inclusive).
This is used in PublishResources() to keep MQTT data at most 1 byte long.
*/
func clamp(val, min, max int) int {
	if val < min {
		return min
	}
	if val > max {
		return max
	}
	return val
}

/*
Queries the jetson nano for its hard drive utilization percentage and stores this in storage_util.
*/
func QueryStorageUtil() error {
	// Create the command
	cmd := exec.Command("df", "-h", "/")

	// Buffer for output
	var out bytes.Buffer
	cmd.Stdout = &out

	if err := cmd.Run(); err != nil {
		return fmt.Errorf("Failed to run df: %v", err)
	}

	lines := strings.Split(out.String(), "\n")
	if len(lines) < 2 {
		return fmt.Errorf("Unexpected df output: %v", out.String())
	}

	fields := strings.Fields(lines[1])
	if len(fields) < 5 {
		return fmt.Errorf("Unexpected df line format: %v", lines[1])
	}

	//fields[4] is the percentage of storage used
	storage_util, _ = strconv.Atoi(strings.Trim(fields[4], "%"))
	
	return nil
}

/*
Runs the shell command 'tegrastats' which outputs a large aggregate of CPU, GPU, and Memory statistics.
Then, queries the storage utilization.

Statistics are formatted according to:
https://docs.nvidia.com/jetson/archives/r35.1/DeveloperGuide/text/AT/JetsonLinuxDevelopmentTools/TegrastatsUtility.html
*/
func QueryResourceStatistics() error {
	// Run tegrastats with --interval 1000 and --count 1
	cmd := exec.Command("tegrastats", "--interval", "1000")

	// Get the output pipe
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		fmt.Println("Error getting stdout pipe:", err)
		return err
	}

	// Start the command
	if err := cmd.Start(); err != nil {
		fmt.Println("Error starting command:", err)
		return err
	}

	// Create a scanner to read the output
	scanner := bufio.NewScanner(stdout)
	if scanner.Scan() {
		// Capture one line of output from tegrastats
		stats_output = scanner.Text()
	}

	// Kill the tegrastats command after fetching the output
	if err := cmd.Process.Kill(); err != nil {
		fmt.Println("Error killing the command:", err)
		return err
	}
	
	err = QueryStorageUtil()
	if err != nil {
		return err
	}

	return nil
}

/*
Parses the resource statistics for CPU utilization and saves to CPU_util.
*/
func GetCPUUtil() error {
	re := regexp.MustCompile(`CPU \[(.*?)\]`)
	CPU_block_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_block_match) < 2 {
		return fmt.Errorf("CPU utilization block not found.")
	}

	CPU_block := CPU_block_match[1]
	CPU_re := regexp.MustCompile(`(\d+)%`)
	CPU_matches := CPU_re.FindAllStringSubmatch(CPU_block, -1)

	// var CPU_percents []int
	var count int = 0
	for _, match := range CPU_matches {
		var value int
		fmt.Sscanf(match[1], "%d", &value)
		count += 1
		CPU_util = ((count * CPU_util) + value) / (count + 1)
		// CPU_percents = append(CPU_percents, value)
	}

	return nil
}

/*
Parses the resource statistics for GPU utilization and saves to GPU_util.
*/
func GetGPUUtil() error {
	re := regexp.MustCompile(`GR3D_FREQ (\d+)%`)
	GPU_util_match := re.FindStringSubmatch(string(stats_output))
	if len(GPU_util_match) < 2 {
		return fmt.Errorf("GPU utilization (GR3D_FREQ) not found.")
	}

	fmt.Sscanf(GPU_util_match[1], "%d", &GPU_util)

	return nil
}

/*
Parses the resource statistics for RAM (memory) utilization and saves to memory_util.
*/
func GetMemoryUtil() error {
	re := regexp.MustCompile(`RAM (\d+)/(\d+)MB`)
	memory_util_matches := re.FindStringSubmatch(string(stats_output))
	if len(memory_util_matches) < 3 {
		return fmt.Errorf("RAM usage not found.")
	}

	var RAM_in_use, RAM_total int
	fmt.Sscanf(memory_util_matches[1], "%d", &RAM_in_use)
	fmt.Sscanf(memory_util_matches[2], "%d", &RAM_total)

	memory_util = int((RAM_in_use / RAM_total) * 100)

	return nil
}

/*
Parses the resource statistics for power usage statistics and saves to power_usage.
*/
func GetPowerUsage() error {
	re := regexp.MustCompile(`VDD_IN \d+/(\d+)`)
	power_usage_match := re.FindStringSubmatch(string(stats_output))
	if len(power_usage_match) < 2 {
		return fmt.Errorf("Power usage not found.")
	}

	fmt.Sscanf(power_usage_match[1], "%d", &power_usage)

	return nil
}

/*
Parses the resource statistics for CPU temperature in Celsius and saves to CPU_temp.
*/
func GetCPUTemp() error {
	re := regexp.MustCompile(`cpu@(\d+\.\d+)C`)
	CPU_temp_match := re.FindStringSubmatch(string(stats_output))
	if len(CPU_temp_match) < 2 {
		return fmt.Errorf("CPU temperature not found.")
	}

	fmt.Sscanf(CPU_temp_match[1], "%d", &CPU_temp)

	return nil
}

/*
Parses the resource statistics for GPU temperature in Celsius and saves to GPU_temp.
*/
func GetGPUTemp() error {
	re := regexp.MustCompile(`gpu@(\d+\.\d+)C`)
	GPU_temp_match := re.FindStringSubmatch(string(stats_output))
	if len(GPU_temp_match) < 2 {
		return fmt.Errorf("GPU temperature not found.")
	}

	fmt.Sscanf(GPU_temp_match[1], "%d", &GPU_temp)

	return nil
}
