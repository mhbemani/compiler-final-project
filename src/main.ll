; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.str_fmt = private constant [4 x i8] c"%s\0A\00"
@1 = private unnamed_addr constant [8 x i8] c"Error: \00", align 1
@.str_fmt.1 = private constant [4 x i8] c"%s\0A\00"

define i32 @main() personality ptr @__gxx_personality_v0 {
entry:
  br label %try

try:                                              ; preds = %entry
  %asd = alloca ptr, align 8
  store ptr @0, ptr %asd, align 8
  store i32 4, ptr %asd, align 4
  %0 = load ptr, ptr %asd, align 8
  %1 = call i32 (ptr, ...) @printf(ptr @.str_fmt, ptr %0)
  br label %after_try_catch

catch:                                            ; No predecessors!
  %landingpad = landingpad { ptr, i32 }
          catch ptr null
  %exception = extractvalue { ptr, i32 } %landingpad, 0
  %e = alloca ptr, align 8
  store ptr %exception, ptr %e, align 8
  %2 = load ptr, ptr %e, align 8
  br label %concat

after_try_catch:                                  ; preds = %after_concat, %try
  ret i32 0

concat:                                           ; preds = %catch
  %leftLen = call i32 @strlen(ptr @1)
  %rightLen = call i32 @strlen(ptr %2)
  %totalLenNoNull = add i32 %leftLen, %rightLen
  %totalLen = add i32 %totalLenNoNull, 1
  %concatResult = call ptr @malloc(i32 %totalLen)
  %3 = add i32 %leftLen, 1
  %4 = call ptr @memcpy(ptr %concatResult, ptr @1, i32 %3)
  %destOffset = getelementptr i8, ptr %concatResult, i32 %leftLen
  %5 = add i32 %rightLen, 1
  %6 = call ptr @memcpy(ptr %destOffset, ptr %2, i32 %5)
  br label %after_concat

after_concat:                                     ; preds = %concat
  %concat_result = phi ptr [ %concatResult, %concat ]
  %7 = call i32 (ptr, ...) @printf(ptr @.str_fmt.1, ptr %concat_result)
  br label %after_try_catch
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)

declare i32 @__gxx_personality_v0(...)
