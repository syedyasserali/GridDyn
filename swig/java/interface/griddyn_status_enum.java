/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 3.0.12
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package com.java.griddyn;

public final class griddyn_status_enum {
  public final static griddyn_status_enum griddyn_ok = new griddyn_status_enum("griddyn_ok", griddynJNI.griddyn_ok_get());
  public final static griddyn_status_enum griddyn_invalid_object = new griddyn_status_enum("griddyn_invalid_object", griddynJNI.griddyn_invalid_object_get());
  public final static griddyn_status_enum griddyn_invalid_parameter_value = new griddyn_status_enum("griddyn_invalid_parameter_value", griddynJNI.griddyn_invalid_parameter_value_get());
  public final static griddyn_status_enum griddyn_unknown_parameter = new griddyn_status_enum("griddyn_unknown_parameter", griddynJNI.griddyn_unknown_parameter_get());
  public final static griddyn_status_enum griddyn_add_failure = new griddyn_status_enum("griddyn_add_failure", griddynJNI.griddyn_add_failure_get());
  public final static griddyn_status_enum griddyn_remove_failure = new griddyn_status_enum("griddyn_remove_failure", griddynJNI.griddyn_remove_failure_get());
  public final static griddyn_status_enum griddyn_query_load_failure = new griddyn_status_enum("griddyn_query_load_failure", griddynJNI.griddyn_query_load_failure_get());
  public final static griddyn_status_enum griddyn_file_load_failure = new griddyn_status_enum("griddyn_file_load_failure", griddynJNI.griddyn_file_load_failure_get());
  public final static griddyn_status_enum griddyn_solve_error = new griddyn_status_enum("griddyn_solve_error", griddynJNI.griddyn_solve_error_get());
  public final static griddyn_status_enum griddyn_object_not_initialized = new griddyn_status_enum("griddyn_object_not_initialized", griddynJNI.griddyn_object_not_initialized_get());
  public final static griddyn_status_enum griddyn_invalid_function_call = new griddyn_status_enum("griddyn_invalid_function_call", griddynJNI.griddyn_invalid_function_call_get());
  public final static griddyn_status_enum griddyn_function_failure = new griddyn_status_enum("griddyn_function_failure", griddynJNI.griddyn_function_failure_get());

  public final int swigValue() {
    return swigValue;
  }

  public String toString() {
    return swigName;
  }

  public static griddyn_status_enum swigToEnum(int swigValue) {
    if (swigValue < swigValues.length && swigValue >= 0 && swigValues[swigValue].swigValue == swigValue)
      return swigValues[swigValue];
    for (int i = 0; i < swigValues.length; i++)
      if (swigValues[i].swigValue == swigValue)
        return swigValues[i];
    throw new IllegalArgumentException("No enum " + griddyn_status_enum.class + " with value " + swigValue);
  }

  private griddyn_status_enum(String swigName) {
    this.swigName = swigName;
    this.swigValue = swigNext++;
  }

  private griddyn_status_enum(String swigName, int swigValue) {
    this.swigName = swigName;
    this.swigValue = swigValue;
    swigNext = swigValue+1;
  }

  private griddyn_status_enum(String swigName, griddyn_status_enum swigEnum) {
    this.swigName = swigName;
    this.swigValue = swigEnum.swigValue;
    swigNext = this.swigValue+1;
  }

  private static griddyn_status_enum[] swigValues = { griddyn_ok, griddyn_invalid_object, griddyn_invalid_parameter_value, griddyn_unknown_parameter, griddyn_add_failure, griddyn_remove_failure, griddyn_query_load_failure, griddyn_file_load_failure, griddyn_solve_error, griddyn_object_not_initialized, griddyn_invalid_function_call, griddyn_function_failure };
  private static int swigNext = 0;
  private final int swigValue;
  private final String swigName;
}
