/** @file str_json.h
*  @brief Definitions for simple json strings creation
*
* Copyright (C) Wiselab CMU.
* @date July, 2020
*/

#ifndef _STR_JSON_JS_
#define _STR_JSON_JS_

#define STR_JSON_INITIALIZER { .len = 0, .str=NULL, .alloc_size=0 }
 /**
  * @typedef t_str_json
  * @brief json string structure
  * @warning this is the structure returned to javascript; it assumes the *first* four bytes are the length of the string
  */
typedef struct {
  size_t len; // string length
  char *str;
  size_t alloc_size; // allocated size
} t_str_json;

// json format string for errors
extern const char fmt_error[];

// json format string for the detection corners
extern const char fmt_det_point[];

// json format string for the detection with pose
extern const char fmt_det_point_pose[];

/**
 * @brief Init a string
 *
 * @param str_json t_str_json structure to hold the string info
 * @param size_bytes size of the string to allocate
 *
 * @return 0=success; -1 on error
 * @warning Declare strings with: t_str_json a_str = STR_JSON_INITIALIZER;
 * @warning Do not call str_json_create() on a string you already created without destroying it
 */
int str_json_create ( t_str_json *str_json, size_t size_bytes );

/**
 * @brief Free a string
 *
 * @param str_json t_str_json structure with the string info
 *
 * @return 0=success; -1 on error
 */
int str_json_destroy ( t_str_json *str_json );

 /**
  * @brief Clear a string
  *
  * @param str_json t_str_json structure of the string to clear
  */
void str_json_clear ( t_str_json *str_json );

/**
 * @brief Copy the content of source_c_str to str_json_dest
 *
 * Copies up to the terminating null-character, or no more space in dest
 * A terminating null character is always appended to dest.str.
 * IMPORTANT: dest is assumed to be null-terminated
 *
 * @param str_json_dest the destination t_str_json structure with the destination string info
 * @param source_c_str a null-terminated c string (char *) to the source string
 *
 * @return the length, in bytes, of the new string
 */
size_t str_json_concat ( t_str_json *str_json_dest, const char *source_c_str );

/**
 * @brief Composes a string with the same text that would be printed if format was used on printf,
 *
 * @param str_json_dest the destination t_str_json structure with the destination string info
 * @param format a format string
 * @param ... additional arguments
 *
 * @return The number of characters that would have been written if str_json_dest had been sufficiently large, not counting the terminating null character.
 */
size_t str_json_printf ( t_str_json *str_json_dest, const char *format, ... );

#endif
