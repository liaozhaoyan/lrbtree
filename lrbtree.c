/*************************************************************************
FileName: lrbtree.c
Author: turbobhh
Mail: turbobhh@gmail.com
CreatedTime: Tue 25 Apr 2017 03:09:31 PM CST
Adjust for luarocks: liaozhaoyan
************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include "rbtree.h"

#ifndef lua_setuservalue
#define lua_setuservalue(L, n) lua_setfenv(L, n)
#endif
#ifndef lua_getuservalue
#define lua_getuservalue(L, n) lua_getfenv(L, n)
#endif

#define MT_NAME "LRBTREE_HANDLE"
#define CHECK_RBTREE(L, n) ((rbroot_t*)luaL_checkudata(L, n, MT_NAME))

typedef struct rb_root rbroot_t;
typedef struct rb_node rbnode_t;

typedef struct {
    rbnode_t rb;
}l_node_t;

static int l_new(lua_State* L);
static int l_insert(lua_State* L);
static int l_delete(lua_State* L);
static int l_range(lua_State* L);
static int l_iterator(lua_State* L);
static int l_walk(lua_State* L);
static int l_first(lua_State* L);
static int l_last(lua_State* L);
static int l_gc(lua_State* L);

static int compare(lua_State* L, l_node_t* nodea, l_node_t* nodeb);
static l_node_t *get_node(lua_State *L, int rbtree_idx, int value_idx);

static int compare(lua_State* L, l_node_t* nodea, l_node_t* nodeb) {
    int top = lua_gettop(L);
    int fret, ret;

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "value_map");
    lua_getfield(L, -2, "comp_func"); 	/*value_map, comp_func*/

    lua_pushlightuserdata(L, nodea);
    lua_rawget(L, -3);		/*value_map, comp_func, valueA*/
    lua_pushlightuserdata(L, nodeb);
    lua_rawget(L, -4);		/*value_map, comp_func, valueA, valueB*/

    fret = lua_pcall(L, 2, 1, 0);
    if (fret != 0) {
        return luaL_error(L, "comp error %s", lua_tostring(L, -1));
    }

    ret = lua_tointeger(L, -1);
    lua_settop(L, top);
    return ret;
}

static l_node_t *get_node(lua_State *L, int rbtree_idx, int value_idx) {
    int top = lua_gettop(L);

    lua_getuservalue(L, rbtree_idx);
    lua_getfield(L, -1, "node_map");

    lua_pushvalue(L, value_idx);
    lua_rawget(L, -2);

    l_node_t* p = lua_isuserdata(L, -1) ? lua_touserdata(L, -1) : NULL;
    lua_settop(L, top);
    return p;
}

static int new(lua_State* L) {
    rbroot_t* root;
    luaL_checktype(L, 1, LUA_TFUNCTION);

    root = (rbroot_t*)lua_newuserdata(L, sizeof(rbroot_t));
    root->rb_node = NULL;

    luaL_getmetatable(L, MT_NAME);
    lua_setmetatable(L, -2);

    lua_newtable(L);
    lua_pushvalue(L, 1);
    lua_setfield(L, -2, "comp_func");

    lua_newtable(L);
    lua_setfield(L, -2, "value_map"); /* k = node_ptr, value = lua_value*/

    lua_newtable(L);
    lua_setfield(L, -2, "node_map"); /* k = lua_value, value = node_ptr*/
    lua_setuservalue(L, -2);
    return 1;
}

static int l_len(lua_State* L) {
    rbroot_t* root;
    root = CHECK_RBTREE(L, 1);

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "node_map");
    luaL_checktype(L, -1, LUA_TTABLE);
    lua_pushinteger(L, lua_objlen(L, -1));
    return 1;
}

static int l_insert(lua_State* L) {
    rbroot_t* root;
    l_node_t* node;
    root = CHECK_RBTREE(L, 1);
    if (lua_isnoneornil(L, 2)) {
        lua_pushnil(L);
        return 1;
    }

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "node_map");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
    if (!lua_isnil(L, -1)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pop(L, 3);

    node = malloc(sizeof(l_node_t));
    if (NULL == node) {
        lua_pushnil(L);
        return 1;
    }

    lua_getuservalue(L, 1);

    lua_getfield(L, -1, "node_map");
    lua_pushvalue(L, 2);
    lua_pushlightuserdata(L, (void*)node);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)node);
    lua_pushvalue(L, 2);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    rbnode_t **new = &root->rb_node, *parent = NULL;
    while(*new) {
        parent = *new;
        l_node_t* cur = rb_entry(parent, l_node_t, rb);
        new = compare(L, node, cur) < 0 ? &parent->rb_left : &parent->rb_right;
    }
    rb_link_node(&node->rb, parent, new);
    rb_insert_color(&node->rb, root);

    lua_pushboolean(L, 1);
    return 1;
}

static int l_delete(lua_State* L) {
    rbroot_t* root;
    l_node_t* node;

    root = CHECK_RBTREE(L, 1);
    if (lua_isnoneornil(L, 2)) {
        lua_pushnil(L);
        return 1;
    }

    node = get_node(L, 1, 2);
    if (NULL == node) {
        lua_pushnil(L);
        return 1;
    }

    lua_getuservalue(L, 1);

    lua_getfield(L, -1, "node_map");
    lua_pushvalue(L, 2);
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)node);
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    rb_erase(&node->rb, root);
    free(node);

    lua_pushboolean(L, 1);
    return 1;
}

static int l_exists(lua_State* L) {
    rbroot_t* root;
    l_node_t* node;

    root = CHECK_RBTREE(L, 1);
    if (lua_isnoneornil(L, 2)) {
        lua_pushnil(L);
        return 1;
    }

    node = get_node(L, 1, 2);
    (void)root;
    lua_pushboolean(L, node != NULL);
    return 1;
}

