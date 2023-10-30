#include "lua_cfg.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PATH_FMT "%s/.config/st/?.lua;%s"

#define FONT_FMT "%s:pixelsize=%d:antialias=true:autohint=true, Noto Emoji:pixelsize=%d:antialias=true"

#define ERR_PREFIX "\x1b[1;31m[Error]:\x1b[0m "
#define FBUFF_LEN 256

static char fontbuff[256],
            colors[20][8];

static void print_type_mismatch_error(lua_State *L, const char *varname, int expected_type, int received_type) {
  fprintf(stderr, ERR_PREFIX"Lua config type mismatch on \"%s\".\n"
      "\tExpected: %s\n\tReceived: %s\n", varname, 
      lua_typename(L, expected_type), lua_typename(L, received_type));
}



static char *setup_path_and_getconfpath(lua_State *L) {
  const char *defpath, *homepath;
  char *ret;
  int len;
  homepath = getenv("HOME");
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path");
  defpath = lua_tostring(L, -1);
  lua_pop(L, 1);
  // Give len some padding.
  len = strlen(defpath) + strlen(homepath) + strlen(PATH_FMT) + 16;
  ret = calloc(len, sizeof(char));
  sprintf(ret, PATH_FMT, homepath, defpath);
  lua_pushstring(L, ret);
  lua_setfield(L, -2, "path");
  lua_pop(L, 1);
  memset((void*)ret, 0x0, len);
  sprintf(ret, "%s/.config/st/config.lua", homepath);
  return ret;
}

