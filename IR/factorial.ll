; ModuleID = 'My Module'
source_filename = "My Module"

declare double @printd(double)

declare i32 @printf(i8*, ...)

define double @fact(double %n) {
entry:
  %output = alloca double, align 8
  %retvalue = alloca double, align 8
  %n1 = alloca double, align 8
  store double %n, double* %n1, align 8
  store double 1.000000e+00, double* %output, align 8
  %n2 = load double, double* %n1, align 8
  %cmptmp = fcmp ugt double %n2, 0.000000e+00
  %booltmp = uitofp i1 %cmptmp to double
  %ifcond = fcmp one double %booltmp, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  %n3 = load double, double* %n1, align 8
  %n4 = load double, double* %n1, align 8
  %subtmp = fsub double %n4, 1.000000e+00
  %calltmp = call double @fact(double %subtmp)
  %multmp = fmul double %n3, %calltmp
  store double %multmp, double* %output, align 8
  br label %ifcont

else:                                             ; preds = %entry
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %output5 = load double, double* %output, align 8
  store double %output5, double* %retvalue, align 8
  br label %exit

exit:                                             ; preds = %ifcont
  %retvalue6 = load double, double* %retvalue, align 8
  ret double %retvalue6
}

define double @xeMain() {
entry:
  %output = alloca double, align 8
  %myvar = alloca double, align 8
  %retvalue = alloca double, align 8
  store double 4.000000e+00, double* %myvar, align 8
  %myvar1 = load double, double* %myvar, align 8
  %calltmp = call double @fact(double %myvar1)
  store double %calltmp, double* %output, align 8
  %output2 = load double, double* %output, align 8
  %calltmp3 = call double @printd(double %output2)
  store double 0.000000e+00, double* %retvalue, align 8
  br label %exit

exit:                                             ; preds = %entry
  %retvalue4 = load double, double* %retvalue, align 8
  ret double %retvalue4
}
