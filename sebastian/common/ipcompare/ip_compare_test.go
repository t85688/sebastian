package ipcompare_test

import (
	"testing"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/ipcompare"
)

func TestCompareIP_NilFirstIP(t *testing.T) {
	ip1 := []byte{}
	ip2 := []byte{192, 168, 1, 1}

	result := ipcompare.CompareIP(ip1, ip2)

	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_EmptyFirstIP(t *testing.T) {
	var ip1 []byte = nil
	ip2 := []byte{192, 168, 1, 1}

	result := ipcompare.CompareIP(ip1, ip2)

	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_InvalidFirstIP(t *testing.T) {
	ip1 := []byte{'i', 'n', 'v', 'a', 'l', 'i', 'd'}
	ip2 := []byte{192, 168, 1, 1}

	result := ipcompare.CompareIP(ip1, ip2)

	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_NilSecondIP(t *testing.T) {
	ip1 := []byte{192, 168, 1, 1}
	var ip2 []byte = nil

	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultSecondIPNotValid {
		t.Errorf("Expected secondIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_EmptySecondIP(t *testing.T) {
	ip1 := []byte{192, 168, 1, 1}
	ip2 := []byte{}

	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultSecondIPNotValid {
		t.Errorf("Expected secondIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_InvalidSecondIP(t *testing.T) {
	ip1 := []byte{192, 168, 1, 1}
	ip2 := []byte{'i', 'n', 'v', 'a', 'l', 'i', 'd'}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultSecondIPNotValid {
		t.Errorf("Expected secondIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_NilBothIP(t *testing.T) {
	var ip1 []byte = nil
	var ip2 []byte = nil
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIP_IPv4Equal(t *testing.T) {
	ip1 := []byte{192, 168, 1, 1}
	ip2 := []byte{192, 168, 1, 1}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultEqual {
		t.Errorf("Expected equal, got %v", result.String())
	}
}

func TestCompareIP_IPv4Less(t *testing.T) {
	ip1 := []byte{192, 168, 1, 1}
	ip2 := []byte{192, 168, 1, 2}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultLess {
		t.Errorf("Expected less, got %v", result.String())
	}
}

func TestCompareIP_IPv4Greater(t *testing.T) {
	ip1 := []byte{192, 168, 1, 2}
	ip2 := []byte{192, 168, 1, 1}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultGreater {
		t.Errorf("Expected greater, got %v", result.String())
	}
}

func TestCompareIP_IPv6Equal(t *testing.T) {
	ip1 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34}
	ip2 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultEqual {
		t.Errorf("Expected equal, got %v", result.String())
	}
}

func TestCompareIP_IPv6Less(t *testing.T) {
	ip1 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x33}
	ip2 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultLess {
		t.Errorf("Expected less, got %v", result.String())
	}
}

func TestCompareIP_IPv6Greater(t *testing.T) {
	ip1 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x35}
	ip2 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultGreater {
		t.Errorf("Expected greater, got %v", result.String())
	}
}

func TestCompareIP_IPv4AndIPv6(t *testing.T) {
	ip1 := []byte{192, 168, 1, 1}
	ip2 := []byte{0x20, 0x01, 0x0d, 0xb8, 0x85, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x8a, 0x2e, 0x03, 0x70, 0x73, 0x34}
	result := ipcompare.CompareIP(ip1, ip2)
	if result != ipcompare.CompareIPResultLess {
		t.Errorf("Expected less, got %v", result.String())
	}
}

func TestCompareIPByString_EmptyFirstIP(t *testing.T) {
	ip1Str := ""
	ip2Str := "192.168.1.1"

	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIPByString_InvalidFirstIP(t *testing.T) {
	ip1Str := "invalid"
	ip2Str := "192.168.1.1"

	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIPByString_EmptySecondIP(t *testing.T) {
	ip1Str := "192.168.1.1"
	ip2Str := ""
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultSecondIPNotValid {
		t.Errorf("Expected secondIPNotValid, got %v", result.String())
	}
}

func TestCompareIPByString_InvalidSecondIP(t *testing.T) {
	ip1Str := "192.168.1.1"
	ip2Str := "invalid"

	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultSecondIPNotValid {
		t.Errorf("Expected secondIPNotValid, got %v", result.String())
	}
}

func TestCompareIPByString_EmptyBothIP(t *testing.T) {
	ip1Str := ""
	ip2Str := ""
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultFirstIPNotValid {
		t.Errorf("Expected firstIPNotValid, got %v", result.String())
	}
}

func TestCompareIPByString_IPv4Less(t *testing.T) {
	ip1Str := "192.168.1.1"
	ip2Str := "192.168.1.2"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultLess {
		t.Errorf("Expected less, got %v", result.String())
	}
}

func TestCompareIPByString_IPv4Equal(t *testing.T) {
	ip1Str := "192.168.1.1"
	ip2Str := "192.168.1.1"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultEqual {
		t.Errorf("Expected equal, got %v", result.String())
	}
}

func TestCompareIPByString_IPv4Greater(t *testing.T) {
	ip1Str := "192.168.1.2"
	ip2Str := "192.168.1.1"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultGreater {
		t.Errorf("Expected greater, got %v", result.String())
	}
}

func TestCompareIPByString_IPv6Less(t *testing.T) {
	ip1Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7333"
	ip2Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultLess {
		t.Errorf("Expected less, got %v", result.String())
	}
}

func TestCompareIPByString_IPv6Equal(t *testing.T) {
	ip1Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
	ip2Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultEqual {
		t.Errorf("Expected equal, got %v", result.String())
	}
}

func TestCompareIPByString_IPv6Greater(t *testing.T) {
	ip1Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7335"
	ip2Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultGreater {
		t.Errorf("Expected greater, got %v", result.String())
	}
}

func TestCompareIPByString_IPv4AndIPv6(t *testing.T) {
	ip1Str := "192.168.1.1"
	ip2Str := "2001:0db8:85a3:0000:0000:8a2e:0370:7334"
	result := ipcompare.CompareIPByString(ip1Str, ip2Str)
	if result != ipcompare.CompareIPResultLess {
		t.Errorf("Expected less, got %v", result.String())
	}
}
