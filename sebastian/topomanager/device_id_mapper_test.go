// Go
package topomanager

import "testing"

// Compile-time check that ActDeviceManager implements IDeviceIDMapper.
var _ IDeviceIDMapper = (*ActDeviceManager)(nil)

func newTestActDeviceManager() *ActDeviceManager {
	return &ActDeviceManager{
		ActDeviceIDToMAFDeviceID: make(map[int64]string),
		MAFDeviceIDToActDeviceID: make(map[string]int64),
	}
}

func Test_DeviceIDMapper_AddAndGet(t *testing.T) {
	t.Parallel()
	m := newTestActDeviceManager()

	m.AddDeviceMapping(1, "maf-1")

	if maf, ok := m.GetMAFDeviceID(1); !ok || maf != "maf-1" {
		t.Fatalf("GetMAFDeviceID(1) = (%q,%v), want (\"maf-1\",true)", maf, ok)
	}
	if act, ok := m.GetActDeviceID("maf-1"); !ok || act != 1 {
		t.Fatalf("GetActDeviceID(\"maf-1\") = (%d,%v), want (1,true)", act, ok)
	}
}

func Test_DeviceIDMapper_DeleteByActID(t *testing.T) {
	t.Parallel()
	m := newTestActDeviceManager()

	m.AddDeviceMapping(1, "maf-1")
	m.AddDeviceMapping(2, "maf-2")

	m.DeleteDeviceMappingByActDeviceID(1)

	if _, ok := m.GetMAFDeviceID(1); ok {
		t.Fatalf("GetMAFDeviceID(1) should be not found after delete by act ID")
	}
	if _, ok := m.GetActDeviceID("maf-1"); ok {
		t.Fatalf("GetActDeviceID(\"maf-1\") should be not found after delete by act ID")
	}
	// Ensure other mapping still exists
	if maf, ok := m.GetMAFDeviceID(2); !ok || maf != "maf-2" {
		t.Fatalf("GetMAFDeviceID(2) = (%q,%v), want (\"maf-2\",true)", maf, ok)
	}
	if act, ok := m.GetActDeviceID("maf-2"); !ok || act != 2 {
		t.Fatalf("GetActDeviceID(\"maf-2\") = (%d,%v), want (2,true)", act, ok)
	}
}

func Test_DeviceIDMapper_DeleteByMAFID(t *testing.T) {
	t.Parallel()
	m := newTestActDeviceManager()

	m.AddDeviceMapping(1, "maf-1")
	m.AddDeviceMapping(2, "maf-2")

	m.DeleteDeviceMappingByMAFDeviceID("maf-1")

	if _, ok := m.GetActDeviceID("maf-1"); ok {
		t.Fatalf("GetActDeviceID(\"maf-1\") should be not found after delete by maf ID")
	}
	if _, ok := m.GetMAFDeviceID(1); ok {
		t.Fatalf("GetMAFDeviceID(1) should be not found after delete by maf ID")
	}
	// Ensure other mapping still exists
	if maf, ok := m.GetMAFDeviceID(2); !ok || maf != "maf-2" {
		t.Fatalf("GetMAFDeviceID(2) = (%q,%v), want (\"maf-2\",true)", maf, ok)
	}
	if act, ok := m.GetActDeviceID("maf-2"); !ok || act != 2 {
		t.Fatalf("GetActDeviceID(\"maf-2\") = (%d,%v), want (2,true)", act, ok)
	}
}

func Test_DeviceIDMapper_Clear(t *testing.T) {
	t.Parallel()
	m := newTestActDeviceManager()

	m.AddDeviceMapping(1, "maf-1")
	m.AddDeviceMapping(2, "maf-2")

	m.ClearDeviceMappings()

	if _, ok := m.GetMAFDeviceID(1); ok {
		t.Fatalf("GetMAFDeviceID(1) should be not found after clear")
	}
	if _, ok := m.GetMAFDeviceID(2); ok {
		t.Fatalf("GetMAFDeviceID(2) should be not found after clear")
	}
	if _, ok := m.GetActDeviceID("maf-1"); ok {
		t.Fatalf("GetActDeviceID(\"maf-1\") should be not found after clear")
	}
	if _, ok := m.GetActDeviceID("maf-2"); ok {
		t.Fatalf("GetActDeviceID(\"maf-2\") should be not found after clear")
	}
	// Optional: verify internal maps are empty
	if got := len(m.ActDeviceIDToMAFDeviceID); got != 0 {
		t.Fatalf("len(ActDeviceIDToMAFDeviceID) = %d, want 0", got)
	}
	if got := len(m.MAFDeviceIDToActDeviceID); got != 0 {
		t.Fatalf("len(MAFDeviceIDToActDeviceID) = %d, want 0", got)
	}
}

func Test_DeviceIDMapper_DeleteNonExistent_NoEffect(t *testing.T) {
	t.Parallel()
	m := newTestActDeviceManager()

	m.AddDeviceMapping(1, "maf-1")
	m.DeleteDeviceMappingByActDeviceID(999)
	m.DeleteDeviceMappingByMAFDeviceID("maf-x")

	// Existing mapping should remain intact
	if maf, ok := m.GetMAFDeviceID(1); !ok || maf != "maf-1" {
		t.Fatalf("GetMAFDeviceID(1) = (%q,%v), want (\"maf-1\",true)", maf, ok)
	}
	if act, ok := m.GetActDeviceID("maf-1"); !ok || act != 1 {
		t.Fatalf("GetActDeviceID(\"maf-1\") = (%d,%v), want (1,true)", act, ok)
	}
}

func Test_DeviceIDMapper_ClearInitializesZeroValue(t *testing.T) {
	t.Parallel()
	// Zero-value manager (nil maps) should be usable after ClearDeviceMappings.
	m := &ActDeviceManager{}
	m.ClearDeviceMappings()
	m.AddDeviceMapping(42, "maf-42")

	if maf, ok := m.GetMAFDeviceID(42); !ok || maf != "maf-42" {
		t.Fatalf("GetMAFDeviceID(42) = (%q,%v), want (\"maf-42\",true)", maf, ok)
	}
	if act, ok := m.GetActDeviceID("maf-42"); !ok || act != 42 {
		t.Fatalf("GetActDeviceID(\"maf-42\") = (%d,%v), want (42,true)", act, ok)
	}
}