// Filename: parameterRemapThis.C
// Created by:  drose (02Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "parameterRemapThis.h"
#include "typeManager.h"

#include <cppStructType.h>
#include <cppSimpleType.h>
#include <cppPointerType.h>
#include <cppConstType.h>

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ParameterRemapThis::
ParameterRemapThis(CPPStructType *type, bool is_const) :
  ParameterRemap(TypeManager::get_void_type())
{
  if (is_const) {
    _new_type = TypeManager::wrap_const_pointer(type);
  } else {
    _new_type = TypeManager::wrap_pointer(type);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::pass_parameter
//       Access: Public, Virtual
//  Description: Outputs an expression that converts the indicated
//               variable from the new type to the original type, for
//               passing into the actual C++ function.
////////////////////////////////////////////////////////////////////
void ParameterRemapThis::
pass_parameter(ostream &out, const string &) {
  out << "**invalid**";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::get_return_expr
//       Access: Public, Virtual
//  Description: Returns an expression that evalutes to the
//               appropriate value type for returning from the
//               function, given an expression of the original type.
////////////////////////////////////////////////////////////////////
string ParameterRemapThis::
get_return_expr(const string &) {
  return "**invalid**";
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapThis::is_this
//       Access: Public, Virtual
//  Description: Returns true if this is the "this" parameter.
////////////////////////////////////////////////////////////////////
bool ParameterRemapThis::
is_this() {
  return true;
}
