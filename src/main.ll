; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 10, ptr %x, align 4
  %y = alloca i32, align 4
  store i32 20, ptr %y, align 4
  %z = alloca i32, align 4
  store i32 6, ptr %z, align 4
  %a = alloca i32, align 4
  %0 = load i32, ptr %z, align 4
  %1 = icmp sgt i32 %0, 5
  %2 = load i32, ptr %y, align 4
  %3 = load i32, ptr %x, align 4
  %ternary_result = select i1 %1, i32 %2, i32 %3
  store i32 %ternary_result, ptr %a, align 4
  %4 = load i32, ptr %a, align 4
  %5 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 %4)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)
