; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt = private constant [4 x i8] c"%s\0A\00"
@1 = private unnamed_addr constant [4 x i8] c"bye\00", align 1
@.str_fmt.1 = private constant [4 x i8] c"%s\0A\00"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 3, ptr %a, align 4
  %0 = load i32, ptr %a, align 4
  %case_cmp = icmp eq i32 %0, 2
  br i1 %case_cmp, label %case_0, label %case_cond_1

after_match:                                      ; preds = %case_1, %case_0, %case_cond_1
  ret i32 0

case_0:                                           ; preds = %entry
  %1 = call i32 (ptr, ...) @printf(ptr @.str_fmt, ptr @0)
  br label %after_match

case_1:                                           ; preds = %case_cond_1
  %2 = call i32 (ptr, ...) @printf(ptr @.str_fmt.1, ptr @1)
  br label %after_match

case_cond_1:                                      ; preds = %entry
  %case_cmp1 = icmp eq i32 %0, 3
  br i1 %case_cmp1, label %case_1, label %after_match
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @strcmp(ptr, ptr)
