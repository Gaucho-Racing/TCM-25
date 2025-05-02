package main

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"strings"
)

var output string

func main() {
	// Create the command
	cmd := exec.Command("df", "-h", "/")

	// Buffer for output
	var out bytes.Buffer
	cmd.Stdout = &out

	// Run the command
	if err := cmd.Run(); err != nil {
		fmt.Fprintf(os.Stderr, "Failed to run df: %v\n", err)
		os.Exit(1)
	}

	// Parse the output
	lines := strings.Split(out.String(), "\n")
	if len(lines) < 2 {
		fmt.Fprintf(os.Stderr, "Unexpected df output: %v\n", out.String())
		os.Exit(1)
	}

	fields := strings.Fields(lines[1])
	if len(fields) < 5 {
		fmt.Fprintf(os.Stderr, "Unexpected df line format: %v\n", lines[1])
		os.Exit(1)
	}

	output = fields[4] // This is usually the usage percentage

	fmt.Println("Disk usage:", output)
}
