; ModuleID = 'My Module'
source_filename = "My Module"

@0 = private unnamed_addr constant [10 x i8] c"Blah Blah\00", align 1

declare double @printd(double)

declare i32 @printf(i8*, ...)

define double @test() {
entry:
  %retvalue = alloca double, align 8
  store double 0.000000e+00, double* %retvalue, align 8
  br label %exit

exit:                                             ; preds = %entry
  %retvalue1 = load double, double* %retvalue, align 8
  ret double %retvalue1
}
