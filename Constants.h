
// Recommended number of bytes per transmittable packet
#define CHUNK_SIZE_IN_BYTES 1400

// Size of our arbitrary byte data (which I assume is a movie here).
// This is equivalent to a GB of data
#define DATA_SIZE_IN_BYTES 1000000

#define LEFTOVER_SIZE_IN_BYTES (DATA_SIZE_IN_BYTES % CHUNK_SIZE_IN_BYTES)

#define DATA_SIZE_IN_CHUNKS ((DATA_SIZE_IN_BYTES / CHUNK_SIZE_IN_BYTES) + 1)

#define MANAGER_SEND_FREQ 3

#define TAG_DATA_REQUEST 0x11000000

#define TAG_WORK_ORDER 0x22000000

#define TAG_TERMINATION_NOTICE 0x33000000
