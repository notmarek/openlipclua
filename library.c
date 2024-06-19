#include "openlipc.h"

#include <stdlib.h>
#include <string.h>
#include "compat-5.3.h"
#include "lua.h"
#include "lauxlib.h"


/* Userdata object that will hold the counter and name. */
typedef struct {
    LIPC *lipc;
} lipc_userdata_t;

typedef struct {
    LIPCha *ha;
} lipcha_userdata_t;

static int lualipc_open_no_name(lua_State *L)
{
    lipc_userdata_t *lu;
    lu       = (lipc_userdata_t *)lua_newuserdata(L, sizeof(*lu));
    lu->lipc = NULL;
    luaL_getmetatable(L, "LuaLipc");
    lua_setmetatable(L, -2);
    lu->lipc    = LipcOpenNoName();
    return 1;
}

static int lualipc_open(lua_State *L)
{

    const char* service_name = luaL_checkstring(L, 1);
    if (service_name == NULL)
        luaL_error(L, "service_name cannot be empty");
    lipc_userdata_t *lu;
    lu       = (lipc_userdata_t *)lua_newuserdata(L, sizeof(*lu));
    lu->lipc = NULL;
    luaL_getmetatable(L, "LuaLipc");
    lua_setmetatable(L, -2);
    LIPCcode code;
    lu->lipc = LipcOpenEx(service_name, &code);
    if (code != 0) {
        if (code != -1) {
            luaL_error(L, "Lipc error: %d:%s", code, LipcGetErrorString(code));
        } else {
            luaL_error(L, "Unknown error while opening a lipc handle, most likely service name wasn't a fully qualified dot-separated identifier.");
        }
    }

    return 1;
}

static int lualipc_get_string_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    char* value;

    LIPCcode code = LipcGetStringProperty(lu->lipc, service, property, &value);
    if (code != 0) {
        if (code != -1) {
            luaL_error(L, "Lipc error: %d:%s", code, LipcGetErrorString(code));
        } else {
            luaL_error(L, "Unknown error while opening a lipc handle, most likely service name wasn't a fully qualified dot-separated identifier.");
        }
    }
    lua_pushstring(L, value);
    LipcFreeString(value);

    return 1;
}

static int lualipc_set_string_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    const char* value = luaL_checkstring(L, 4);

    LIPCcode code = LipcSetStringProperty(lu->lipc, service, property, value);
    if (code != 0) {
        if (code != -1) {
            luaL_error(L, "Lipc error: %d:%s", code, LipcGetErrorString(code));
        } else {
            luaL_error(L, "Unknown error while opening a lipc handle, most likely service name wasn't a fully qualified dot-separated identifier.");
        }
    }

    return 1;
}


static int lualipc_get_int_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    int value;

    LIPCcode code = LipcGetIntProperty(lu->lipc, service, property, &value);
    if (code != 0) {
        if (code != -1) {
            luaL_error(L, "Lipc error: %d:%s", code, LipcGetErrorString(code));
        } else {
            luaL_error(L, "Unknown error while opening a lipc handle, most likely service name wasn't a fully qualified dot-separated identifier.");
        }
    }
    lua_pushinteger(L, value);

    return 1;
}

static int lualipc_set_int_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    int value = luaL_checkint(L, 4);

    LIPCcode code = LipcSetIntProperty(lu->lipc, service, property, value);
    if (code != 0) {
        if (code != -1) {
            luaL_error(L, "Lipc error: %d:%s", code, LipcGetErrorString(code));
        } else {
            luaL_error(L, "Unknown error while opening a lipc handle, most likely service name wasn't a fully qualified dot-separated identifier.");
        }
    }

    return 1;
}



static int lualipc_destroy(lua_State *L)
{
    lipc_userdata_t *lu;

    lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    if (lu->lipc != NULL)
        LipcClose(lu->lipc);
    lu->lipc = NULL;

    return 0;
}

static int lualipc_tostring(lua_State *L)
{
    lipc_userdata_t *lu;

    lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    if (lu->lipc == NULL) {
        lua_pushfstring(L, "LipcHandle is closed.");
        return 1;
    }
    const char* lipc_service_name = LipcGetServiceName(lu->lipc);
    
    lua_pushfstring(L, "LipcHandle (%s)", lipc_service_name != NULL ? lipc_service_name : "No registered service" );

    return 1;
}

static const struct luaL_Reg lualipc_methods[] = {

    { "set_string_property",  lualipc_set_string_property },
    { "get_string_property",  lualipc_get_string_property },
    { "set_int_property",  lualipc_set_int_property },
    { "get_int_property",  lualipc_get_int_property },
    { "close",       lualipc_destroy   },
    { "__gc",        lualipc_destroy   },
    { "__tostring",  lualipc_tostring  },
    { NULL,          NULL               },
};

static const struct luaL_Reg lualipc_functions[] = {
    { "open_no_name", lualipc_open_no_name },
    { "open", lualipc_open },
    { NULL,  NULL         }
};

int luaopen_liblualipc(lua_State *L)
{
    luaL_newmetatable(L, "LuaLipc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lualipc_methods, 0);
    luaL_newlib(L, lualipc_functions);
    return 1;
}
