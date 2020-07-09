// IO

#define IO_INPUT_TE "input: prompt must be a String"

#define IO_WRITE_ARITY "writeFile needs 2 to 3 arguments: filename, [mode], content"
#define IO_WRITE_TE0   "writeFile: filename must be a String"
#define IO_WRITE_TE1   "writeFile: mode must be a String"
#define IO_WRITE_VE_1  "writeFile: mode must be equal to \"a\" or \"w\""

#define IO_READ_ARITY "readFile needs 1 argument: filename"
#define IO_READ_TE0   "readFile: filename must be a String"

#define IO_EXISTS_ARITY "fileExists? needs 1 argument: filename"
#define IO_EXISTS_TE0   "fileExists?: filename must be a String"

#define IO_LS_ARITY "listFiles needs 1 argument: filename"
#define IO_LS_TE0   "listFiles: filename must be a String"

#define IO_ISDIR_ARITY "isDir? needs 1 argument: path"
#define IO_ISDIR_TE0   "isDir?: path must be a String"

#define IO_MKD_ARITY "makeDir needs 1 argument: filename"
#define IO_MKD_TE0   "makeDir: filename must be a String"

#define IO_RM_ARITY "removeFiles needs at least 1 argument: filename [...]"
#define IO_RM_TE0   "removeFiles: filename must be a String"

// List

#define LIST_APPEND_ARITY "append needs at least 2 arguments: list, content [...]"
#define LIST_APPEND_TE0   "append: list must be a List"

#define LIST_CONCAT_ARITY "concat needs at least 2 arguments: list [...]"
#define LIST_CONCAT_TE    "concat: list must be a List"

#define LIST_REVERSE_ARITY "list:reverse needs 1 argument: list"
#define LIST_REVERSE_TE0   "list:reverse: list must be a List"

#define LIST_FIND_ARITY "list:find needs 2 arguments: list, value"
#define LIST_FIND_TE0   "list:find: list must be a List"

#define LIST_RMAT_ARITY "list:removeAt needs 2 arguments: list, index"
#define LIST_RMAT_TE0   "list:removeAt: list must be a List"
#define LIST_RMAT_TE1   "list:removeAt: index must be a Number"
#define LIST_RMAT_OOR   "list:removeAt: index out of range"

#define LIST_SLICE_ARITY "list:slice needs 4 arguments: list, start, end, step"
#define LIST_SLICE_TE0   "list:slice: list must be a List"
#define LIST_SLICE_TE1   "list:slice: start must be a Number"
#define LIST_SLICE_TE2   "list:slice: end must be a Number"
#define LIST_SLICE_TE3   "list:slice: step must be a Number"
#define LIST_SLICE_STEP  "list:slice: step can not be null"
#define LIST_SLICE_ORDER "list:slice: start position must be less or equal to the end position"
#define LIST_SLICE_OOR   "list:slice: indices out of range"

#define LIST_SORT_ARITY "list:sort needs 1 argument: a list"
#define LIST_SORT_TE0   "list:sort: list must be a List"

#define LIST_FILL_ARITY "list:fill needs 2 arguments: size, value"
#define LIST_FILL_TE0   "list:fill: size must be a Number"

#define LIST_SETAT_ARITY "list:setAt needs 3 arguments: list, index, value"
#define LIST_SETAT_TE0   "list:setAt: list must be a List"
#define LIST_SETAT_TE1   "list:setAt: index must be a Number"

// Mathmatics

#define MATH_ARITY(name) (name " needs 1 argument: value")
#define MATH_TE0(name)   (name ": value must be a Number")

// String

#define STR_FORMAT_ARITY "str:format needs at least 1 argument: string, [values...]"
#define STR_FORMAT_TE0   "str:format: string must be a String"

#define STR_FIND_ARITY "str:find needs 2 arguments: string, substr"
#define STR_FIND_TE0   "str:find: string must be a String"
#define STR_FIND_TE1   "str:find: substr must be a String"

#define STR_RM_ARITY "str:removeAt needs 2 arguments: string, index"
#define STR_RM_TE0   "str:removeAt: string must be a String"
#define STR_RM_TE1   "str:removeAt: index must be a Number"
#define STR_RM_OOR   "str:removeAt: index out of range"

// System

#define SYS_SYS_ARITY "sys:exec needs 1 argument: command"
#define SYS_SYS_TE0   "sys:exec: command must be a String"

#define SYS_SLEEP_ARITY "sleep needs 1 argument: duration (milliseconds)"
#define SYS_SLEEP_TE0   "sleep: duration must be a Number"

// Time