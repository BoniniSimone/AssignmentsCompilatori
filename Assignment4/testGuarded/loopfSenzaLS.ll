; ModuleID = 'loopfLS.ll'
source_filename = "loopf.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@__const.loop.b = private unnamed_addr constant [10 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10], align 16
@__const.loop.c = private unnamed_addr constant [10 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10], align 16
@.str = private unnamed_addr constant [30 x i8] c"Loop completed successfully.\0A\00", align 1

; Function Attrs: noinline nounwind uwtable
define dso_local void @loop(i32 noundef %0) #0 {
  %2 = alloca [10 x i32], align 16
  %3 = alloca [10 x i32], align 16
  %4 = alloca [10 x i32], align 16
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %3, ptr align 16 @__const.loop.b, i64 40, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 16 %4, ptr align 16 @__const.loop.c, i64 40, i1 false)
  %5 = icmp sgt i32 %0, 0
  br i1 %5, label %6, label %21

6:                                                ; preds = %1
  br label %7

7:                                                ; preds = %18, %6
  %.01 = phi i32 [ 0, %6 ], [ %17, %18 ]
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
  %17 = add nsw i32 %.01, 1
  br label %18

18:                                               ; preds = %7
  %19 = icmp slt i32 %17, %0
  br i1 %19, label %7, label %20, !llvm.loop !6

20:                                               ; preds = %18
  br label %21

21:                                               ; preds = %20, %1
  %22 = icmp sgt i32 %0, 0
  br i1 %22, label %23, label %38

23:                                               ; preds = %21
  br label %24

24:                                               ; preds = %35, %23
  %.0 = phi i32 [ 0, %23 ], [ %34, %35 ]
  %25 = sext i32 %.0 to i64
  %26 = getelementptr inbounds [10 x i32], ptr %3, i64 0, i64 %25
  %27 = load i32, ptr %26, align 4
  %28 = sext i32 %.0 to i64
  %29 = getelementptr inbounds [10 x i32], ptr %4, i64 0, i64 %28
  %30 = load i32, ptr %29, align 4
  %31 = add nsw i32 %27, %30
  %32 = sext i32 %.0 to i64
  %33 = getelementptr inbounds [10 x i32], ptr %2, i64 0, i64 %32
  store i32 %31, ptr %33, align 4
  %34 = add nsw i32 %.0, 1
  br label %35

35:                                               ; preds = %24
  %36 = icmp slt i32 %34, %0
  br i1 %36, label %24, label %37, !llvm.loop !8

37:                                               ; preds = %35
  br label %38

38:                                               ; preds = %37, %21
  ret void
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #1

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  call void @loop(i32 noundef %0)
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  ret i32 0
}

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
