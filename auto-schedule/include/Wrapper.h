#ifndef _WRAPPER_H_
#define _WRAPPER_H_

#include "3rdparty/json.hpp"
#include "Halide.h"

template <typename T>
class Wrapper{
  public:
    Wrapper(Halide::Buffer<T>& buf, string name)
      : name(name)
       ,func(Halide::Func("name"))
       ,x(Var("x_at_" + name))
       ,y(Var("y_at_" + name))
       ,z(Var("z_at_" + name))
       ,xi(Var("xi_at_" + name))
       ,yi(Var("yi_at_" + name))
       ,zi(Var("zi_at_" + name))
       ,xo(Var("xo_at_" + name))
       ,yo(Var("yo_at_" + name))
       ,zo(Var("zo_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xio(Var("xio_at_" + name))
       ,yii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name))
       ,xii(Var("xii_at_" + name)){

       func(x, y, z) = buf(x, y, z);
    }
};
#endif
