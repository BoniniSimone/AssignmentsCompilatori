; ModuleID = './test/test.ll'
source_filename = "test.c"
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
  %1 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  %2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef 10)
  ret void
}

declare i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local void @strength_red() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str.3)
  %4 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str.4, ptr noundef %1)
  %5 = load i32, ptr %1, align 4
  %6 = shl i32 %5, 3
  %7 = load i32, ptr %1, align 4
  %8 = shl i32 %7, 3
  %9 = load i32, ptr %1, align 4
  %10 = lshr i32 %9, 3
  %11 = load i32, ptr %1, align 4
  %12 = shl i32 %11, 4
  %13 = sub i32 %12, %11
  %14 = load i32, ptr %1, align 4
  %15 = shl i32 %14, 4
  %16 = add i32 %15, %14
  %17 = call i32 (ptr, ...) @printf(ptr noundef @.str.5, i32 noundef %8)
  %18 = call i32 (ptr, ...) @printf(ptr noundef @.str.6, i32 noundef %10)
  %19 = call i32 (ptr, ...) @printf(ptr noundef @.str.7, i32 noundef %13)
  %20 = call i32 (ptr, ...) @printf(ptr noundef @.str.8, i32 noundef %16)
  ret void
}

declare i32 @__isoc99_scanf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind uwtable
define dso_local void @multi_ins_opt() #0 {
  %1 = alloca i32, align 4
  %2 = call i32 (ptr, ...) @printf(ptr noundef @.str.9)
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str.10)
  %4 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str.4, ptr noundef %1)
  %5 = load i32, ptr %1, align 4
  %6 = sub nsw i32 %5, 1
  %7 = add nsw i32 %6, 6
  %8 = sub nsw i32 %7, 2
  %9 = sub nsw i32 %7, 1
  %10 = call i32 (ptr, ...) @printf(ptr noundef @.str.11, i32 noundef %5)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main() #0 {
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
