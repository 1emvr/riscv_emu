#ifndef VMUTILS_H
#define VMUTILS_H
#include <windows.h>
#include "vmmain.hpp"

namespace simple_map {
	template<typename K, typename V>
	struct entry {
		K key;
		V value;
	};

	template<typename K, typename V>
	class unordered_map {
		entry<K, V> *entries;
		size_t capacity{};

		unordered_map(): entries(nullptr) {
			this = (unordered_map*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(unordered_map));

			this->entries = nullptr;
			this->capacity = 0;
		}

		~unordered_map() {
			HeapFree(GetProcessHeap(), 0, this->entries);
			HeapFree(GetProcessHeap(), 0, this);
		}

		void push(K key, V value) {
			auto temp = (entry<K, V> *) realloc(this->entries, sizeof(entry<K, V>) * (this->capacity + 1));

			if (!temp) {
				return;
			}
			this->entries = temp;
			this->entries[this->capacity].key = key;
			this->entries[this->capacity].value = value;
			this->capacity += 1;
		}

		V find(K key) {
			if (!this->entries || !this->capacity) {
				return 0;
			}
			for (size_t i = 0; i < this->capacity; i++) {
				if (this->entries[i].key == key) {
					return this->entries[i].value;
				}
			}
			return this->entries + this->capacity;
		}

		void pop(K key) {
			if (!this->entries) {
				return;
			}
			for (size_t i = 0; i < this->capacity; i++) {
				if (this->entries[i].key == key) {
					for (size_t j = i; j < this->capacity - 1; j++) {
						this->entries[j] = this->entries[j + 1];
					}
					this->capacity -= 1;

					if (this->capacity > 0) {
						this->entries = (entry<K, V> *) HeapreAlloc(
							GetProcessHeap(), HEAP_ZERO_MEMORY, this->entries, sizeof(entry<K, V>) * this->capacity);
					} else {
						HeapFree(GetProcessHeap(), 0, this->entries);
						this->entries = nullptr;
					}
					return;
				}
			}
		}

		entry<K, V> *begin(simple_map::unordered_map<K, V> *map) {
			return map->entries;
		}

		entry<K, V> *end(simple_map::unordered_map<K, V> *map) {
			return map->entries + map->capacity;
		}

	};
};

int map_main() {
	simple_map::unordered_map<uint64_t, uint64_t> map;
	map.push(0, 0);
}

#endif //VMUTILS_H
