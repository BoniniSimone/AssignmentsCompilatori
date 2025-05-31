; ModuleID = 'loopfLS.ll'
source_filename = "loopf.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__const.loop.a = private unnamed_addr constant [3 x i32] [i32 1, i32 2, i32 3], align 4
@.str = private unnamed_addr constant [11 x i8] c"Loop0: %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"Loop1: %d\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @loop(i32 noundef %0) #0 {
  %2 = alloca [3 x i32], align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %2, ptr align 4 @__const.loop.a, i64 12, i1 false)
  br label %3

3:                                                ; preds = %13, %1
  %.01 = phi i32 [ 0, %1 ], [ %14, %13 ]
  %4 = icmp slt i32 %.01, 3
  br i1 %4, label %5, label %15

5:                                                ; preds = %3
  %6 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %.01)
  %7 = sext i32 %.01 to i64
  %8 = getelementptr inbounds [3 x i32], ptr %2, i64 0, i64 %7
  %9 = load i32, ptr %8, align 4
  %10 = add nsw i32 %9, %.01
  %11 = sext i32 %.01 to i64
  %12 = getelementptr inbounds [3 x i32], ptr %2, i64 0, i64 %11
  store i32 %10, ptr %12, align 4
  br label %13

13:                                               ; preds = %5
  %14 = add nsw i32 %.01, 1
  br label %3, !llvm.loop !6

15:                                               ; preds = %3
  br label %16

16:                                               ; preds = %20, %15
  %.0 = phi i32 [ 0, %15 ], [ %21, %20 ]
  %17 = icmp slt i32 %.0, 3
  br i1 %17, label %18, label %22

18:                                               ; preds = %16
  %19 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %.0)
  br label %20

20:                                               ; preds = %18
  %21 = add nsw i32 %.0, 1
  br label %16, !llvm.loop !8

22:                                               ; preds = %16
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

declare i32 @printf(ptr noundef, ...) #2

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 19.1.7 (++20250114103320+cd708029e0b2-1~exp1~20250114103432.75)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
