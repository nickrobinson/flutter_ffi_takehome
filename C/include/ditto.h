// ditto.h
#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef _WIN32
  #ifdef DITTOFFI_EXPORTS
    #define DITTO_API __declspec(dllexport)
  #else
    #define DITTO_API __declspec(dllimport)
  #endif
#else
  #define DITTO_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ditto_db ditto_db_t;          // opaque handle

// Returns 0 on success, non-zero on error.
// On success, *out_db is a valid handle. Call ditto_close() when done.
DITTO_API int32_t ditto_open(const char* path, ditto_db_t** out_db);

// Always safe to call; frees resources and invalidates pointer.
DITTO_API void ditto_close(ditto_db_t* db);

// Put a value. Returns 0 on success.
DITTO_API int32_t ditto_put(ditto_db_t* db,
                            const char* key,
                            const uint8_t* data,
                            size_t len);

// Get a value into caller-provided buffer. If out_len is smaller than value,
// function sets *out_len to required size and returns 1 (buffer too small).
// Returns 0 on success, 2 if key not found, other non-zero on error.
DITTO_API int32_t ditto_get(ditto_db_t* db,
                            const char* key,
                            uint8_t* out_buf,
                            size_t* inout_len);

// Delete a key. Returns 0 on success, 2 if key not found.
DITTO_API int32_t ditto_delete(ditto_db_t* db, const char* key);

// Callback signature for change notifications.
// user_data is an opaque pointer provided at subscription time.
typedef void (*ditto_on_change_cb)(void* user_data, const char* key);

// Subscribe to changes for all keys. Returns a subscription id (>=1) via out_sub_id.
// Returns 0 on success.
DITTO_API int32_t ditto_subscribe(ditto_db_t* db,
                                  ditto_on_change_cb cb,
                                  void* user_data,
                                  int32_t* out_sub_id);

// Unsubscribe by id. Returns 0 on success.
DITTO_API int32_t ditto_unsubscribe(ditto_db_t* db, int32_t sub_id);

// Returns a null-terminated, static string (do not free).
DITTO_API const char* ditto_version(void);

// Thread-safety: ditto_db_t is safe to use from a single thread at a time.
// Callbacks may be invoked from a background thread.
#ifdef __cplusplus
}
#endif
