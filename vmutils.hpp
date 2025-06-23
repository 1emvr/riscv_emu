#ifndef VMUTILS_H
#define VMUTILS_H
#include "vmcommon.hpp"
#include "vmmain.hpp"

namespace simple_map {
	template<typename A, typename B>
	struct entry {
		A key;
		B value;
	};

	template<typename A, typename B>
	struct unordered_map {
		entry<A, B> *entries;
		size_t capacity;
	};

	template<typename A, typename B>
	simple_map::unordered_map<A, B> *init() {
		simple_map::unordered_map<A, B> *map =
				(simple_map::unordered_map<A, B> *) malloc(sizeof(simple_map::unordered_map<A, B>));

		map->entries = nullptr;
		map->capacity = 0;
		return map;
	}

	template<typename A, typename B>
	void push(simple_map::unordered_map<A, B> *map, A key, B value) {
		entry<A, B> *temp = (entry<A, B> *) realloc(map->entries, sizeof(entry<A, B>) * (map->capacity + 1));

		if (!temp) {
			return;
		}
		map->entries = temp;
		map->entries[map->capacity].key = key;
		map->entries[map->capacity].value = value;
		map->capacity += 1;
	}

	template<typename A, typename B>
	B find(simple_map::unordered_map<A, B> *map, A key) {
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->entries[i].key == key) {
				return map->entries[i].value;
			}
		}
		return B{};
	}

	template<typename A, typename B>
	void pop(simple_map::unordered_map<A, B> *map, A key) {
		for (size_t i = 0; i < map->capacity; i++) {
			if (map->entries[i].key == key) {
				for (size_t j = i; j < map->capacity - 1; j++) {
					// unlink from the list
					map->entries[j] = map->entries[j + 1];
				}
				map->capacity -= 1;

				if (map->capacity > 0) {
					map->entries = (entry<A, B> *) realloc(map->entries, sizeof(entry<A, B>) * map->capacity);
				} else {
					free(map->entries);
					map->entries = nullptr;
				}
				return;
			}
		}
	}

	template<typename A, typename B>
	void destroy(simple_map::unordered_map<A, B> *map) {
		if (map) {
			free(map->entries);
			free(map);
		}
	}
}
#endif //VMUTILS_H
