package config

import "github.com/fatih/color"

func PrintStartupBanner() {
	banner := color.New(color.Bold, color.FgHiMagenta).PrintlnFunc()
	banner("GR25 TCM MQTT")
	version := color.New(color.Bold, color.FgMagenta).PrintlnFunc()
	version("Running v" + Version + " [ENV: " + Env + "]")
	println()
}
