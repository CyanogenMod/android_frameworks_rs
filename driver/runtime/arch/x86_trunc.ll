target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S128"
target triple = "i686-unknown-linux"

; http://b/26165616 - As a WAR for this bug, define __truncxfhf2.  Note that
; this exhibits the double-rounding problem.  This WAR will be removed once
; a proper implementation is added to compiler-rt.
define half @__truncxfhf2(x86_fp80 %v1) nounwind readnone alwaysinline {
  %1 = fptrunc x86_fp80 %v1 to float
  %2 = fptrunc float %1 to half
  ret half %2
}
