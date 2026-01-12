package monitor_test

import (
	"testing"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/monitor"
)

type testCacheValue struct {
	Id    int64
	Name  string
	Value string
}

func Test_MultiIndexCache(t *testing.T) {
	cache := monitor.NewMultiIndexCache[int64, *testCacheValue]()

	cacheMap := cache.GetAll()
	if len(cacheMap) != 0 {
		t.Fatalf("Expected empty cache, got %d items", len(cacheMap))
	}

	data1 := &testCacheValue{Id: 1, Name: "name1", Value: "value1"}
	data2 := &testCacheValue{Id: 2, Name: "name2", Value: "value2"}
	data3 := &testCacheValue{Id: 3, Name: "name3", Value: "value3"}
	data4 := &testCacheValue{Id: 4, Name: "name4", Value: "value4"}

	cache.Set(1, data1)
	cache.Set(2, data2)
	cache.Set(3, data3)
	cache.Set(4, data4)

	val, exists := cache.Get(1)
	if !exists || val == nil {
		t.Fatalf("Expected %v, got %v", data1, val)
	}

	if val.Id != 1 {
		t.Errorf("Expected Id 1, got %d", val.Id)
	}

	if val.Name != "name1" {
		t.Errorf("Expected name1, got %s", val.Name)
	}

	if val.Value != "value1" {
		t.Errorf("Expected value1, got %s", val.Value)
	}

	valByNotExistsIndex, exists := cache.GetByIndex("not_exists", "some_value")
	if exists || valByNotExistsIndex != nil {
		t.Fatalf("Expected not exists, got %v", valByNotExistsIndex)
	}

	valByName, exists := cache.GetByIndex("name", "name1")
	if exists || valByName != nil {
		t.Fatalf("Expected not exists, got %v", valByName)
	}

	cache.AddIndex("name", func(value *testCacheValue) string {
		return value.Name
	})

	valByName, exists = cache.GetByIndex("name", "name1")
	if !exists {
		t.Fatalf("Expected exists, but not found")
	}

	if valByName == nil {
		t.Fatalf("Expected value, got nil")
	}

	if valByName.Id != 1 {
		t.Errorf("Expected Id 1, got %d", valByName.Id)
	}

	if valByName.Name != "name1" {
		t.Errorf("Expected name1, got %s", valByName.Name)
	}

	if valByName.Value != "value1" {
		t.Errorf("Expected value1, got %s", valByName.Value)
	}

	cache.Delete(1)
	val, exists = cache.Get(1)
	if exists || val != nil {
		t.Fatalf("Expected not exists, got %v", val)
	}

	valByName, exists = cache.GetByIndex("name", "name1")
	if exists || valByName != nil {
		t.Fatalf("Expected not exists, got %v", valByName)
	}

	cacheMap = cache.GetAll()
	if len(cacheMap) != 3 {
		t.Fatalf("Expected 3 items in cache, got %d items", len(cacheMap))
	}

	cache.DeleteByIndex("name", "name2")

	val, exists = cache.Get(2)
	if exists || val != nil {
		t.Fatalf("Expected not exists, got %v", val)
	}

	cacheMap = cache.GetAll()
	if len(cacheMap) != 2 {
		t.Fatalf("Expected 2 items in cache, got %d items", len(cacheMap))
	}

	valByName, exists = cache.GetByIndex("name", "name2")
	if exists || valByName != nil {
		t.Fatalf("Expected not exists, got %v", valByName)
	}

	cache.Clear()
	cacheMap = cache.GetAll()
	if len(cacheMap) != 0 {
		t.Fatalf("Expected empty cache, got %d items", len(cacheMap))
	}
}
