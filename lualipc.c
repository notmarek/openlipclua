#include "openlipc.h"

#include <stdlib.h>
#include <string.h>
#include "compat-5.3.h"
#include "lua.h"
#include "lauxlib.h"

// Symbol visibility
#define DLL_PUBLIC __attribute__((visibility("default")))
#define DLL_LOCAL  __attribute__((visibility("hidden")))

typedef struct {
    LIPC *lipc;
} lipc_userdata_t;

typedef struct {
LIPCha *ha; 
} lipcha_userdata_t;

void check_lipc_code(lua_State *L, LIPCcode code) {
    if (code != 0) {
        if (code != -1) {
            luaL_error(L, "Lipc error: %d:%s", code, LipcGetErrorString(code));
        } else {
            luaL_error(L, "Unknown error while opening a lipc handle, most likely service name wasn't a fully qualified dot-separated identifier.");
        }
    }
}

static int lualipc_new_hasharray(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    lipcha_userdata_t *lha;
    lha       = (lipcha_userdata_t *)lua_newuserdata(L, sizeof(*lha));
    lha->ha = NULL;
    luaL_getmetatable(L, "LuaLipcHA");
    lua_setmetatable(L, -2);
    lha->ha = LipcHasharrayNew(lu->lipc);
    return 1;
}


static int lualipcha_keys(lua_State *L) {


    lipcha_userdata_t *lha;
    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    int index = luaL_checkint(L, 2);

    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }
    
    int count = 0;
    LIPCcode code = LipcHasharrayKeys(lha->ha, index, NULL, &count);
    const char* array[count];
    code = LipcHasharrayKeys(lha->ha, index, &array, &count);
    check_lipc_code(L, code);

    lua_createtable(L, count, 0);
    for (int i=0; i<count; i++) {
        lua_pushstring(L, array[i]);
        lua_rawseti (L, -2, i+1); /* In lua indices start at 1 */
    }

    return 1;
}

static int lualipcha_put_string(lua_State *L) {
    lipcha_userdata_t *lha;
    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    int index = luaL_checkint(L, 2);
    const char* key = luaL_checkstring(L, 3);
    const char* value = luaL_checkstring(L, 4);
    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }
    LIPCcode code = LipcHasharrayPutString(lha->ha, index, key, value);
    check_lipc_code(L, code);
    return 1;
}

static int lualipcha_put_int(lua_State *L) {
    lipcha_userdata_t *lha;
    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    int index = luaL_checkint(L, 2);
    const char* key = luaL_checkstring(L, 3);
    const char* value = luaL_checkint(L, 4);
    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }
    LIPCcode code = LipcHasharrayPutInt(lha->ha, index, key, value);
    check_lipc_code(L, code);
    return 1;
}

static int lualipcha_add_hash(lua_State *L) {
    lipcha_userdata_t *lha;
    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");

    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }
    size_t index = 0;
    LIPCcode code = LipcHasharrayAddHash(lha->ha, &index);
    check_lipc_code(L, code);

    return 1;
}

static int lualipcha_count(lua_State *L) {
    lipcha_userdata_t *lha;
    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }
    int count = LipcHasharrayGetHashCount(lha->ha);
    lua_pushinteger(L, count);
    return 1;
}

static int lualipcha_to_table(lua_State *L) {
    lipcha_userdata_t *lha;
    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }

    int count = LipcHasharrayGetHashCount(lha->ha);
    lua_createtable(L, count, 0);
    for (int i=0; i<count; i++) {
        int key_count = 0;
        LIPCcode code = LipcHasharrayKeys(lha->ha, i, NULL, &key_count);
        const char* array[key_count];
        code = LipcHasharrayKeys(lha->ha, i, &array, &key_count);
        check_lipc_code(L, code);
        
        lua_createtable(L, key_count, 0);
        for (int j=0; j<key_count; j++) {
            LIPCHasharrayType type;
            size_t value_size;
            lua_pushstring(L, array[j]);

            code = LipcHasharrayCheckKey(lha->ha, i, array[j], &type, &value_size);
            check_lipc_code(L, code);
            
            switch(type) {
                case LIPC_HASHARRAY_INT:
                    int data;
                    code = LipcHasharrayGetInt(lha->ha, i, array[j], &data);
                    check_lipc_code(L, code);
                    lua_pushinteger(L, data);
                    break;
                case LIPC_HASHARRAY_STRING:
                    char* data_str;
                    code = LipcHasharrayGetString(lha->ha, i, array[j], &data_str);
                    check_lipc_code(L, code);
                    
                    lua_pushstring(L, data_str);
                    break;
                case LIPC_HASHARRAY_BLOB:
                    lua_pushstring(L, "unsupported");
                    break;
            }
            lua_settable(L, -3);
        }
        lua_rawseti (L, -2, i+1); /* In lua indices start at 1 */
    }
    return 1; 
}

