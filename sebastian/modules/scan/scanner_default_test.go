package scan

import "testing"

func Test_calculateOverallProgress(t *testing.T) {
	testDatas := [][]int{
		{0, 0, 0, 0},
		{50, 0, 100, 50},
		{50, 0, 30, 15},
		{60, 0, 30, 18},
		{65, 0, 30, 19},
		{70, 0, 30, 21},
		{100, 0, 30, 30},
		{0, 70, 100, 70},
		{33, 70, 100, 79},
		{100, 70, 100, 100},
		{100, 0, 100, 100},
	}

	for _, data := range testDatas {
		actual := calculateOverallProgress(data[0], data[1], data[2])
		if actual != data[3] {
			t.Fatalf("Test_calculateOverallProgress(%v) = %v, want %v", data, actual, data[3])
		}
	}
}
