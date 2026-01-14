package macutility

import (
	"net"
	"strings"
)

type MACAddress struct {
	hw net.HardwareAddr
}

const (
	UpperCase = "UpperCase"
	LowerCase = "LowerCase"
)

// 解析字串是否是 mac address 格式
func ParseMACAddress(macstr string) (*MACAddress, error) {
	hw, err := net.ParseMAC(macstr)
	if err != nil {
		// 處理沒有分隔符號的case, e.g., 0090E88671DA
		hw, err = net.ParseMAC(withColons(macstr))
		if err != nil {
			return nil, err
		}
	}

	result := &MACAddress{
		hw: hw,
	}

	return result, nil
}

// 00:90:E8:86:71:DA
func (m *MACAddress) String() string {
	return m.hw.String()
}

// 00-90-E8-86-71-DA
func (m *MACAddress) Dashes(letterCase string) string {
	mac := m.hw.String()
	mac = strings.Replace(mac, ":", "-", -1)

	return transLetterCase(mac, letterCase)
}

// 0090E88671DA
func (m *MACAddress) NoSeparators(letterCase string) string {
	mac := m.hw.String()
	mac = strings.Replace(mac, ":", "", -1)

	return transLetterCase(mac, letterCase)
}

// 00:90:E8:86:71:DA
func (m *MACAddress) Colon(letterCase string) string {
	return transLetterCase(m.hw.String(), letterCase)
}

func (m *MACAddress) Int() int64 {
	var res int64
	for _, v := range m.hw {
		res <<= 8
		res |= int64(v)
	}
	return res
}
