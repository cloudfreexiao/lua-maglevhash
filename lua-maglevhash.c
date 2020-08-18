#define LUA_LIB

#include <lua.h>
#include <lauxlib.h>

#include "maglevhash.h"

static void *
check_userdata(lua_State *L, int idx)
{
    void *ret = lua_touserdata(L, idx);
    luaL_argcheck(L, ret != NULL, idx, "Userdata should not be NULL");
    return ret;
}

static void
create_meta(lua_State *L, luaL_Reg *l, const char *name, lua_CFunction tostring, lua_CFunction gcfunc)
{
    int n = 0;
    while (l[n].name)
        ++n;
    lua_newtable(L);
    lua_createtable(L, 0, n);
    int i;
    for (i = 0; i < n; i++)
    {
        lua_pushcfunction(L, l[i].func);
        lua_setfield(L, -2, l[i].name);
    }
    lua_setfield(L, -2, "__index");
    lua_pushstring(L, name);
    lua_setfield(L, -2, "__metatable");
    if (tostring)
    {
        lua_pushcfunction(L, tostring);
        lua_setfield(L, -2, "__tostring");
    }

    if (gcfunc)
    {
        lua_pushcfunction(L, gcfunc);
        lua_setfield(L, -2, "__gc");
    }
}

typedef struct s_maglevhash
{
    struct MAGLEV_LOOKUP_HASH *psrv;
};

static int
lnew(lua_State *L)
{
    struct MAGLEV_LOOKUP_HASH *psrv = (struct MAGLEV_LOOKUP_HASH *)malloc(sizeof(*psrv));
    if (!psrv)
    {
        lua_pushnil(L);
        return 1;
    }
    maglev_init(psrv);

    struct s_maglevhash *lhash = (struct s_maglevhash *)lua_newuserdata(L, sizeof(*lhash));
    memset(lhash, 0, sizeof(*lhash));

    lhash->psrv = psrv;

    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setmetatable(L, -2);
    return 1;
}

static int
lrelease(lua_State *L)
{
    struct s_maglevhash *lhash = (struct s_maglevhash *)check_userdata(L, 1);
    struct MAGLEV_LOOKUP_HASH *psrv = lhash->psrv;
    free(lhash->psrv);
    lhash->psrv = NULL;
    return 0;
}

static int
lupdate_service(lua_State *L)
{
    struct s_maglevhash *lhash = (struct s_maglevhash *)check_userdata(L, 1);
    int node_size = (int)luaL_checknumber(L, 2);
    int hash_bucket_size = (int)luaL_checknumber(L, 3);

    int res = maglev_update_service(lhash->psrv, node_size, hash_bucket_size);
    lua_pushnumber(L, res);
    return 1;
}

static int
ladd_node(lua_State *L)
{
    struct s_maglevhash *lhash = (struct s_maglevhash *)check_userdata(L, 1);

    const char *node_name_key = luaL_checkstring(L, 2);
    const char *rs_info = luaL_checkstring(L, 3);

    int res = maglev_add_node(lhash->psrv, node_name_key, rs_info);
    lua_pushnumber(L, res);
    return 1;
}

static int
lcreate_ht(lua_State *L)
{
    struct s_maglevhash *lhash = (struct s_maglevhash *)check_userdata(L, 1);
    maglev_create_ht(lhash->psrv);
    return 0;
}

static int
lswap_entry(lua_State *L)
{
    struct s_maglevhash *lhash = (struct s_maglevhash *)check_userdata(L, 1);
    maglev_swap_entry(lhash->psrv);
    return 0;
}

static int
llookup_node(lua_State *L)
{
    struct s_maglevhash *lhash = (struct s_maglevhash *)check_userdata(L, 1);

    size_t len;
    const char *key = luaL_tolstring(L, 2, &len);

    void *pnode_info = maglev_lookup_node(lhash->psrv, key, len);
    if (!pnode_info)
    {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, (const char *)pnode_info);
    return 1;
}

static void
lmaglevhash(lua_State *L)
{
    luaL_Reg l[] = {
        {"update_service", lupdate_service},
        {"add_node", ladd_node},
        {"create_ht", lcreate_ht},
        {"swap_entry", lswap_entry},
        {"lookup_node", llookup_node},
        {NULL, NULL},
    };
    create_meta(L, l, "maglevhash", NULL, lrelease);
    lua_pushcclosure(L, lnew, 1);
}

LUAMOD_API int
luaopen_maglevhash_core(lua_State *L)
{
    luaL_checkversion(L);
    lua_newtable(L);

    lmaglevhash(L);
    lua_setfield(L, -2, "maglevhash");
    return 1;
}