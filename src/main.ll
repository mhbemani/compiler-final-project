; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"
@.int_fmt.1 = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 4, ptr %a, align 4
  %b = alloca i32, align 4
  %0 = load i32, ptr %a, align 4
  %1 = add i32 %0, 1
  store i32 %1, ptr %a, align 4
  store i32 %0, ptr %b, align 4
  %2 = load i32, ptr %a, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 %2)
  %4 = load i32, ptr %b, align 4
  %5 = call i32 (ptr, ...) @printf(ptr @.int_fmt.1, i32 %4)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)
