; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [4 x i8] c"%d\0A\00"

define i32 @main() {
entry:
  %i = alloca i32, align 4
  store i32 3, ptr %i, align 4
  br label %loop_start

loop_start:                                       ; preds = %loop_body, %entry
  %0 = load i32, ptr %i, align 4
  %1 = icmp sgt i32 %0, 0
  br i1 %1, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_start
  %2 = load i32, ptr %i, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 %2)
  %4 = load i32, ptr %i, align 4
  %5 = sub i32 %4, 1
  store i32 %5, ptr %i, align 4
  br label %loop_start

loop_end:                                         ; preds = %loop_start
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @strcmp(ptr, ptr)
Modified Node #1:
Original: for (i = 3; i > 0; --i) { { print(i); } }
Modified: { print(i); print(i); print(i); }

