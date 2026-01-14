package macutility

import "testing"

func Test_ParseMacAddress(t *testing.T) {
	macstr_colon := "00:90:E8:86:71:DA"
	_, err := ParseMACAddress(macstr_colon)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	macstr_colon_lower := "00:90:e8:86:71:da"
	_, err = ParseMACAddress(macstr_colon_lower)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	macstr_dash := "00-90-E8-86-71-DA"
	_, err = ParseMACAddress(macstr_dash)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	macstr_dash_lower := "00-90-e8-86-71-da"
	_, err = ParseMACAddress(macstr_dash_lower)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	macstr_raw := "0090E88671DA"
	_, err = ParseMACAddress(macstr_raw)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	macstr_raw_lower := "0090e88671da"
	_, err = ParseMACAddress(macstr_raw_lower)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	macstr_failcase := "failcase"
	_, err = ParseMACAddress(macstr_failcase)
	if err == nil {
		t.Errorf("err should not be nil: %v", err)
	}

	macstr_empty := ""
	_, err = ParseMACAddress(macstr_empty)
	if err == nil {
		t.Errorf("err should not be nil: %v", err)
	}
}

func Test_FormatMacAddress_Colon(t *testing.T) {
	macstr_colon := "00:90:E8:86:71:DA"
	mac, err := ParseMACAddress(macstr_colon)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	upp := mac.Colon(UpperCase)
	upp_expected := "00:90:E8:86:71:DA"
	if upp != upp_expected {
		t.Errorf("upp should be: %v", upp_expected)
	}

	low := mac.Colon(LowerCase)
	low_expected := "00:90:e8:86:71:da"
	if low != low_expected {
		t.Errorf("low should be: %v", low_expected)
	}
}

func Test_FormatMacAddress_Dash(t *testing.T) {
	macstr_colon := "00:90:E8:86:71:DA"
	mac, err := ParseMACAddress(macstr_colon)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	upp := mac.Dashes(UpperCase)
	upp_expected := "00-90-E8-86-71-DA"
	if upp != upp_expected {
		t.Errorf("upp should be: %v", upp_expected)
	}

	low := mac.Dashes(LowerCase)
	low_expected := "00-90-e8-86-71-da"
	if low != low_expected {
		t.Errorf("low should be: %v", low_expected)
	}

}

func Test_FormatMacAddress_Raw(t *testing.T) {

	macstr_colon := "00:90:E8:86:71:DA"
	mac, err := ParseMACAddress(macstr_colon)
	if err != nil {
		t.Errorf("err should be nil: %v", err)
	}

	upp := mac.NoSeparators(UpperCase)
	upp_expected := "0090E88671DA"
	if upp != upp_expected {
		t.Errorf("upp should be: %v", upp_expected)
	}

	low := mac.NoSeparators(LowerCase)
	low_expected := "0090e88671da"
	if low != low_expected {
		t.Errorf("low should be: %v", low_expected)
	}
}
