// ditto.c - Simple KV store implementation for Flutter FFI interview module
#include "../include/ditto.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define VERSION "1.0.0"
#define MAX_SUBSCRIPTIONS 100

// ============================================================================
// Internal Data Structures
// ============================================================================

// Simple hash table entry
typedef struct kv_entry {
    char* key;
    uint8_t* data;
    size_t len;
    struct kv_entry* next;
} kv_entry_t;

// Hash table
#define HASH_SIZE 256
typedef struct {
    kv_entry_t* buckets[HASH_SIZE];
    pthread_mutex_t lock;
} hash_table_t;

// Subscription entry
typedef struct {
    int32_t id;
    ditto_on_change_cb callback;
    void* user_data;
    int active;
} subscription_t;

// Database structure
struct ditto_db {
    hash_table_t* table;
    subscription_t subscriptions[MAX_SUBSCRIPTIONS];
    int32_t next_sub_id;
    pthread_mutex_t sub_lock;
};

// ============================================================================
// Hash Table Implementation
// ============================================================================

static uint32_t hash_string(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_SIZE;
}

static hash_table_t* hash_table_create(void) {
    hash_table_t* table = (hash_table_t*)calloc(1, sizeof(hash_table_t));
    if (!table) return NULL;

    pthread_mutex_init(&table->lock, NULL);
    return table;
}

static void hash_table_destroy(hash_table_t* table) {
    if (!table) return;

    for (int i = 0; i < HASH_SIZE; i++) {
        kv_entry_t* entry = table->buckets[i];
        while (entry) {
            kv_entry_t* next = entry->next;
            free(entry->key);
            free(entry->data);
            free(entry);
            entry = next;
        }
    }

    pthread_mutex_destroy(&table->lock);
    free(table);
}

static int32_t hash_table_put(hash_table_t* table, const char* key,
                              const uint8_t* data, size_t len) {
    pthread_mutex_lock(&table->lock);

    uint32_t idx = hash_string(key);
    kv_entry_t* entry = table->buckets[idx];

    // Check if key exists
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Update existing entry
            uint8_t* new_data = (uint8_t*)malloc(len);
            if (!new_data) {
                pthread_mutex_unlock(&table->lock);
                return -1;
            }
            memcpy(new_data, data, len);
            free(entry->data);
            entry->data = new_data;
            entry->len = len;
            pthread_mutex_unlock(&table->lock);
            return 0;
        }
        entry = entry->next;
    }

    // Create new entry
    entry = (kv_entry_t*)calloc(1, sizeof(kv_entry_t));
    if (!entry) {
        pthread_mutex_unlock(&table->lock);
        return -1;
    }

    entry->key = strdup(key);
    entry->data = (uint8_t*)malloc(len);
    if (!entry->key || !entry->data) {
        free(entry->key);
        free(entry->data);
        free(entry);
        pthread_mutex_unlock(&table->lock);
        return -1;
    }

    memcpy(entry->data, data, len);
    entry->len = len;
    entry->next = table->buckets[idx];
    table->buckets[idx] = entry;

    pthread_mutex_unlock(&table->lock);
    return 0;
}

static int32_t hash_table_get(hash_table_t* table, const char* key,
                              uint8_t* out_buf, size_t* inout_len) {
    pthread_mutex_lock(&table->lock);

    uint32_t idx = hash_string(key);
    kv_entry_t* entry = table->buckets[idx];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Found the key
            if (out_buf == NULL) {
                // Query size
                *inout_len = entry->len;
                pthread_mutex_unlock(&table->lock);
                return 1; // Buffer too small
            }

            if (*inout_len < entry->len) {
                // Buffer too small
                *inout_len = entry->len;
                pthread_mutex_unlock(&table->lock);
                return 1;
            }

            // Copy data
            memcpy(out_buf, entry->data, entry->len);
            *inout_len = entry->len;
            pthread_mutex_unlock(&table->lock);
            return 0;
        }
        entry = entry->next;
    }

    pthread_mutex_unlock(&table->lock);
    return 2; // Key not found
}

