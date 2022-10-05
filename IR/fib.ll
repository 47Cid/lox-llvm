; ModuleID = 'My Module'
source_filename = "My Module"

declare double @printd(double)

declare i32 @printf(i8*, ...)

define double @fib(double %n) {
entry:
  %retvalue = alloca double, align 8
  %n1 = alloca double, align 8
  store double %n, double* %n1, align 8
  %n2 = load double, double* %n1, align 8
  %cmptmp = fcmp ult double %n2, 3.000000e+00
  %booltmp = uitofp i1 %cmptmp to double
  %ifcond = fcmp one double %booltmp, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  store double 1.000000e+00, double* %retvalue, align 8
  br label %exit

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else
  %n3 = load double, double* %n1, align 8
  %subtmp = fsub double %n3, 1.000000e+00
  %calltmp = call double @fib(double %subtmp)
  %n4 = load double, double* %n1, align 8
  %subtmp5 = fsub double %n4, 2.000000e+00
  %calltmp6 = call double @fib(double %subtmp5)
  %addtmp = fadd double %calltmp, %calltmp6
  store double %addtmp, double* %retvalue, align 8
  br label %exit

exit:                                             ; preds = %ifcont, %then
  %retvalue7 = load double, double* %retvalue, align 8
  ret double %retvalue7
}

define double @xeMain() {
entry:
  %output = alloca double, align 8
  %y = alloca double, align 8
  %retvalue = alloca double, align 8
  store double 9.000000e+00, double* %y, align 8
  %y1 = load double, double* %y, align 8
  %calltmp = call double @fib(double %y1)
  store double %calltmp, double* %output, align 8
  %output2 = load double, double* %output, align 8
  %calltmp3 = call double @printd(double %output2)
  store double 0.000000e+00, double* %retvalue, align 8
  br label %exit

exit:                                             ; preds = %entry
  %retvalue4 = load double, double* %retvalue, align 8
  ret double %retvalue4
}
