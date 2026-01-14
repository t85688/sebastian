package macutility

import "testing"

func Test_ChunkStr(t *testing.T) {
	str := "aabbccddeeff"
	expected := []string{"aa", "bb", "cc", "dd", "ee", "ff"}
	chunk := chunkStr(str, 2)

	if len(chunk) != len(expected) {
		t.Errorf("len should be: %v", len(expected))
	}

	for i, v := range chunk {
		if v != expected[i] {
			t.Errorf("expected should be: %v, but actual was %v", expected, chunk)
		}
	}
}

func Test_WithColon(t *testing.T) {
	str := "aabbccddeeff"
	expected := "aa:bb:cc:dd:ee:ff"

	actual := withColons(str)
	if actual != expected {
		t.Errorf("expected should be: %v, but actual was %v", expected, actual)
	}

}

func Test_TransLetterCase_Upper(t *testing.T) {
	str := "aabbccddeeff"
	expected := "AABBCCDDEEFF"

	actual := transLetterCase(str, UpperCase)

	if actual != expected {
		t.Errorf("expected should be: %v, but actual was %v", expected, actual)
	}

}

func Test_TransLetterCase_Lower(t *testing.T) {
	str := "AABBccDDeeFF"
	expected := "aabbccddeeff"

	actual := transLetterCase(str, LowerCase)

	if actual != expected {
		t.Errorf("expected should be: %v, but actual was %v", expected, actual)
	}

}
