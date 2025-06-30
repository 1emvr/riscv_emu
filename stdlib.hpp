#ifndef STDLIB_HPP
#define STDLIB_HPP
#include <stdint.h>
#include <windows.h>

void x_memcpy(void *dst, const void *const src, const size_t n) {
    const auto a = (uint8_t*) dst;
    const auto b = (const uint8_t*) src;

    for (size_t i = 0; i < n; i++) {
        a[i] = b[i];
    }
}

void *x_memset(void *const dst, const int val, size_t len) {
    auto ptr = (uint8_t*) dst;
    while (len-- > 0) {
        *ptr++ = val;
    }
    return dst;
}

char* x_strcpy(char *dst, const char *src, size_t n) {
    if (!n) {
        return dst;
    }

    char *d = dst;
    size_t copied = 0;

    while (copied < n - 1 && *src) {
        *d++ = *src++;
        copied++;
    }

    *d++ = '\0';
    copied++;

    while (copied < n) {
        *d++ = '\0';
        copied++;
    }

    return dst;
}

size_t x_strncmp(const char *str1, const char *str2, size_t len) {
    while (len && *str1 && (*str1 == *str2)) {
        len--; str1++; str2++;

        if (len == 0) {
            return  0;
        }
    }
    return len ? (unsigned char)*str1 - (unsigned char)*str2 : 0;
}


size_t x_strcmp(const char *str1, const char *str2) {
    while (*str1 && *str1 == *str2) {
        str1++; str2++;
    }

    return (uint8_t) *str1 - (uint8_t) *str2;
}

size_t x_memcmp(const void *const ptr1, const void *const ptr2, size_t len) {
    const auto *p1 = (const uint8_t*) ptr1;
    const auto *p2 = (const uint8_t*) ptr2;

    while (len--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }

        p1++; p2++;
    }

    return 0;
}

size_t x_strlen(const char* str) {
    auto len = 0;
    const auto s_str = str;

    while (s_str[len] != 0x00) {
        len++;
    }

    return len;
}

char *x_strcat(char *const dst, const char *const src) {

    size_t len1 = x_strlen(dst);
    x_strcpy(dst + len1, src, len1);
    return dst;
}

size_t x_wcslen(const wchar_t *const s) {
    size_t len = 0;
    const auto s_str = s;

    while (s_str[len] != 0x0000) {
        len++;
    }

    return len;
}

wchar_t* x_wcscpy(wchar_t *dst, const wchar_t *src, size_t n) {
    if (!n) {
        return dst;
    }

    wchar_t *d      = dst;
    size_t copied   = 0;

    while (copied < n - 1 && *src) {
        *d++ = *src++;
        copied++;
    }

    *d++ = L'\0';
    copied++;

    while (copied < n) {
        *d++ = L'\0';
        copied++;
    }

    return dst;
}

size_t x_wcscmp(const wchar_t *str1, const wchar_t *str2) {
    for (; *str1 == *str2; str1++, str2++) {
        if (*str1 == 0x0000) {
            return 0;
        }
    }
    return *str1 < *str2 ? -1 : +1;
}

wchar_t *x_wcscat(wchar_t *const str1, const wchar_t *const str2) {
    size_t len1 = x_wcslen(str1);
    x_wcscpy(str1 + len1, str2, len1);
    return str1;
}

wchar_t x_tolowerw(const wchar_t c) {
    if (c >= 0x0041 && c <= 0x005A) {
        return c + (0x0061 - 0x0041);
    }

    return c;
}

char x_tolower(const char c) {
    if (c >= 0x41 && c <= 0x5A) {
        return c + (0x61 - 0x41);
    }

    return c;
}

wchar_t *x_wcs_tolower(wchar_t *const dst, const wchar_t *const src) {
    const auto len = x_wcslen(src);
    for (size_t i = 0; i < len; ++i) {
        dst[i] = x_tolowerw(src[i]);
    }

    dst[len] = 0x0000;
    return dst;
}

char *x_mbs_tolower(char *const dst, const char *const src) {
    const auto len = x_strlen(src);
    for (size_t i = 0; i < len; ++i) {
        dst[i] = x_tolower((uint8_t) src[i]);
    }

    dst[len] = 0x00;
    return dst;
}

size_t x_mbstowcs(wchar_t *dst, const char *src, const size_t length) {
	for (size_t i = 0; i < length; i++) {
		dst[i] = (wchar_t)src[i] | 0x00;
	}

	dst[length] = L'\0';
	return x_wcslen(dst);
}

