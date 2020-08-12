/** @file str_json.c
 *  @brief Simple json strings creation
 *
 *  @author Nuno Pereira; CMU (this file)
 *  @date Jul, 2020
 */
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include "str_json.h"

// json format string for errors
const char fmt_error[] = "{ \"result\": \"%s\" }";

// json format string for the detection corners
const char fmt_det_point[] = "{\"id\":%d, \"size\":%.2f, \"corners\": [{\"x\":%.2f,\"y\":%.2f},{\"x\":%.2f,\"y\":%.2f},{\"x\":%.2f,\"y\":%.2f},{\"x\":%.2f,\"y\":%.2f}], \"center\": {\"x\":%.2f,\"y\":%.2f} }";

// json format string for the detection with pose
const char fmt_det_point_pose[] = "{\"id\":%d, \"size\":%.2f, \"corners\": [{\"x\":%.2f,\"y\":%.2f},{\"x\":%.2f,\"y\":%.2f},{\"x\":%.2f,\"y\":%.2f},{\"x\":%.2f,\"y\":%.2f}], \"center\": {\"x\":%.2f,\"y\":%.2f}, \"pose\": { \"R\": [[%f,%f,%f],[%f,%f,%f],[%f,%f,%f]], \"t\": [%f,%f,%f], \"e\": %f, %s } }";

/** @copydoc str_json_create */
int str_json_create ( t_str_json *str_json, size_t size_bytes ) {
   if (str_json->str != NULL) return -1;
   if (str_json->alloc_size != 0) return -1;
   if (str_json->len != 0) return -1;
   if (size_bytes == 0) return -1;
   str_json->alloc_size=size_bytes;
   str_json->str = malloc(size_bytes+1); // allocate 1 byte extra
   if (str_json->str == NULL) return -1;
   str_json->len = 0;
   str_json->str[0] = '\0';
   return 0;
 }

/** @copydoc str_json_destroy */
int str_json_destroy ( t_str_json *str_json ) {
   if (str_json->str==NULL) return -1;
   free(str_json->str);
   str_json->str = NULL;
   str_json->alloc_size = 0;
   str_json->len = 0;
   return 0;
 }

/** @copydoc str_json_clear */
void str_json_clear ( t_str_json *str_json ) {
   if (str_json->alloc_size == 0) return;
   if (str_json->str == NULL) return;
   str_json->str[0] = '\0';
   str_json->len = 0;
}

/** @copydoc str_json_concat */
size_t str_json_concat ( t_str_json *str_json_dest, const char *source_c_str ) {
  if (str_json_dest->alloc_size == 0) return 0;
  if (str_json_dest->str == NULL) return 0;
  if (source_c_str == NULL) return 0;
  if (str_json_dest->alloc_size == str_json_dest->len) return str_json_dest->len;
  assert(str_json_dest->alloc_size >= str_json_dest->len); // should never be smaller

  int num = str_json_dest->alloc_size - str_json_dest->len;
  assert(num >= 0);
  strncat(str_json_dest->str, source_c_str, num); // strncat should always add a terminating null character
  str_json_dest->str[str_json_dest->alloc_size]='\0'; // we add our own too
  str_json_dest->len = strlen(str_json_dest->str);

  assert(str_json_dest->alloc_size >= str_json_dest->len); // should never be smaller
  return str_json_dest->len;
}

/** @copydoc str_json_printf */
size_t str_json_printf ( t_str_json *str_json_dest, const char *format, ... ) {
  if (str_json_dest->alloc_size == 0) return 0;
  if (str_json_dest->str == NULL) return 0;
  if (str_json_dest->alloc_size == str_json_dest->len) return str_json_dest->len;
  assert(str_json_dest->alloc_size >= str_json_dest->len); // should never be smaller

  int num = str_json_dest->alloc_size - str_json_dest->len;
  va_list args;
  va_start (args, format);
  vsnprintf (str_json_dest->str, num, format, args);
  va_end (args);
  str_json_dest->str[str_json_dest->alloc_size]='\0'; // we add our own too (string is allocated with +1)
  str_json_dest->len = strlen(str_json_dest->str);

  assert(str_json_dest->alloc_size >= str_json_dest->len); // should never be smaller
  return str_json_dest->len;
}
