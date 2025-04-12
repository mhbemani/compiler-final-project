; ModuleID = 'main'
source_filename = "main"

@0 = private unnamed_addr constant [6 x i8] c"conca\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"tination\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

define i32 @main() {
entry:
  %a = alloca ptr, align 8
  store ptr @0, ptr %a, align 8
  %b = alloca ptr, align 8
  store ptr @1, ptr %b, align 8
  %0 = load ptr, ptr %a, align 8
  %1 = load ptr, ptr %b, align 8
  %leftLen = call i32 @strlen(ptr %0)
  %rightLen = call i32 @strlen(ptr %1)
  %totalLenNoNull = add i32 %leftLen, %rightLen
  %totalLen = add i32 %totalLenNoNull, 1
  %concatResult = call ptr @malloc(i32 %totalLen)
  %2 = add i32 %leftLen, 1
  %3 = call ptr @memcpy(ptr %concatResult, ptr %0, i32 %2)
  %destOffset = getelementptr i8, ptr %concatResult, i32 %leftLen
  %4 = add i32 %rightLen, 1
  %5 = call ptr @memcpy(ptr %destOffset, ptr %1, i32 %4)
  %6 = call i32 (ptr, ...) @printf(ptr @3, ptr %concatResult)
  ret i32 0
}

declare i32 @printf(ptr, ...)

declare i32 @strlen(ptr)

declare ptr @malloc(i32)

declare ptr @memcpy(ptr, ptr, i32)