size_t x_wcstombs(char *const dst, const wchar_t *src, size_t length) {
	for (size_t i = 0; i < length; i++) {
		dst[i] = (char)(src[i] & 0xff);
	}

	dst[length * sizeof(wchar_t)] = '\0';
	return x_strlen(dst);
}

int x_endwith(const char *string, const char *const end) {
    uint32_t length1 = 0;
    uint32_t length2 = 0;

    if (!string || !end) {
        return false;
    }

    length1 = x_strlen(string);
    length2 = x_strlen(end);

    if (length1 < length2) {
        return false;
    }
    string = &string[length1 - length2];
    return x_strcmp(string, end) == 0;
}

int x_endwithw(const wchar_t *string, const wchar_t *const end) {
    uint32_t length1 = 0;
    uint32_t length2 = 0;

    if ( !string || !end ) {
        return false;
    }

    length1 = x_wcslen(string);
    length2 = x_wcslen(end);

    if ( length1 < length2 ) {
        return false;
    }

    string = &string[ length1 - length2 ];
    return x_wcscmp(string, end) == 0;
}

size_t x_span(const char* s, const char* accept) {
    int a = 1;
    int i;

    size_t offset = 0;

    while (a && *s) {
        for (a = i = 0; !a && i < x_strlen(accept); i++) {
            if (*s == accept[i]) {
                a = 1;
            }
        }

        if (a) {
            offset++;
        }
        s++;
    }

    return offset;
}

char* x_strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) {
            return (char*) str;
        }
        str++;
    }

    if (c == 0) {
        return (char*)str;
    }

    return nullptr;
}

char* x_strtok(char* str, const char* delim) {
    static char *saved          = { };
    char        *token_start    = { };
    char        *token_end      = { };

    if (str) { saved = str; }
    token_start = saved;

    if (!token_start) {
        saved = nullptr;
        return nullptr;
    }

    while (*token_start && x_strchr(delim, *token_start)) {
        token_start++;
    }

    token_end = token_start;
    while (*token_end && !x_strchr(delim, *token_end)) {
        token_end++;
    }

    if (*token_end) {
        *token_end  = 0;
        saved       = token_end + 1;
    }
    else {
        saved = nullptr;
    }

    return token_start;
}

char* x_strdup(const char* str) {
    char*   str2      = { };
    size_t  length  = 0;

    length  = x_strlen(str);
    str2    = (char*)HeapAlloc(GetProcessHeap(), 0, length + 1);

    if (!str2) {
        return nullptr;
    }

    x_memcpy(str2, str, length + 1);
    return str2;
}

char** x_split_new(const char* str, const char* delim, int* count) {
    char **result   = { };
    char **temp_res = { };

    char *temp      = { };
    char *token     = { };

    int size    = 2;
    int index   = 0;

    if (!(temp = x_strdup(str)) ||
        !(result = (char**) HeapAlloc(GetProcessHeap(), 0, size * sizeof(char *))) ||
        !(token = x_strtok(temp, delim))) {
        return nullptr;
    }

    while (token) {
        if (index > size) {
            size    += 1;
            temp_res = (char**) HeapReAlloc(GetProcessHeap(), 0, result, size * sizeof(char*));

            if (!temp_res) {
                HeapFree(GetProcessHeap(), 0, result);
                result = nullptr;
                goto defer;
            }

            result = temp_res;
        }

        result[index] = x_strdup(token);

        if (!result[index]) {
            for (auto i = 0; i < index; i++) {
                HeapFree(GetProcessHeap(), 0, result[i]);
            }

            HeapFree(GetProcessHeap(), 0, result);
            result = nullptr;

            goto defer;
        }

        index++;
        token = x_strtok(0, delim);
    }

    *count = index;
    result[index] = 0;

defer:
    HeapFree(GetProcessHeap(), 0, temp);
    return result;
}

void x_split_free(char** split, int count) {
    for (auto i = 0; i < count; i++) {
        HeapFree(GetProcessHeap(), 0, split[i]);
    }

    HeapFree(GetProcessHeap(), 0, split);
}

void x_trim(char* str, char delim) {
    for (auto i = 0; str[i]; i++) {
        if (str[i] == delim) {
            str[i] = 0;
        }
    }
}

BOOL StringChar(const char* string, const char symbol, size_t length) {
    for (auto i = 0; i < length - 1; i++) {
        if (string[i] == symbol) {
            return true;
        }
    }

    return false;
}
#endif // STDLIB_HPP
