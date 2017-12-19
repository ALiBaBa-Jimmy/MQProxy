#pragma once  
#include "mysql.h"

#define DECL_BIND(h,n)\   
class bind_##h:public h{\   
    typedef h parent;\   
    MYSQL_BIND _bind[n];\   
    my_bool _is_null[n];\   
    unsigned long _length[n];\   
public:\   
       bind_##h(){\   
       int i=0;\   
       memset(_bind, 0x00, sizeof(_bind));   
  
#define BIND_BIN(x,l)\   
    _bind.buffer_type= MYSQL_TYPE_STRING;\   
    _bind[i].buffer= (char *)&(parent::x);\   
    _bind[i].buffer_length= l;\   
    _bind[i].is_null= _is_null+i;\   
    _bind[i].length= _length+i;\   
    ++i;   
  
#define BIND_INT(x)\   
    _bind[i].buffer_type= MYSQL_TYPE_LONG;\   
    _bind[i].buffer= (char *)&(parent::x);\   
    _bind[i].buffer_length= 0;\   
    _bind[i].is_null= _is_null+i;\   
    _bind[i].length= _length+i;\   
    ++i;   
  
#define BIND_TINY(x)\   
    _bind[i].buffer_type= MYSQL_TYPE_TINY;\   
    _bind[i].buffer= (char *)&(parent::x);\   
    _bind[i].buffer_length= 0;\   
    _bind[i].is_null= _is_null+i;\   
    _bind[i].length= _length+i;\   
    ++i;   
  
#define BIND_SHORT(x)\   
    _bind[i].buffer_type= MYSQL_TYPE_SHORT;\   
    _bind[i].buffer= (char *)&(parent::x);\   
    _bind[i].buffer_length= 0;\   
    _bind[i].is_null= _is_null+i;\   
    _bind[i].length= _length+i;\   
    ++i;   
  
#define END_BIND(h) }\   
    operator MYSQL_BIND*(){\   
    return _bind;\   
    }\   
};   
  
#endif 
