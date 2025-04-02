; ModuleID = './test/test.c'
source_filename = "./test/test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"Algebraic Identity\0A\00", align 1
@.str.1 = private unnamed_addr constant [21 x i8] c"Il risultato \C3\A8: %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [20 x i8] c"Strength Reduction\0A\00", align 1
@.str.3 = private unnamed_addr constant [22 x i8] c"Inserisci un valore: \00", align 1
@.str.4 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.5 = private unnamed_addr constant [28 x i8] c"Moltiplicazione 8 * x: %d \0A\00", align 1
@.str.6 = private unnamed_addr constant [22 x i8] c"Divisione x / 8: %u \0A\00", align 1
@.str.7 = private unnamed_addr constant [29 x i8] c"Moltiplicazione x * 15: %d \0A\00", align 1
@.str.8 = private unnamed_addr constant [29 x i8] c"Moltiplicazione x * 17: %d \0A\00", align 1
@.str.9 = private unnamed_addr constant [35 x i8] c"Multiple Instruction Optimization\0A\00", align 1
@.str.10 = private unnamed_addr constant [21 x i8] c"Inserire un numero: \00", align 1
@.str.11 = private unnamed_addr constant [27 x i8] c"Il nuemro inserito \C3\A8: %d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @algebraic_id() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 10, ptr %1, align 4
  %4 = load i32, ptr %1, align 4
  %5 = add nsw i32 %4, 0
  store i32 %5, ptr %2, align 4
  %6 = load i32, ptr %1, align 4
  %7 = add nsw i32 0, %6
  store i32 %7, ptr %2, align 4
  %8 = load i32, ptr %1, align 4
  %9 = mul nsw i32 %8, 1
  store i32 %9, ptr %2, align 4
  %10 = load i32, ptr %1, align 4
  %11 = mul nsw i32 1, %10
  store i32 %11, ptr %2, align 4
  %12 = load i32, ptr %2, align 4
  %13 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %12)
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local void @strength_red() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  %7 = call i32 (ptr, ...) @printf(ptr noundef @.str.3)
  %8 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str.4, ptr noundef %1)
  %9 = load i32, ptr %1, align 4
  %10 = mul i32 %9, 8
  store i32 %10, ptr %2, align 4
  %11 = load i32, ptr %1, align 4
  %12 = mul i32 8, %11
  store i32 %12, ptr %2, align 4
  %13 = load i32, ptr %1, align 4
  %14 = udiv i32 %13, 8
  store i32 %14, ptr %3, align 4
  %15 = load i32, ptr %1, align 4
  %16 = mul i32 %15, 15
  store i32 %16, ptr %4, align 4
  %17 = load i32, ptr %1, align 4
  %18 = mul i32 %17, 17
  store i32 %18, ptr %5, align 4
  %19 = load i32, ptr %2, align 4
  %20 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i32 noundef %19)
  %21 = load i32, ptr %3, align 4
  %22 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, i32 noundef %21)
  %23 = load i32, ptr %4, align 4
  %24 = call i32 (ptr, ...) @printf(ptr noundef @.str.7, i32 noundef %23)
  %25 = load i32, ptr %5, align 4
  %26 = call i32 (ptr, ...) @printf(ptr noundef @.str.8, i32 noundef %25)
  ret void
}

declare i32 @__isoc99_scanf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local void @multi_ins_opt() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = call i32 (ptr, ...) @printf(ptr noundef @.str.9)
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.10)
  %11 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str.4, ptr noundef %1)
  %12 = load i32, ptr %1, align 4
  %13 = sub nsw i32 %12, 1
  store i32 %13, ptr %2, align 4
  %14 = load i32, ptr %2, align 4
  %15 = add nsw i32 %14, 6
  store i32 %15, ptr %3, align 4
  %16 = load i32, ptr %3, align 4
  %17 = sub nsw i32 %16, 2
  store i32 %17, ptr %4, align 4
  %18 = load i32, ptr %4, align 4
  %19 = sub nsw i32 %18, 3
  store i32 %19, ptr %5, align 4
  %20 = load i32, ptr %4, align 4
  %21 = add nsw i32 %20, 2
  store i32 %21, ptr %6, align 4
  %22 = load i32, ptr %6, align 4
  %23 = sub nsw i32 %22, 1
  store i32 %23, ptr %7, align 4
  %24 = load i32, ptr %7, align 4
  %25 = add nsw i32 %24, 1
  store i32 %25, ptr %8, align 4
  %26 = load i32, ptr %5, align 4
  %27 = call i32 (ptr, ...) @printf(ptr noundef @.str.11, i32 noundef %26)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  call void @algebraic_id()
  call void @strength_red()
  call void @multi_ins_opt()
  ret i32 0
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.1.7 (++20250114103320+cd708029e0b2-1~exp1~20250114103432.75)"}
