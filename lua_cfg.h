#ifndef LUA_CFG_H_
#define LUA_CFG_H_

typedef struct _luacfg_ctx {
  char *colors[20];
  char *fontnamebuff;
  float alpha;
  int cursorshape;
  

} LuaCfgContext;

void grab_luasettings(LuaCfgContext *dest);






#endif
