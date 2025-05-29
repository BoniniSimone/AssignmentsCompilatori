; ModuleID = 'loopfLS.ll'
source_filename = "loopf.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__const.loop.b = private unnamed_addr constant [10 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10], align 16
@__const.loop.c = private unnamed_addr constant [10 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10], align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @loop(i32 noundef %0) #0 {
  %2 = alloca [10 x i32], align 16
  %3 = alloca [10 x i32], align 16
  %4 = alloca [10 x i32], align 16
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %3, ptr align 16 @__const.loop.b, i64 40, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %4, ptr align 16 @__const.loop.c, i64 40, i1 false)
  br label %5

5:                                                ; preds = %17, %1
  %.01 = phi i32 [ 0, %1 ], [ %18, %17 ]
  %6 = icmp slt i32 %.01, 10
  br i1 %6, label %7, label %19

7:                                                ; preds = %5
  %8 = sext i32 %.01 to i64
  %9 = getelementptr inbounds [10 x i32], ptr %3, i64 0, i64 %8
  %10 = load i32, ptr %9, align 4
  %11 = sext i32 %.01 to i64
  %12 = getelementptr inbounds [10 x i32], ptr %4, i64 0, i64 %11
  %13 = load i32, ptr %12, align 4
  %14 = add nsw i32 %10, %13
  %15 = sext i32 %.01 to i64
  %16 = getelementptr inbounds [10 x i32], ptr %2, i64 0, i64 %15
  store i32 %14, ptr %16, align 4
  br label %17

17:                                               ; preds = %7
  %18 = add nsw i32 %.01, 1
  br label %5, !llvm.loop !6

19:                                               ; preds = %5
  br label %20

20:                                               ; preds = %32, %19
  %.0 = phi i32 [ 0, %19 ], [ %33, %32 ]
  %21 = icmp slt i32 %.0, 10
  br i1 %21, label %22, label %34

22:                                               ; preds = %20
  %23 = sext i32 %.0 to i64
  %24 = getelementptr inbounds [10 x i32], ptr %3, i64 0, i64 %23
  %25 = load i32, ptr %24, align 4
  %26 = sext i32 %.0 to i64
  %27 = getelementptr inbounds [10 x i32], ptr %4, i64 0, i64 %26
  %28 = load i32, ptr %27, align 4
  %29 = add nsw i32 %25, %28
  %30 = sext i32 %.0 to i64
  %31 = getelementptr inbounds [10 x i32], ptr %2, i64 0, i64 %30
  store i32 %29, ptr %31, align 4
  br label %32

32:                                               ; preds = %22
  %33 = add nsw i32 %.0, 1
  br label %20, !llvm.loop !8

34:                                               ; preds = %20
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

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