static void try_running_cfg(lua_State *L, char *cfg_path) {
  FILE *check_exists;
  if (!(check_exists = fopen(cfg_path, "r"))) {
    perror(ERR_PREFIX"Could not open config file: ");
    free((void*)cfg_path);
    lua_close(L);
    exit(EXIT_FAILURE);
  }

  fclose(check_exists);
  if (luaL_dofile(L, cfg_path)!=LUA_OK) {
    fprintf(stderr, ERR_PREFIX"Error occurred during Lua config execution.\n"
        "Details from Lua State: %s\n",
        lua_tostring(L, -1));
    free((void*)cfg_path);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
}

static void parse_ith_color(lua_State *L, int i) {
  const char *src;
  char *dest;
  int tmp, j;
  lua_geti(L, -1, i+1);
  if ((tmp=lua_type(L, -1))!=LUA_TSTRING) {
    print_type_mismatch_error(L, "ColorScheme table entry", LUA_TSTRING, tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  src = lua_tostring(L, -1);
  lua_pop(L, 1);
  dest = colors[i];
  if ((dest[0] = src[0]) != '#') {
    fprintf(stderr, ERR_PREFIX"Format error on color entry at\n"
        "ColorScheme[%d]: \"%s\" (Missing the leading '#').\n"
        "Correct format is: \"#RRGGBB\", where"
        "RR, GG, BB are the hexadecimal values for\nthe red, green, and blue"
        "color channels, respectively.\n", i+1, src);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  for (j = 1; j < 7; ++j) {
    if (!isxdigit((dest[j] = src[j]))) {
      fprintf(stderr, ERR_PREFIX"Format error on color entry at\n"
          "ColorScheme[%d]: \"%s\" (%c is not a hex digit.).\n"
          "Correct format is: \"#RRGGBB\", where"
          "RR, GG, BB are the hexadecimal values for\nthe red, green, and blue"
          "color channels, respectively.\n", i+1, src, src[j]);
      lua_close(L);
      exit(EXIT_FAILURE);
    }
  }

}

static void set_colors(lua_State *L) {
  int tmp, i;
  memset((void*)colors, '\0', 20*8*sizeof(char));

  lua_getglobal(L, "ColorScheme");
  if ((tmp=lua_type(L, -1))!=LUA_TTABLE) {
    print_type_mismatch_error(L, "ColorScheme", LUA_TTABLE, tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  if ((tmp=luaL_len(L, -1))!=20) {
    fprintf(stderr, ERR_PREFIX"Lua config global, \"ColorScheme\" of "
        "unexpected length.\n\tExpected: 20 color entries.\n\tReceived:"
        " %d color entries.\n", tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < 20; ++i) {
    parse_ith_color(L, i);
  }
}

static void set_font(lua_State *L) {
  const char *fname;
  int font_size, fnamelen, tmp;

  lua_getglobal(L, "FontSize");
  if ((tmp = lua_type(L, -1)) != LUA_TNUMBER) {
    print_type_mismatch_error(L, "FontSize", LUA_TNUMBER, tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  font_size = (int)lua_tointeger(L, -1);
  if (font_size > 72 || font_size <= 0) {
    fprintf(stderr, ERR_PREFIX"Font size specified, %d, not within valid "
        "range, (0, 72].\n", font_size);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  lua_pop(L, 1);
  lua_getglobal(L, "Font");
  
  if ((tmp = lua_type(L, -1))!=LUA_TSTRING) {
    print_type_mismatch_error(L, "Font", LUA_TSTRING, tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }

  if (((fnamelen = strlen((fname = lua_tostring(L, -1)))) + 
        (tmp = strlen(FONT_FMT))) > FBUFF_LEN) {
    fprintf(stderr, ERR_PREFIX"Font name too long to fit in static font buffer."
        "\nPlease choose a reasonably-sized font name\n(i.e: less than %d "
        "characters long).\n", FBUFF_LEN-tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  lua_pop(L, 1);
  
  memset((void*)fontbuff, '\0', FBUFF_LEN*sizeof(char));

  sprintf(fontbuff, FONT_FMT, fname, font_size, font_size);
}

static int set_cursorshape(lua_State *L) {
  int tmp;
  lua_getglobal(L, "CursorShape");
  if ((tmp=lua_type(L, -1))!=LUA_TNUMBER) {
    print_type_mismatch_error(L, "CursorShape", LUA_TNUMBER, tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  tmp = (int)lua_tointeger(L, -1);
  lua_pop(L, 1);
  switch (tmp) {
    case 2:
    case 4:
    case 6:
    case 7:
      return tmp;
    default:
      break;
  }
  fprintf(stderr, ERR_PREFIX"CursorShape ID number specified, %d, is an invalid"
      " ID.\nValid IDs are as follows:\n"
      "\t\x1b[1;34m2:\x1b[0m\tBlock (\"█\")\n"
      "\t\x1b[1;34m4:\x1b[0m\tUnderline (\"_\")\n"
      "\t\x1b[1;34m6:\x1b[0m\tBar (\"|\")"
      "\t\x1b[1;34m7:\x1b[0m\tSnowman (\"☃\")\n", tmp);

  lua_close(L);
  exit(EXIT_FAILURE);
}

static float set_alpha(lua_State *L) {
  float ret;
  int tmp;
  
  lua_getglobal(L, "Alpha");
  if ((tmp = lua_type(L, -1))!=LUA_TNUMBER) {
    print_type_mismatch_error(L, "Alpha", LUA_TNUMBER, tmp);
    lua_close(L);
    exit(EXIT_FAILURE);
  }
  ret = (float)lua_tonumber(L, -1);
  lua_pop(L, 1);
  if ( ret > 1.0f || ret < 0.0f) {
    fprintf(stderr, ERR_PREFIX"Alpha value specified, %f, not within valid "
        "range of [0.0, 1.0].\n", ret);
    lua_close(L);
    exit(EXIT_FAILURE);
  }

  return ret;

}

void grab_luasettings(LuaCfgContext *dest) {
  lua_State *L = luaL_newstate();
  char *conf_fpath;
  int i;
  luaL_openlibs(L);
  conf_fpath = setup_path_and_getconfpath(L);
  try_running_cfg(L, conf_fpath);
  free((void*)conf_fpath);
  dest->cursorshape = set_cursorshape(L);
  dest->alpha = set_alpha(L);
  set_colors(L);
  set_font(L);
  for (i = 0; i < 20; ++i) {
    dest->colors[i] = colors[i];
  }
  dest->fontnamebuff = fontbuff;
  lua_close(L);
}


