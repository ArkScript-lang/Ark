// IO

#define IO_WRITE_VE_1 "writeFile: mode must be equal to \"a\" or \"w\""

// List

#define LIST_FIND_ARITY "list:find needs 2 arguments: list, value"
#define LIST_FIND_TE0 "list:find: list must be a List"

// TO BE DEPRECATED
#define LIST_RMAT_ARITY "list:removeAt needs 2 arguments: list, index"
#define LIST_RMAT_TE0 "list:removeAt: list must be a List"
#define LIST_RMAT_TE1 "list:removeAt: index must be a Number"
#define LIST_RMAT_OOR "list:removeAt: index out of range"
// --

#define LIST_SLICE_ARITY "list:slice needs 4 arguments: list, start, end, step"
#define LIST_SLICE_TE0 "list:slice: list must be a List"
#define LIST_SLICE_TE1 "list:slice: start must be a Number"
#define LIST_SLICE_TE2 "list:slice: end must be a Number"
#define LIST_SLICE_TE3 "list:slice: step must be a Number"
#define LIST_SLICE_STEP "list:slice: step can not be null"
#define LIST_SLICE_ORDER "list:slice: start position must be less or equal to the end position"
#define LIST_SLICE_OOR "list:slice: indices out of range"

#define LIST_SORT_ARITY "list:sort needs 1 argument: a list"
#define LIST_SORT_TE0 "list:sort: list must be a List"

#define LIST_FILL_ARITY "list:fill needs 2 arguments: size, value"
#define LIST_FILL_TE0 "list:fill: size must be a Number"

#define LIST_SETAT_ARITY "list:setAt needs 3 arguments: list, index, value"
#define LIST_SETAT_TE0 "list:setAt: list must be a List"
#define LIST_SETAT_TE1 "list:setAt: index must be a Number"

// Mathematics

#define MATH_ARITY(name) (name " needs 1 argument: value")
#define MATH_TE0(name) (name ": value must be a Number")

// String

#define STR_FORMAT_ARITY "str:format needs at least 1 argument: string, [values...]"
#define STR_FORMAT_TE0 "str:format: string must be a String"

#define STR_FIND_ARITY "str:find needs 2 arguments: string, substr"
#define STR_FIND_TE0 "str:find: string must be a String"
#define STR_FIND_TE1 "str:find: substr must be a String"

#define STR_RM_ARITY "str:removeAt needs 2 arguments: string, index"
#define STR_RM_TE0 "str:removeAt: string must be a String"
#define STR_RM_TE1 "str:removeAt: index must be a Number"
#define STR_RM_OOR "str:removeAt: index out of range"

#define STR_ORD_ARITY "str:ord needs at least 1 argument: string"
#define STR_ORD_TE0 "str:ord: string must be a String"

#define STR_CHR_ARITY "str:chr needs at least 1 argument: codepoint"
#define STR_CHR_TE0 "str:chr: codepoint must be a Number"

// System

#define SYS_SYS_ARITY "sys:exec needs 1 argument: command"
#define SYS_SYS_TE0 "sys:exec: command must be a String"

#define SYS_SLEEP_ARITY "sleep needs 1 argument: duration (milliseconds)"
#define SYS_SLEEP_TE0 "sleep: duration must be a Number"

#define SYS_EXIT_ARITY "sys:exit needs 1 argument: exit code"
#define SYS_EXIT_TE0 "sys:exit: exit code must be a Number"

// Time
