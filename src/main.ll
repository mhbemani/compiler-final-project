; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  store i32 1, ptr %i, align 4
  br label %loop_start

loop_start:                                       ; preds = %loop_body, %entry
  %0 = load i32, ptr %i, align 4
  %1 = icmp slt i32 %0, 4
  br i1 %1, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_start
  %2 = load i32, ptr %i, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @0, i32 %2)
  %4 = load i32, ptr %i, align 4
  %5 = sub i32 %4, 1
  store i32 %5, ptr %i, align 4
  br label %loop_start

loop_end:                                         ; preds = %loop_start
  ret i32 0
}

declare i32 @printf(ptr, ...)