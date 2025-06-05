; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"
@.int_fmt.1 = private constant [4 x i8] c"%d\0A\00"
@.int_fmt.2 = private constant [4 x i8] c"%d\0A\00"
@.int_fmt.3 = private constant [4 x i8] c"%d\0A\00"
@.int_fmt.4 = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 0, ptr %a, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 1)
  %1 = call i32 (ptr, ...) @printf(ptr @.int_fmt.1, i32 1)
  %2 = call i32 (ptr, ...) @printf(ptr @.int_fmt.2, i32 1)
  %3 = call i32 (ptr, ...) @printf(ptr @.int_fmt.3, i32 1)
  %4 = call i32 (ptr, ...) @printf(ptr @.int_fmt.4, i32 7809)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @strcmp(ptr, ptr)