static int l_range(lua_State* L) {
    rbroot_t* root;
    rbnode_t* rbnode_from;
    rbnode_t* rbnode_to;
    rbnode_t* rbnode_cur;
    l_node_t* node;
    int i;

    root = CHECK_RBTREE(L, 1);
    rbnode_from = lua_isnoneornil(L, 2) ? rb_first(root) : &get_node(L, 1, 2)->rb;
    rbnode_to = lua_isnoneornil(L, 3) ? rb_last(root) : &get_node(L, 1, 3)->rb;
    if (NULL == rbnode_from || NULL == rbnode_to) {
        lua_newtable(L);
        return 1;
    }

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "value_map");

    lua_newtable(L);
    for (rbnode_cur = rbnode_from, i = 1; rbnode_cur; rbnode_cur = rb_next(rbnode_cur), ++i){
        node = rb_entry(rbnode_cur, l_node_t, rb);
        lua_pushlightuserdata(L, (void *)node);
        lua_rawget(L, -3);
        lua_rawseti(L, -2, i);
        if (rbnode_cur == rbnode_to) {
            break;
        }
    }
    return 1;
}

static int l_iterator(lua_State* L) {
    rbroot_t* root;
    l_node_t* node;
    l_node_t* next;
    int index;

    root = CHECK_RBTREE(L, lua_upvalueindex(1));
    (void)root;
    index = lua_tointeger(L, lua_upvalueindex(2));
    node = lua_touserdata(L, lua_upvalueindex(3));
    if (NULL == node) {
        lua_pushnil(L);
        return 1;
    }

    next = rb_entry(rb_next(&node->rb), l_node_t, rb);

    lua_pushinteger(L, index + 1);
    lua_replace(L, lua_upvalueindex(2));

    lua_pushlightuserdata(L, next);
    lua_replace(L, lua_upvalueindex(3));

    lua_getuservalue(L, lua_upvalueindex(1));
    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)node);
    lua_rawget(L, -2);

    lua_pushinteger(L, index);
    lua_pushvalue(L, -2);
    return 2;
}

static int l_walk(lua_State* L) {
    rbroot_t* root;
    l_node_t* first;

    root = CHECK_RBTREE(L, 1);
    first = rb_entry(rb_first(root), l_node_t, rb);

    lua_pushvalue(L, 1);
    lua_pushinteger(L, 1);
    lua_pushlightuserdata(L, (void*)first);
    lua_pushcclosure(L, l_iterator, 3);
    return 1;
}

static int l_first(lua_State* L) {
    rbroot_t* root;
    l_node_t* first;

    root = CHECK_RBTREE(L, 1);
    first = rb_entry(rb_first(root), l_node_t, rb);
    if (NULL == first) {
        lua_pushnil(L);
        return 1;
    }

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)first);
    lua_rawget(L, -2);
    return 1;
}

static int l_pop(lua_State* L) {
    rbroot_t* root;
    l_node_t* node;

    root = CHECK_RBTREE(L, 1);
    node = rb_entry(rb_first(root), l_node_t, rb);
    if (NULL == node) {
        lua_pushnil(L);
        return 1;
    }

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)node);
    lua_rawget(L, -2);

    lua_insert(L, 1);
    lua_settop(L, 2);   // now stack is retValue, self, size is 2

    lua_getuservalue(L, 2);

    lua_getfield(L, -1, "node_map");
    lua_pushvalue(L, 1);  // value
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)node);
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    rb_erase(&node->rb, root);
    free(node);

    lua_pushvalue(L, 1);
    return 1;
}

static int l_last(lua_State* L) {
    rbroot_t* root;
    l_node_t* last;

    root = CHECK_RBTREE(L, 1);
    last = rb_entry(rb_last(root), l_node_t, rb);
    if (NULL == last) {
        lua_pushnil(L);
        return 1;
    }

    lua_getuservalue(L, 1);
    lua_getfield(L, -1, "value_map");
    lua_pushlightuserdata(L, (void*)last);
    lua_rawget(L, -2);
    return 1;
}

static int l_gc(lua_State* L)
{
    rbroot_t* root;
    rbnode_t* rbnode, *rbnext;
    l_node_t* node;
    root = CHECK_RBTREE(L, 1);

    for (rbnode = rb_first(root); rbnode; rbnode = rbnext) {
        node = rb_entry(rbnode, l_node_t, rb);
        rb_erase(&node->rb, root);
        rbnext = rb_next(rbnode);
        free(node);
    }
    return 0;
}

static luaL_Reg module_m[] = {
        {"insert", l_insert},
        {"len", l_len},
        {"delete", l_delete},
        {"exists", l_exists},
        {"range", l_range},
        {"walk", l_walk},
        {"first", l_first},
        {"last", l_last},
        {"pop", l_pop},
        {NULL, NULL},
};

static luaL_Reg module_f[] = {
        {"new", new},
        {NULL, NULL}
};

int luaopen_lrbtree(lua_State* L) {
    luaL_newmetatable(L, MT_NAME);

    lua_createtable(L, 0, sizeof(module_m) / sizeof(luaL_Reg) - 1);
#if LUA_VERSION_NUM > 501
    luaL_setfuncs(L, module_m, 0);
#else
    luaL_register(L, NULL, module_m);
#endif
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, l_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, l_len);
    lua_setfield(L, -2, "__len");

    lua_pop(L, 1);

#if LUA_VERSION_NUM > 501
    luaL_newlib(L, module_f);
#else
    luaL_register(L, "lrbtree", module_f);
#endif

    return 1;
}
