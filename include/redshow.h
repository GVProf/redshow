#ifndef REDSHOW_H
#define REDSHOW_H

#include <gpu-patch.h>

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

typedef enum redshow_analysis_type {
  REDSHOW_ANALYSIS_UNKNOWN = 0,
  REDSHOW_ANALYSIS_SPATIAL_REDUNDANCY = 1,
  REDSHOW_ANALYSIS_TEMPORAL_REDUNDANCY = 2,
  REDSHOW_ANALYSIS_VALUE_FLOW = 3
} redshow_analysis_type_t;

typedef enum redshow_access_type {
  REDSHOW_ACCESS_UNKNOWN = 0,
  REDSHOW_ACCESS_READ = 1,
  REDSHOW_ACCESS_WRITE = 2
} redshow_access_type_t;

typedef enum redshow_data_type {
  REDSHOW_DATA_UNKNOWN = 0,
  REDSHOW_DATA_FLOAT = 1,
  REDSHOW_DATA_INT = 2
} redshow_data_type_t;

typedef enum redshow_memory_type {
  REDSHOW_MEMORY_UNKNOWN = 0,
  REDSHOW_MEMORY_SHARED = 1,
  REDSHOW_MEMORY_LOCAL = 2,
  REDSHOW_MEMORY_GLOBAL = 3,
  REDSHOW_MEMORY_CONSTANT = 4,
  REDSHOW_MEMORY_UVM = 5,
  REDSHOW_MEMORY_HOST = 6
} redshow_memory_type_t;

typedef enum redshow_result {
  REDSHOW_SUCCESS = 0,
  REDSHOW_ERROR_NOT_IMPL = 1,
  REDSHOW_ERROR_NOT_EXIST_ENTRY = 2,
  REDSHOW_ERROR_DUPLICATE_ENTRY = 3,
  REDSHOW_ERROR_NOT_REGISTER_CALLBACK = 4,
  REDSHOW_ERROR_NO_SUCH_FILE = 5,
  REDSHOW_ERROR_FAILED_ANALYZE_CUBIN = 6,
  REDSHOW_ERROR_FAILED_ANALYZE_TRACE = 7,
  REDSHOW_ERROR_NO_SUCH_APPROX = 8,
  REDSHOW_ERROR_NO_SUCH_DATA_TYPE = 9
} redshow_result_t;

typedef enum redshow_approx_level {
  REDSHOW_APPROX_NONE = 0,
  REDSHOW_APPROX_MIN = 1,
  REDSHOW_APPROX_LOW = 2,
  REDSHOW_APPROX_MID = 3,
  REDSHOW_APPROX_HIGH = 4,
  REDSHOW_APPROX_MAX = 5,
} redshow_approx_level_t;

typedef struct redshow_record_view {
  uint32_t function_index;
  uint64_t pc_offset;
  uint64_t memory_id;
  uint64_t memory_op_id;
  uint64_t red_count;
  uint64_t access_count;
} redshow_record_view_t;

typedef struct redshow_record_data {
  uint32_t num_views;
  redshow_analysis_type_t analysis_type;
  redshow_access_type_t access_type;
  redshow_record_view_t *views;
} redshow_record_data_t;

/*
 * Config default data type
 * 
 * Thread-Safety: NO
 */
EXTERNC redshow_result_t redshow_data_type_config(redshow_data_type_t data_type);

/*
 * Get default data type
 * 
 * Thread-Safety: NO
 */
EXTERNC redshow_result_t redshow_data_type_get(redshow_data_type_t *data_type);

/*
 * Config floating point redundancy approximate level
 * 
 * Thread-Safety: NO
 */
EXTERNC redshow_result_t redshow_approx_level_config(redshow_approx_level_t level);

/*
 * This function is used to setup specific analysis types.
 * 
 * Thread-Safety: NO
 */
EXTERNC redshow_result_t redshow_analysis_enable(redshow_analysis_type_t analysis_type);

/*
 * This function is used to cancel specific analysis types.
 * 
 * Thread-Safety: NO
 */
EXTERNC redshow_result_t redshow_analysis_disable(redshow_analysis_type_t analysis_type);

