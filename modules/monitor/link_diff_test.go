package monitor_test

import (
	"fmt"
	"testing"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/monitor"
)

func Test_ComputeLinkDiff(t *testing.T) {
	inputBaseline := map[string]*monitor.DiffLink{}
	inputActual := map[string]*monitor.DiffLink{}

	// baseline
	diffLink := monitor.NewDiffLink("192.168.127.1", 1, "192.168.127.2", 1)
	inputBaseline[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.127.2", 2, "192.168.127.3", 2)
	inputBaseline[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.127.3", 3, "192.168.127.4", 3)
	inputBaseline[diffLink.String()] = diffLink

	// invalid baseline links
	diffLink = monitor.NewDiffLink("", 1, "192.168.130.1", 1)
	inputBaseline[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.130.2", 1, "192.168.130.2", 1)
	inputBaseline[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("invalidBaselineIP", 1, "192.168.130.3", 1)
	inputBaseline[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.130.4", 0, "192.168.130.4", 1)
	inputBaseline[diffLink.String()] = diffLink

	// actual
	diffLink = monitor.NewDiffLink("192.168.127.1", 1, "192.168.127.2", 1)
	inputActual[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.127.2", 2, "192.168.127.3", 8)
	inputActual[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.127.4", 4, "192.168.127.5", 4)
	inputActual[diffLink.String()] = diffLink

	// invalid actual links
	diffLink = monitor.NewDiffLink("", 1, "192.168.140.1", 1)
	inputActual[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.140.2", 4, "192.168.140.2", 1)
	inputActual[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("invalidActualIP", 4, "192.168.127.3", 4)
	inputActual[diffLink.String()] = diffLink
	diffLink = monitor.NewDiffLink("192.168.140.4", 0, "192.168.140.4", 4)
	inputActual[diffLink.String()] = diffLink

	expectedUnchangedLink := map[string]*monitor.DiffLink{
		"192.168.127.1:1-192.168.127.2:1": monitor.NewDiffLink("192.168.127.1", 1, "192.168.127.2", 1),
	}

	expectedRemovedLink := map[string]*monitor.DiffLink{
		"192.168.127.2:2-192.168.127.3:2": monitor.NewDiffLink("192.168.127.2", 2, "192.168.127.3", 2),
		"192.168.127.3:3-192.168.127.4:3": monitor.NewDiffLink("192.168.127.3", 3, "192.168.127.4", 3),
	}

	expectedNewLink := map[string]*monitor.DiffLink{
		"192.168.127.2:2-192.168.127.3:8": monitor.NewDiffLink("192.168.127.2", 2, "192.168.127.3", 8),
		"192.168.127.4:4-192.168.127.5:4": monitor.NewDiffLink("192.168.127.4", 4, "192.168.127.5", 4),
	}

	expectedRewiredLink := map[string]*monitor.DiffLink{
		"192.168.127.2:2-192.168.127.3:8": monitor.NewDiffLink("192.168.127.2", 2, "192.168.127.3", 8),
	}

	expectedInvalidBaselineLink := map[string]*monitor.DiffLink{
		":1-192.168.130.1:1":                  monitor.NewDiffLink("", 1, "192.168.130.1", 1),
		"192.168.130.2:1-192.168.130.2:1":     monitor.NewDiffLink("192.168.130.2", 1, "192.168.130.2", 1),
		"invalidBaselineIP:1-192.168.130.3:1": monitor.NewDiffLink("invalidBaselineIP", 1, "192.168.130.3", 1),
		"192.168.130.4:0-192.168.130.4:1":     monitor.NewDiffLink("192.168.130.4", 0, "192.168.130.4", 1),
	}

	expectedInvalidActualLink := map[string]*monitor.DiffLink{
		"192.168.140.4:0-192.168.140.4:4":   monitor.NewDiffLink("192.168.140.4", 0, "192.168.140.4", 4),
		":1-192.168.140.1:1":                monitor.NewDiffLink("", 1, "192.168.140.1", 1),
		"192.168.140.2:4-192.168.140.2:1":   monitor.NewDiffLink("192.168.140.2", 4, "192.168.140.2", 1),
		"invalidActualIP:4-192.168.127.3:4": monitor.NewDiffLink("invalidActualIP", 4, "192.168.127.3", 4),
	}

	diffResult, err := monitor.ComputeLinkDiff(inputBaseline, inputActual)

	if err != nil {
		t.Fatalf("ComputeLinkDiff failed: %v", err)
	}

	// assert result
	for linkStr, _ := range expectedUnchangedLink {
		if _, exists := diffResult.UnchangedLinks[linkStr]; !exists {
			t.Errorf("Unchanged link %s not found in result", linkStr)
		}
	}

	for linkStr, _ := range expectedRemovedLink {
		if _, exists := diffResult.RemovedLinks[linkStr]; !exists {
			t.Errorf("Removed link %s not found in result", linkStr)
		}
	}

	for linkStr, _ := range expectedNewLink {
		if _, exists := diffResult.NewLinks[linkStr]; !exists {
			t.Errorf("New link %s not found in result", linkStr)
		}
	}

	for linkStr, _ := range expectedRewiredLink {
		if _, exists := diffResult.RewiredLinks[linkStr]; !exists {
			t.Errorf("Rewired link %s not found in result", linkStr)
		}
	}

	for linkStr, _ := range expectedInvalidBaselineLink {
		if _, exists := diffResult.InvalidBaselineLinks[linkStr]; !exists {
			t.Errorf("Invalid baseline link %s not found in result", linkStr)
		}
	}

	for linkStr, _ := range expectedInvalidActualLink {
		if _, exists := diffResult.InvalidActualLinks[linkStr]; !exists {
			t.Errorf("Invalid actual link %s not found in result", linkStr)
		}
	}

	// assert link count
	if len(diffResult.UnchangedLinks) != len(expectedUnchangedLink) {
		t.Errorf("Unchanged link count mismatch: expected %d, got %d", len(expectedUnchangedLink), len(diffResult.UnchangedLinks))
	}

	if len(diffResult.RemovedLinks) != len(expectedRemovedLink) {
		t.Errorf("Removed link count mismatch: expected %d, got %d", len(expectedRemovedLink), len(diffResult.RemovedLinks))
	}

	if len(diffResult.NewLinks) != len(expectedNewLink) {
		t.Errorf("New link count mismatch: expected %d, got %d", len(expectedNewLink), len(diffResult.NewLinks))
	}

	if len(diffResult.RewiredLinks) != len(expectedRewiredLink) {
		t.Errorf("Rewired link count mismatch: expected %d, got %d", len(expectedRewiredLink), len(diffResult.RewiredLinks))
	}

	if len(diffResult.InvalidBaselineLinks) != len(expectedInvalidBaselineLink) {
		t.Errorf("Invalid baseline link count mismatch: expected %d, got %d", len(expectedInvalidBaselineLink), len(diffResult.InvalidBaselineLinks))
	}

	if len(diffResult.InvalidActualLinks) != len(expectedInvalidActualLink) {
		t.Errorf("Invalid actual link count mismatch: expected %d, got %d", len(expectedInvalidActualLink), len(diffResult.InvalidActualLinks))
	}

	// print result
	fmt.Println(diffResult.String())
}
