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
	public:
		entry<K, V> *entries = {};
		size_t capacity{};

		unordered_map() = default;
		~unordered_map() {
			if (entries) {
				HeapFree(GetProcessHeap(), 0, entries);
			}
		}

		void push(K key, V value) {
			auto temp = (entry<K, V> *) HeapReAlloc(
				GetProcessHeap(), HEAP_ZERO_MEMORY, this->entries, sizeof(entry<K, V>) * (this->capacity + 1));

			if (!temp) {
				return;
			}
			this->entries = temp;
			this->entries[this->capacity].key = key;
			this->entries[this->capacity].value = value;
			this->capacity += 1; // ...past the end
		}

		entry<K, V>* find(K key) {
			if (!this->entries || !this->capacity) {
				return end();
			}
			for (size_t i = 0; i < this->capacity; i++) {
				if (this->entries[i].key == key) {
					return this->entries[i];
				}
			}
			return end();
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
						this->entries = (entry<K, V> *) HeapReAlloc(
							GetProcessHeap(), HEAP_ZERO_MEMORY, this->entries, sizeof(entry<K, V>) * this->capacity);
					} else {
						HeapFree(GetProcessHeap(), 0, this->entries);
						this->entries = nullptr;
					}
					return;
				}
			}
		}

		entry<K, V> *begin() {
			return this->entries;
		}

		entry<K, V> *end() {
			return this->entries + this->capacity;
		}

	};
};

int map_main() {
	simple_map::unordered_map<uint64_t, uint64_t> map;
	map.push(1234, 5678);
	auto val = map.find(1234);
	if (val == 5678) {
		map.pop(1234);
		return true;
	}
	return false;
}

#endif //VMUTILS_H
