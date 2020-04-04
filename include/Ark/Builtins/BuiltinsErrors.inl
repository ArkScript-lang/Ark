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

#define LIST_REVERSE_ARITY "reverseList needs 1 argument: list"
#define LIST_REVERSE_TE0   "reverseLists: list must be a List"

#define LIST_FIND_ARITY "findInList needs 2 arguments: list, value"
#define LIST_FIND_TE0   "findInList: list must be a List"

#define LIST_RMAT_ARITY "removeAtList needs 2 arguments: list, index"
#define LIST_RMAT_TE0   "removeAtList: list must be a List"
#define LIST_RMAT_TE1   "removeAtList: index must be a Number"
#define LIST_RMAT_OOR   "removeAtList: index out of range"

#define LIST_SLICE_ARITY "sliceList needs 4 arguments: list, start, end, step"
#define LIST_SLICE_TE0   "sliceList: list must be a List"
#define LIST_SLICE_TE1   "sliceList: start must be a Number"
#define LIST_SLICE_TE2   "sliceList: end must be a Number"
#define LIST_SLICE_TE3   "sliceList: step must be a Number"
#define LIST_SLICE_STEP  "sliceList: step can not be null"
#define LIST_SLICE_ORDER "sliceList: start position must be less or equal to the end position"
#define LIST_SLICE_OOR   "sliceList: indices out of range"

#define LIST_SORT_ARITY "sort needs 1 argument: a list"
#define LIST_SORT_TE0   "sort: list must be a List"

#define LIST_FILL_ARITY "fill needs 2 arguments: size, value"
#define LIST_FILL_TE0   "fill: size must be a Number"

#define LIST_SETAT_ARITY "setListAt needs 3 arguments: list, index, value"
#define LIST_SETAT_TE0   "setListAt: list must be a List"
#define LIST_SETAT_TE1   "setListAt: index must be a Number"

// Mathmatics

#define MATH_ARITY(name) (name " needs 1 argument: value")
#define MATH_TE0(name)   (name ": value must be a Number")

// String

#define STR_FORMAT_ARITY "format needs at least 1 argument: string, [values...]"
#define STR_FORMAT_TE0   "format: string must be a String"
#define STR_FORMAT_TE1   "format: value should be a String or a Number"

#define STR_FIND_ARITY "findSubStr needs 2 arguments: string, substr"
#define STR_FIND_TE0   "findSubStr: string must be a String"
#define STR_FIND_TE1   "findSubStr: substr must be a String"

#define STR_RM_ARITY "removeAtStr needs 2 arguments: string, index"
#define STR_RM_TE0   "removeAtStr: string must be a String"
#define STR_RM_TE1   "removeAtStr: index must be a Number"
#define STR_RM_OOR   "removeAtStr: index out of range"

// System

#define SYS_SYS_ARITY "system needs 1 argument: command"
#define SYS_SYS_TE0   "system: command must be a String"

// Time

#define TIME_SLEEP_ARITY "sleep needs 1 argument: duration (milliseconds)"
#define TIME_SLEEP_TE0   "sleep: duration must be a Number"