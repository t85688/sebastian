package monitor

import "sync"

type MultiIndexCache[K comparable, V any] struct {
	data       map[K]V
	mutex      sync.RWMutex
	indexFnMap map[string]func(V) string
	indexMap   map[string]map[string]K
}

func (cache *MultiIndexCache[K, V]) AddIndex(indexName string, indexFunc func(V) string) {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	cache.indexFnMap[indexName] = indexFunc
	cache.indexMap[indexName] = make(map[string]K, len(cache.data))
	for key, value := range cache.data {
		indexKey := indexFunc(value)
		cache.indexMap[indexName][indexKey] = key
	}
}

func (cache *MultiIndexCache[K, V]) DeleteIndex(indexName string) {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	delete(cache.indexFnMap, indexName)
	delete(cache.indexMap, indexName)
}

func (cache *MultiIndexCache[K, V]) Get(key K) (V, bool) {
	cache.mutex.RLock()
	defer cache.mutex.RUnlock()

	value, exists := cache.data[key]
	return value, exists
}

func (cache *MultiIndexCache[K, V]) GetByIndex(indexName string, indexKey string) (V, bool) {
	cache.mutex.RLock()
	defer cache.mutex.RUnlock()

	index, exists := cache.indexMap[indexName]
	if !exists {
		var zero V
		return zero, false
	}

	primaryKey, exists := index[indexKey]
	if !exists {
		var zero V
		return zero, false
	}

	value, exists := cache.data[primaryKey]
	return value, exists
}

func (cache *MultiIndexCache[K, V]) GetAll() map[K]V {
	cache.mutex.RLock()
	defer cache.mutex.RUnlock()

	result := make(map[K]V, len(cache.data))
	for key, value := range cache.data {
		result[key] = value
	}

	return result
}

func (cache *MultiIndexCache[K, V]) GetAllByIndex(indexName string) map[string]V {
	cache.mutex.RLock()
	defer cache.mutex.RUnlock()

	result := make(map[string]V)
	index, exists := cache.indexMap[indexName]
	if !exists {
		return result
	}

	for indexKey, primaryKey := range index {
		if value, exists := cache.data[primaryKey]; exists {
			result[indexKey] = value
		}
	}

	return result
}

func (cache *MultiIndexCache[K, V]) Clear() {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	cache.data = make(map[K]V)
	for indexName := range cache.indexMap {
		cache.indexMap[indexName] = make(map[string]K)
	}
}

func (cache *MultiIndexCache[K, V]) Set(key K, value V) {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	cache.data[key] = value
	for indexName, indexFunc := range cache.indexFnMap {
		indexKey := indexFunc(value)
		cache.indexMap[indexName][indexKey] = key
	}
}

func (cache *MultiIndexCache[K, V]) SetAll(data map[K]V) {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	for key, value := range data {
		cache.data[key] = value
		for indexName, indexFunc := range cache.indexFnMap {
			indexKey := indexFunc(value)
			cache.indexMap[indexName][indexKey] = key
		}
	}
}

func (cache *MultiIndexCache[K, V]) Delete(key K) {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	value, exists := cache.data[key]
	if !exists {
		return
	}

	delete(cache.data, key)

	for indexName, indexFunc := range cache.indexFnMap {
		indexKey := indexFunc(value)
		delete(cache.indexMap[indexName], indexKey)
	}
}

func (cache *MultiIndexCache[K, V]) DeleteByIndex(indexName string, indexKey string) {
	cache.mutex.Lock()
	defer cache.mutex.Unlock()

	index, exists := cache.indexMap[indexName]
	if !exists {
		return
	}

	primaryKey, exists := index[indexKey]
	if !exists {
		return
	}

	value, exists := cache.data[primaryKey]
	if !exists {
		return
	}

	delete(cache.data, primaryKey)

	for idxName, indexFunc := range cache.indexFnMap {
		idxKey := indexFunc(value)
		delete(cache.indexMap[idxName], idxKey)
	}
}

func NewMultiIndexCache[K comparable, V any]() *MultiIndexCache[K, V] {
	return &MultiIndexCache[K, V]{
		data:       make(map[K]V),
		indexFnMap: make(map[string]func(V) string),
		indexMap:   make(map[string]map[string]K),
	}
}
