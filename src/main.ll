; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [5 x i8] c"zero\00", align 1
@.str_fmt = private constant [4 x i8] c"%s\0A\00"
@1 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@.str_fmt.1 = private constant [4 x i8] c"%s\0A\00"
@2 = private unnamed_addr constant [6 x i8] c"other\00", align 1
@.str_fmt.2 = private constant [4 x i8] c"%s\0A\00"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 1, ptr %x, align 4
  %0 = load i32, ptr %x, align 4
  %case_cmp = icmp eq i32 %0, 0
  br i1 %case_cmp, label %case_0, label %case_cond_1

after_match:                                      ; preds = %case_2, %case_1, %case_0
  ret i32 0

case_0:                                           ; preds = %entry
  %1 = call i32 (ptr, ...) @printf(ptr @.str_fmt, ptr @0)
  br label %after_match

case_1:                                           ; preds = %case_cond_1
  %2 = call i32 (ptr, ...) @printf(ptr @.str_fmt.1, ptr @1)
  br label %after_match

case_2:                                           ; preds = %case_cond_1
  %3 = call i32 (ptr, ...) @printf(ptr @.str_fmt.2, ptr @2)
  br label %after_match

case_cond_1:                                      ; preds = %entry
  %case_cmp1 = icmp eq i32 %0, 1
  br i1 %case_cmp1, label %case_1, label %case_2
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @strcmp(ptr, ptr)
