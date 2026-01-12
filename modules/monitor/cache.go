package monitor

import "sync"

type LocalCache[K comparable, T any] struct {
	data  map[K]T
	mutex sync.RWMutex
}

func NewLocalCache[K comparable, T any]() *LocalCache[K, T] {
	return &LocalCache[K, T]{
		data: make(map[K]T),
	}
}

func (localCache *LocalCache[K, T]) Get(key K) (T, bool) {
	localCache.mutex.RLock()
	defer localCache.mutex.RUnlock()

	value, ok := localCache.data[key]
	return value, ok
}

func (localCache *LocalCache[K, T]) GetAll() map[K]T {
	localCache.mutex.RLock()
	defer localCache.mutex.RUnlock()

	newMap := make(map[K]T)
	for k, v := range localCache.data {
		newMap[k] = v
	}
	return newMap
}

func (localCache *LocalCache[K, T]) Set(key K, value T) {
	localCache.mutex.Lock()
	defer localCache.mutex.Unlock()

	localCache.data[key] = value
}

func (localCache *LocalCache[K, T]) SetAll(newPairs map[K]T) {
	localCache.mutex.Lock()
	defer localCache.mutex.Unlock()

	localCache.data = make(map[K]T)
	for k, v := range newPairs {
		localCache.data[k] = v
	}
}

func (localCache *LocalCache[K, T]) Delete(key K) {
	localCache.mutex.Lock()
	defer localCache.mutex.Unlock()

	delete(localCache.data, key)
}

func (localCache *LocalCache[K, T]) Clear() {
	localCache.mutex.Lock()
	defer localCache.mutex.Unlock()

	localCache.data = make(map[K]T)
}
