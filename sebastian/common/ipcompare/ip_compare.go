package ipcompare

import (
	"bytes"
	"net"
)

type CompareIPResult int

const (
	CompareIPResultLess             CompareIPResult = -1
	CompareIPResultEqual            CompareIPResult = 0
	CompareIPResultGreater          CompareIPResult = 1
	CompareIPResultFirstIPNotValid  CompareIPResult = 2
	CompareIPResultSecondIPNotValid CompareIPResult = 3
)

var compareIPResultStrMap = map[CompareIPResult]string{
	CompareIPResultLess:             "less",
	CompareIPResultEqual:            "equal",
	CompareIPResultGreater:          "greater",
	CompareIPResultFirstIPNotValid:  "firstIPNotValid",
	CompareIPResultSecondIPNotValid: "secondIPNotValid",
}

func (result CompareIPResult) String() string {
	if str, exists := compareIPResultStrMap[result]; exists {
		return str
	}

	return "unknown"
}

func isValidIP(ip net.IP) bool {
	return ip != nil && (ip.To4() != nil || ip.To16() != nil)
}

func CompareIP(ip1 net.IP, ip2 net.IP) CompareIPResult {
	isValidIP1 := isValidIP(ip1)
	if !isValidIP1 {
		return CompareIPResultFirstIPNotValid
	}

	isValidIP2 := isValidIP(ip2)
	if !isValidIP2 {
		return CompareIPResultSecondIPNotValid
	}

	ip1 = ip1.To16()
	ip2 = ip2.To16()
	return CompareIPResult(bytes.Compare(ip1, ip2))
}

func CompareIPByString(ip1Str string, ip2Str string) CompareIPResult {
	ip1 := net.ParseIP(ip1Str)
	if ip1 == nil {
		return CompareIPResultFirstIPNotValid
	}

	ip2 := net.ParseIP(ip2Str)
	if ip2 == nil {
		return CompareIPResultSecondIPNotValid
	}

	compareResult := CompareIP(ip1, ip2)
	return compareResult
}
