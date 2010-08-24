#include "ruby.h"
#include "main.h"

// backward compatibility for ruby 1.8
#ifndef RSTRING_PTR
#define RSTRING_PTR(s) RSTRING(s)->ptr
#endif


static VALUE rb_mDM68;
static VALUE rb_cDM68;




static VALUE parse_file(VALUE obj, VALUE path)
{
  Check_Type(path, T_STRING);

  // reset global state
  memset(result, 0, MAXRESULT);
  demo.demoMessageSequence = 0;
  demo.gameStatesParsed = 0;

  if(!(demo.demofile=fopen(RSTRING_PTR(path), "rb")))
  {
    Com_Error(ERR_FATAL, "Couldn't open demofile");
  }

  Huff_Init();

  append_result("---\n");

  while(!demo.gameStatesParsed) {
    if(!Parse_NextDemoMessage()) {
      break;
    }
  }

  GameStateParsed();

  append_result("prints:\n");

  while(1) {
    if(!Parse_NextDemoMessage()) {
      break;
    }
    NewFrameParsed();
  }

  fclose(demo.demofile);

  return rb_str_new2(result);
}


void Init_dm68()
{
  rb_mDM68 = rb_define_module("DM68");
  rb_define_module_function(rb_mDM68, "parse_file", parse_file, 1);
}

