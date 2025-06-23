#ifndef VMUTILS_H
#define VMUTILS_H
#include "vmmain.hpp"

#define malloc(size) ctx->win32.RtlAllocateHeap(ctx->heap, 0, size);
#define realloc(ptr, size) ctx->win32.RtlReAllocateHeap(ctx->heap, 0, ptr, size);
#define free(ptr) ctx->win32.RtlFreeHeap(ctx->heap, 0, ptr);

namespace simple_map {
	template<typename K, typename V>
	struct entry {
		K key;
		V value;
	};

	template<typename K, typename V>
	struct unordered_map {
		entry<K, V>* entries;
		size_t capacity;
	};

	template<typename K, typename V>
	simple_map::unordered_map<K, V> *init() {
		simple_map::unordered_map<K, V> *map =
				(simple_map::unordered_map<K, V> *) malloc(sizeof(simple_map::unordered_map<K, V>));

		map->entries = nullptr;
		map->capacity = 0;
		return map;
	}

	template<typename K, typename V>
	void push(simple_map::unordered_map<K, V> *map, K key, V value) {
		entry<K, V> *temp = (entry<K, V> *) realloc(map->entries, sizeof(entry<K, V>) * (map->capacity + 1));

		if (!temp) {
			return;
		}
		map->entries = temp;
		map->entries[map->capacity].key = key;
		map->entries[map->capacity].value = value;
		map->capacity += 1;
	}

	template<typename K, typename V>
	V find(simple_map::unordered_map<K, V> *map, K key) {
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->entries[i].key == key) {
				return map->entries[i].value;
			}
		}
		return V{};
	}

	template<typename K, typename V>
	void pop(simple_map::unordered_map<K, V> *map, K key) {
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->entries[i].key == key) {
				for (size_t j = i; j < map->capacity - 1; j++) {
					map->entries[j] = map->entries[j + 1];
				}
				map->capacity -= 1;

				if (map->capacity > 0) {
					map->entries = (entry<K, V> *) realloc(map->entries, sizeof(entry<K, V>) * map->capacity);
				} else {
					free(map->entries);
					map->entries = nullptr;
				}
				return;
			}
		}
	}

	template<typename K, typename V>
	void destroy(simple_map::unordered_map<K, V> *map) {
		if (map) {
			free(map->entries);
			free(map);
		}
	}
}
#endif //VMUTILS_H