/*
 * This function is used to register a cubin module.
 * redshow analyzes a cubin module to extract CFGs and instruction statistics.
 *
 * cubin_id:
 * Unique identifier for cubins
 *
 * cubin_offset:
 * hpcrun cubin offset
 *
 * nsymbols:
 * Number of symbols in cubin
 *
 * symbols:
 * An array of symbol start addresses in memory.
 * Use 0 for non-function symbols
 * 
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_cubin_register(uint32_t cubin_id, uint32_t nsymbols, uint64_t *symbol_pcs, const char *path);

/*
 * For a large-scale program that loads a large number of CUBINs, we do not analyze every of them because not all
 * the cubins will be used.
 *
 * Instead, we cache the cubin's symbols and path and analyze the cubins we use
 */
EXTERNC redshow_result_t redshow_cubin_cache_register(uint32_t cubin_id, uint32_t nsymbols, uint64_t *symbol_pcs, const char *path);

/*
 * This function is used to unregister a module.
 * 
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_cubin_unregister(uint32_t cubin_id);

/*
 * This function is used to register a global memory region.
 * 
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_memory_register(uint64_t start, uint64_t end, uint64_t host_op_id, uint64_t memory_id);

/*
 * This function is used to unregister a global memory region.
 * 
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_memory_unregister(uint64_t start, uint64_t end, uint64_t host_op_id);

/*
 * This funciton is used to query the address of a shadow memory
 * 
 * Thread-Safety: YES
 *
 * host_op_id:
 * Unique identifier of the current timestamp
 *
 * start:
 * The address of the memory object
 *
 * memory_id:
 * The calling context of the memory object
 *
 * memory_addr:
 * The shadow memory address of the memory object
 *
 * len:
 * The size of the memory object
 *
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_memory_query(uint64_t host_op_id, uint64_t start, uint64_t *memory_id, uint64_t *memory_addr, uint64_t *len);

/*
 * This funciton is used to track a memcpy operation
 * 
 * memcpy_id: 
 * Unique identifier of a memcpy operation
 *
 * src_memory_id:
 * Unique identifier of a src memory object except for MEMORY_ID_HOST as we do not track host memory objects
 *
 * dst_memory_id:
 * Unique identifier of a dst memory object except for MEMORY_ID_HOST as we do not track host memory objects
 *
 * len:
 * Number of copied bytes
 *
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_memcpy_register(uint64_t memcpy_id, uint64_t src_memory_id, uint64_t dst_memory_id, uint64_t len);

/*
 * This funciton is used to track a memset operation
 * 
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_memset_register(uint64_t memset_id, uint64_t memory_id, uint64_t len);

/*
 * Let a user handle data when a trace log is done analyzing
 *
 * Thread-Safety: NO
 */
typedef void (*redshow_log_data_callback_func)(uint64_t kernel_id, gpu_patch_buffer_t *trace_data);

EXTERNC redshow_result_t redshow_log_data_callback_register(redshow_log_data_callback_func func);

/*
 * Let a user get overview data for all kernels when the program is finished.
 *
 * Thread-Safety: NO
 */

typedef void (*redshow_record_data_callback_func)(uint32_t cubin_id, uint64_t kernel_id, redshow_record_data_t *record_data);

EXTERNC redshow_result_t redshow_record_data_callback_register(redshow_record_data_callback_func func, uint32_t pc_views_limit, uint32_t mem_views_limit);


/*
 * Apply registered analysis to a gpu trace, analysis results are buffered.
 * redshow_callback_func is called when the analysis is done.
 * Multi-threading is enable by `export OMP_NUM_THREADS=N.`
 *
 * First use binary search to find an enclosed region of function addresses
 * instruction_offset = instruction_pc - function_address
 *
 * thread_id:
 * Which thread launches the kernel
 *
 * cubin_id:
 * Lookup correponding cubin
 
 * kernel_id:
 * Unique identifier for a calling context
 *
 * host_op_id:
 * Unique identifier for the operation
 *
 * trace_data:
 * GPU memory trace for a single kernel launch.
 *
 * Thread-Safety: YES
 */
EXTERNC redshow_result_t redshow_analyze(uint32_t thread_id, uint32_t cubin_id, uint64_t kernel_id, uint64_t host_op_id,
  gpu_patch_buffer_t *trace_data);

/*
 * Mark the begin of the current analysis region
 */
EXTERNC redshow_result_t redshow_analysis_begin();

/*
 * Mark the end of the current analysis region
 */
EXTERNC redshow_result_t redshow_analysis_end();

/*
 * Flush back all the result.
 * This function is supposed to be called when all the analysis and kernel launches are done.
 *
 * Thread-Safety: NO
 */
EXTERNC redshow_result_t redshow_flush(uint32_t thread_id);

#endif  // REDSHOW_H
