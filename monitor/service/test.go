package main

import (
	"bufio"
	"fmt"
	"os/exec"
)

func main() {
	// Run tegrastats with --interval 1000 and --count 1
	cmd := exec.Command("tegrastats", "--interval", "1000", "--count", "1")

	// Get the output pipe
	stdout, err := cmd.StdoutPipe()
	if err != nil {
		fmt.Println("Error getting stdout pipe:", err)
		return
	}

	// Start the command
	if err := cmd.Start(); err != nil {
		fmt.Println("Error starting command:", err)
		return
	}

	// Create a scanner to read the output
	scanner := bufio.NewScanner(stdout)
	var stats_output string
	if scanner.Scan() {
		// Capture one line of output from tegrastats
		stats_output = scanner.Text()
		fmt.Println("Tegrastats output:", stats_output)
	}

	// Kill the tegrastats command after fetching the output
	if err := cmd.Process.Kill(); err != nil {
		fmt.Println("Error killing the command:", err)
		return
	}

	// Optionally, you can handle the output here or return it
	fmt.Println("Tegrastats output has been captured and command killed.")
}
