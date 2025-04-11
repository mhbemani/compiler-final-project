; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [6 x i8] c"rqual\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 3, ptr %a, align 4
  %0 = load i32, ptr %a, align 4
  %1 = icmp sle i32 %0, 2
  br i1 %1, label %then, label %else

then:                                             ; preds = %entry
  %2 = call i32 (ptr, ...) @printf(ptr @2, ptr @0)
  br label %after_if_else

else:                                             ; preds = %entry
  %3 = call i32 (ptr, ...) @printf(ptr @1, i32 0)
  br label %after_if_else

after_if_else:                                    ; preds = %else, %then
  ret i32 0
}

declare i32 @printf(ptr, ...)