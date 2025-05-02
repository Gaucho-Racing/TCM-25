package main

import (
	"bufio"
	"bytes"
	"fmt"
	"os/exec"
	"strings"
)

var output string

func main() err {
	md := exec.Command("df", "-h", "/")
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

	output = fields[4]

	fmt.Println(output)
	
	return nil
}
