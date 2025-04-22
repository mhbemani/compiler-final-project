; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 -2, ptr %a, align 4
  store i32 4, ptr %a, align 4
  %0 = load i32, ptr %a, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 %0)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @strcmp(ptr, ptr)