static int32_t hash_table_delete(hash_table_t* table, const char* key) {
    pthread_mutex_lock(&table->lock);

    uint32_t idx = hash_string(key);
    kv_entry_t* entry = table->buckets[idx];
    kv_entry_t* prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Found the key
            if (prev) {
                prev->next = entry->next;
            } else {
                table->buckets[idx] = entry->next;
            }

            free(entry->key);
            free(entry->data);
            free(entry);
            pthread_mutex_unlock(&table->lock);
            return 0;
        }
        prev = entry;
        entry = entry->next;
    }

    pthread_mutex_unlock(&table->lock);
    return 2; // Key not found
}

// ============================================================================
// Subscription Management
// ============================================================================

static void notify_subscribers(ditto_db_t* db, const char* key) {
    pthread_mutex_lock(&db->sub_lock);

    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (db->subscriptions[i].active) {
            subscription_t* sub = &db->subscriptions[i];
            // Call the callback (may be from a different thread)
            if (sub->callback) {
                sub->callback(sub->user_data, key);
            }
        }
    }

    pthread_mutex_unlock(&db->sub_lock);
}

// ============================================================================
// Public API Implementation
// ============================================================================

DITTO_API int32_t ditto_open(const char* path, ditto_db_t** out_db) {
    if (!path || !out_db) {
        return -1;
    }

    ditto_db_t* db = (ditto_db_t*)calloc(1, sizeof(ditto_db_t));
    if (!db) {
        return -1;
    }

    db->table = hash_table_create();
    if (!db->table) {
        free(db);
        return -1;
    }

    pthread_mutex_init(&db->sub_lock, NULL);
    db->next_sub_id = 1;

    // Initialize subscriptions
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        db->subscriptions[i].active = 0;
    }

    *out_db = db;
    return 0;
}

DITTO_API void ditto_close(ditto_db_t* db) {
    if (!db) return;

    hash_table_destroy(db->table);
    pthread_mutex_destroy(&db->sub_lock);
    free(db);
}

DITTO_API int32_t ditto_put(ditto_db_t* db, const char* key,
                            const uint8_t* data, size_t len) {
    if (!db || !key || !data) {
        return -1;
    }

    int32_t rc = hash_table_put(db->table, key, data, len);
    if (rc == 0) {
        // Notify subscribers on success
        notify_subscribers(db, key);
    }

    return rc;
}

DITTO_API int32_t ditto_get(ditto_db_t* db, const char* key,
                            uint8_t* out_buf, size_t* inout_len) {
    if (!db || !key || !inout_len) {
        return -1;
    }

    return hash_table_get(db->table, key, out_buf, inout_len);
}

DITTO_API int32_t ditto_delete(ditto_db_t* db, const char* key) {
    if (!db || !key) {
        return -1;
    }

    int32_t rc = hash_table_delete(db->table, key);
    if (rc == 0) {
        // Notify subscribers on success
        notify_subscribers(db, key);
    }

    return rc;
}

DITTO_API int32_t ditto_subscribe(ditto_db_t* db, ditto_on_change_cb cb,
                                  void* user_data, int32_t* out_sub_id) {
    if (!db || !cb || !out_sub_id) {
        return -1;
    }

    pthread_mutex_lock(&db->sub_lock);

    // Find an empty subscription slot
    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (!db->subscriptions[i].active) {
            db->subscriptions[i].id = db->next_sub_id++;
            db->subscriptions[i].callback = cb;
            db->subscriptions[i].user_data = user_data;
            db->subscriptions[i].active = 1;

            *out_sub_id = db->subscriptions[i].id;
            pthread_mutex_unlock(&db->sub_lock);
            return 0;
        }
    }

    pthread_mutex_unlock(&db->sub_lock);
    return -1; // No available slots
}

DITTO_API int32_t ditto_unsubscribe(ditto_db_t* db, int32_t sub_id) {
    if (!db) {
        return -1;
    }

    pthread_mutex_lock(&db->sub_lock);

    for (int i = 0; i < MAX_SUBSCRIPTIONS; i++) {
        if (db->subscriptions[i].active && db->subscriptions[i].id == sub_id) {
            db->subscriptions[i].active = 0;
            db->subscriptions[i].callback = NULL;
            db->subscriptions[i].user_data = NULL;
            pthread_mutex_unlock(&db->sub_lock);
            return 0;
        }
    }

    pthread_mutex_unlock(&db->sub_lock);
    return -1; // Subscription not found
}

DITTO_API const char* ditto_version(void) {
    return VERSION;
}
