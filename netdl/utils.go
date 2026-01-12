package netdl

import "strings"

func FormatMAC(mac string) string {
	mac = strings.TrimSpace(mac)
	mac = strings.ReplaceAll(mac, ":", "")
	mac = strings.ToLower(mac)
	return mac
}
