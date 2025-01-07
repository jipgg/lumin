#include "halua/libapi.h"
#include <lualib.h>
#include <string>

int halua_newtypetag() {
    static int curr_type_tag = 1;
    return curr_type_tag++; 
}

static int uint_tag = halua_newtypetag();

static int uint_tostring(lua_State* L) {
    lua_pushstring(L, std::to_string(*halua_touint(L, 1)).c_str());
    return 1;
}

bool halua_isuint(lua_State* L, int idx) {
    return lua_userdatatag(L, idx) == uint_tag;
}

uint32_t* halua_touint(lua_State* L, int idx) {
    return static_cast<uint32_t*>(lua_touserdatatagged(L, idx, uint_tag));
}

uint32_t* halua_newuint(lua_State *L, uint32_t uint) {
    if (luaL_newmetatable(L, "uint")) {
        const luaL_Reg meta[] = {
            {"__tostring", uint_tostring},
            {nullptr, nullptr}
        };
        luaL_register(L, nullptr, meta);
        lua_pushstring(L, "uint");
        lua_setfield(L, -2, "__type");
    }
    lua_pop(L, 1);
    uint32_t* p = static_cast<uint32_t*>(lua_newuserdatatagged(L, sizeof(uint32_t), uint_tag));
    luaL_getmetatable(L, "uint");
    lua_setmetatable(L, -2);
    *p = uint;
    return p;
}
static const int opaque_tag = halua_newtypetag();
halua_Opaque halua_newopaque(lua_State* L, halua_Opaque opaque) {
    halua_Opaque* p = static_cast<halua_Opaque*>(lua_newuserdatatagged(L, sizeof(halua_Opaque), opaque_tag));
    *p = opaque;
    return *p;
}
halua_Opaque halua_toopaque(lua_State *L, int idx) {
    halua_Opaque* p = static_cast<halua_Opaque*>(lua_touserdatatagged(L, idx, opaque_tag));
    return *p;
}
bool halua_isopaque(lua_State *L, int idx) {
    return lua_userdatatag(L, idx) == opaque_tag;
}