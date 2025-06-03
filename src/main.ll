; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"
@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt = private constant [4 x i8] c"%s\0A\00"
@.int_fmt.1 = private constant [4 x i8] c"%d\0A\00"
@1 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt.2 = private constant [4 x i8] c"%s\0A\00"
@.int_fmt.3 = private constant [4 x i8] c"%d\0A\00"
@2 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt.4 = private constant [4 x i8] c"%s\0A\00"
@.int_fmt.5 = private constant [4 x i8] c"%d\0A\00"
@3 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt.6 = private constant [4 x i8] c"%s\0A\00"
@.int_fmt.7 = private constant [4 x i8] c"%d\0A\00"
@4 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt.8 = private constant [4 x i8] c"%s\0A\00"

define i32 @main() {
entry:
  %0 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 5)
  %1 = call i32 (ptr, ...) @printf(ptr @.str_fmt, ptr @0)
  %2 = call i32 (ptr, ...) @printf(ptr @.int_fmt.1, i32 6)
  %3 = call i32 (ptr, ...) @printf(ptr @.str_fmt.2, ptr @1)
  %4 = call i32 (ptr, ...) @printf(ptr @.int_fmt.3, i32 7)
  %5 = call i32 (ptr, ...) @printf(ptr @.str_fmt.4, ptr @2)
  %6 = call i32 (ptr, ...) @printf(ptr @.int_fmt.5, i32 8)
  %7 = call i32 (ptr, ...) @printf(ptr @.str_fmt.6, ptr @3)
  %8 = call i32 (ptr, ...) @printf(ptr @.int_fmt.7, i32 9)
  %9 = call i32 (ptr, ...) @printf(ptr @.str_fmt.8, ptr @4)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @strcmp(ptr, ptr)
