; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %nums = alloca ptr, align 8
  %0 = call ptr @malloc(i32 12)
  %1 = getelementptr i32, ptr %0, i32 0
  store i32 1, ptr %1, align 4
  %2 = getelementptr i32, ptr %0, i32 1
  store i32 2, ptr %2, align 4
  %3 = getelementptr i32, ptr %0, i32 2
  store i32 3, ptr %3, align 4
  store ptr %0, ptr %nums, align 8
  %x = alloca i32, align 4
  store i32 0, ptr %x, align 4
  br label %loop_start

loop_start:                                       ; preds = %loop_body, %entry
  %4 = load i32, ptr %x, align 4
  %5 = icmp slt i32 %4, 10
  br i1 %5, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_start
  %6 = load i32, ptr %x, align 4
  %7 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 %6)
  %8 = load i32, ptr %x, align 4
  %9 = add i32 %8, 1
  store i32 %9, ptr %x, align 4
  br label %loop_start

loop_end:                                         ; preds = %loop_start
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)