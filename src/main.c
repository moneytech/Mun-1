#include <mun/asm/core.h>
#include <mun/codegen/il_core.h>
#include <mun/codegen/il_value.h>
#include <mun/codegen/il_defs.h>
#include <mun/asm/x64.h>

int
main(int argc, char** argv){
  asm_buff code;
  asm_buff_init(&code);

  instance* ten = number_new(20.0);

  constant_instr* c1 = constant_new(ten);
  return_instr* ret = return_new(value_new(&c1->defn));

  asm_address c1_value_addr;
  asm_addr_init(&c1_value_addr, TMP, offsetof(number, value));

  instr_compile(((instruction*) c1), &code);
  asm_movq_ra(&code, RAX, &c1_value_addr);
  instr_compile(((instruction*) ret), &code);

  ((void (*)(void)) asm_compile(&code))();

  register instance* r11 asm("r11");
  printf("%s\n", lua_to_string(r11));

  register int rax asm("rax");
  printf("%d\n", rax);
  return 0;
}