; ModuleID = 'LoopInv.ll'
source_filename = "LoopInv3.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo(i32 noundef %0) #0 {
  br label %2

2:                                                ; preds = %9, %1
  %.01 = phi i32 [ 0, %1 ], [ %6, %9 ]
  %.0 = phi i32 [ %0, %1 ], [ %7, %9 ]
  %3 = icmp sgt i32 %.0, 0
  br i1 %3, label %4, label %8

4:                                                ; preds = %2
  %5 = call i32 @expensive_constant()
  %6 = add nsw i32 %.01, %5
  %7 = add nsw i32 %.0, -1
  br label %9

8:                                                ; preds = %2
  br label %10

9:                                                ; preds = %4
  br label %2

10:                                               ; preds = %8
  ret i32 %.01
}

; Function Attrs: noinline nounwind uwtable
define internal i32 @expensive_constant() #0 {
  ret i32 42
}

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.1.7 (++20250114103320+cd708029e0b2-1~exp1~20250114103432.75)"}