// lipc = require("liblualipc")
// lel = lipc.open("com.notmarek.test")
// scan = lel:access_hash_property("com.lab126.wifid", "profileData", lel:new_hasharray())
// d = scan:to_table()

static int lualipcha_tostring(lua_State *L)
{
    lipcha_userdata_t *lha;

    lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    if (lha->ha == NULL) {
        lua_pushfstring(L, "HashArray doesn't exist.");
        return 1;
    }
    int size = 0;
    LIPCcode code = LipcHasharrayToString(lha->ha, NULL, &size);
    char* value = malloc(size);
    code = LipcHasharrayToString(lha->ha, value, &size);
    check_lipc_code(L, code);

    lua_pushstring(L, value);

    return 1;
}

static int lualipcha_destroy(lua_State *L) {
    lipcha_userdata_t *lha = (lipcha_userdata_t *)luaL_checkudata(L, 1, "LuaLipcHA");
    LIPCcode code = LipcHasharrayDestroy(lha->ha);
    check_lipc_code(L, code);
    lha->ha = NULL;
    return 1;
}



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
    check_lipc_code(L, code);
    return 1;
}

static int lualipc_get_string_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    char* value;

    LIPCcode code = LipcGetStringProperty(lu->lipc, service, property, &value);
    check_lipc_code(L, code);
    lua_pushstring(L, value);
    LipcFreeString(value);

    return 1;
}

static int lualipc_access_hasharray_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    lipcha_userdata_t *lha = (lipcha_userdata_t *)luaL_checkudata(L, 4, "LuaLipcHA");

    LIPCha* value;

    LIPCcode code = LipcAccessHasharrayProperty(lu->lipc, service, property, lha->ha, &value);
    check_lipc_code(L, code);
    
    lipcha_userdata_t* outlha = (lipcha_userdata_t *)lua_newuserdata(L, sizeof(*lha));
    outlha->ha = NULL;
    luaL_getmetatable(L, "LuaLipcHA");
    lua_setmetatable(L, -2);
    outlha->ha = value;
    return 1;
}

static int lualipc_set_string_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    const char* value = luaL_checkstring(L, 4);

    LIPCcode code = LipcSetStringProperty(lu->lipc, service, property, value);
    check_lipc_code(L, code);
    

    return 1;
}


static int lualipc_get_int_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    int value;

    LIPCcode code = LipcGetIntProperty(lu->lipc, service, property, &value);
    check_lipc_code(L, code);
    
    lua_pushinteger(L, value);

    return 1;
}

static int lualipc_set_int_property(lua_State *L) {
    lipc_userdata_t *lu = (lipc_userdata_t *)luaL_checkudata(L, 1, "LuaLipc");
    const char* service = luaL_checkstring(L, 2);
    const char* property = luaL_checkstring(L, 3);
    int value = luaL_checkint(L, 4);

    LIPCcode code = LipcSetIntProperty(lu->lipc, service, property, value);
    check_lipc_code(L, code);


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

static const struct luaL_Reg lualipcha_methods[] = {


    { "put_string", lualipcha_put_string },
    { "put_int", lualipcha_put_int },
    { "add_hash", lualipcha_add_hash },
    { "count", lualipcha_count },
    { "keys", lualipcha_keys },
    { "to_table", lualipcha_to_table },

    { "__gc",        lualipcha_destroy   },
    { "__tostring",  lualipcha_tostring  },
    { NULL,          NULL               },
};

static const struct luaL_Reg lualipc_methods[] = {
    { "new_hasharray", lualipc_new_hasharray },
    { "access_hash_property", lualipc_access_hasharray_property },
    { "set_string_property",  lualipc_set_string_property },
    { "get_string_property",  lualipc_get_string_property },
    { "set_int_property",  lualipc_set_int_property },
    { "get_int_property",  lualipc_get_int_property },
    { "__gc",        lualipc_destroy   },
    { "__tostring",  lualipc_tostring  },
    { NULL,          NULL               },
};

static const struct luaL_Reg lualipc_functions[] = {
    { "open_no_name", lualipc_open_no_name },
    { "open", lualipc_open },
    { NULL,  NULL         }
};

DLL_PUBLIC int luaopen_liblualipc(lua_State *L)
{
    luaL_newmetatable(L, "LuaLipc");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lualipc_methods, 0);

    luaL_newmetatable(L, "LuaLipcHA");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, lualipcha_methods, 0);

    luaL_newlib(L, lualipc_functions);
    return 1;
}
