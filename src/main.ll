; ModuleID = 'main'
source_filename = "main"

@.int_fmt = private constant [3 x i8] c"%d\00"

define i32 @main() {
entry:
  br i1 true, label %pow_loop, label %pow_end

pow_loop:                                         ; preds = %pow_body, %entry
  %result = phi i32 [ 1, %entry ], [ %pow_mult, %pow_body ]
  %exp = phi i32 [ 4, %entry ], [ %exp_decr, %pow_body ]
  %exp_positive = icmp sgt i32 %exp, 0
  br i1 %exp_positive, label %pow_body, label %pow_end

pow_body:                                         ; preds = %pow_loop
  %pow_mult = mul i32 %result, -2
  %exp_decr = sub i32 %exp, 1
  br label %pow_loop

pow_end:                                          ; preds = %pow_loop, %entry
  %final_result = phi i32 [ 1, %entry ], [ %result, %pow_loop ]
  %0 = call i32 (ptr, ...) @printf(ptr @.int_fmt, i32 %final_result)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)