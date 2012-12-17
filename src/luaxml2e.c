#include <stdio.h>
#include <stdlib.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <lua.h>
#include <lauxlib.h>
#include <string.h>

#define XS(a) ((xmlChar*)(a))

typedef void(*LuaObj_freeproc)(void*);

typedef struct _LuaObj {
    void *ptr;
    int attached;
    LuaObj_freeproc freeproc;
    luaL_Reg *procs;
} LuaObj;

////////// Node Functions /////////////////////////////////////////////////////

int l_node_addchild(lua_State *L);
int l_node_childelementcount(lua_State *L);
int l_node_copynode(lua_State *L);
int l_node_firstelementchild(lua_State *L);
int l_node_getlastchild(lua_State *L);
int l_node_getlineno(lua_State *L);
int l_node_getnodepath(lua_State *L);
int l_node_getprop(lua_State *L);
int l_node_hasprop(lua_State *L);
int l_node_isblanknode(lua_State *L);
int l_node_lastelementchild(lua_State *L);
int l_node_newprop(lua_State *L);
int l_node_getcontent(lua_State *L);
int l_node_istext(lua_State *L);
int l_node_setcontent(lua_State *L);
int l_node_setname(lua_State *L);
int l_node_previouselementsibling(lua_State *L);
int l_node_removeProp(lua_State *L);
int l_node_getname(lua_State *L);
int l_node_getfirstchild(lua_State *L);
int l_node_getparent(lua_State *L);
int l_node_nextsibling(lua_State *L);
int l_node_prevsibling(lua_State *L);
int l_node_getdoc(lua_State *L);
int l_node_gettype(lua_State *L);

static luaL_Reg xmlnode_lreg[] = {
    { "addChild", l_node_addchild },
    { "childElementCount", l_node_childelementcount },
    { "copyNode", l_node_copynode },
    { "firstElementChild", l_node_firstelementchild },
    { "getLastChild", l_node_getlastchild },
    { "getLineNo", l_node_getlineno },
    { "getNodePath", l_node_getnodepath },
    { "getProp", l_node_getprop },
    { "hasProp", l_node_hasprop },
    { "isBlankNode", l_node_isblanknode },
    { "lastElementChild", l_node_lastelementchild },
    { "newProp", l_node_newprop },
    { "getContent", l_node_getcontent },
    { "isText", l_node_istext },
    { "setContent", l_node_setcontent },
    { "setName", l_node_setname },
    { "previousElementSibling", l_node_previouselementsibling },
    { "removeProp", l_node_removeProp },

    // from properties
    { "getName", l_node_getname },
    { "getFirstChild", l_node_getfirstchild },
    { "getParent", l_node_getparent },
    { "nextSibling", l_node_nextsibling },
    { "prevSibling", l_node_prevsibling },
    { "getDoc", l_node_getdoc },
    { "getType", l_node_gettype },

    // TODO { "copyprop", l_node_copyprop },
    { 0, 0 }
};

////////// Document Functions /////////////////////////////////////////////////

int l_doc_copydoc(lua_State *L);
int l_doc_getrootelement(lua_State *L);
int l_doc_setrootelement(lua_State *L);
int l_doc_dump(lua_State *L);

static luaL_Reg xmldoc_lreg[] = {
    { "copyDoc", l_doc_copydoc },
    { "dump", l_doc_dump },
    { "getRoot", l_doc_getrootelement },
    { "setRoot", l_doc_setrootelement },
    { 0, 0 }
};

////////// module Functions ///////////////////////////////////////////////////

int l_newdoc(lua_State *L);
int l_newnode(lua_State *L);
int l_newcomment(lua_State *L);
int l_newtext(lua_State *L);
int l_parsedoc(lua_State *L);

static luaL_Reg lreg[] = {
    { "newDoc", l_newdoc },
    { "newNode", l_newnode },
    { "newComment", l_newcomment },
    { "newText", l_newtext },
    { "parseDoc", l_parsedoc },
    { 0, 0 }
};


///////////////////////////////////////////////////////////////////////////////

static int LuaObj_gc(lua_State *L) {
    // TODO: take account to refcount
    return 0;
}

static int LuaObj_index(lua_State *L) {
    LuaObj *obj;
    const char *fname;
    luaL_Reg *R;
    obj = (LuaObj*)lua_touserdata(L, 1);
    fname = lua_tostring(L, 2);
    R = obj->procs;
    for (; R && R->name; R++) {
        if (!strcmp(fname, R->name)) {
            lua_pushcfunction(L, R->func);
            return 1;
        }
    }
    return 0;
}

LuaObj *luaobj_new(lua_State *L, LuaObj_freeproc fp, void *ptr,
    int attached, luaL_Reg *R)
{
    LuaObj *p;
    p = (LuaObj*)lua_newuserdata(L, sizeof(LuaObj));
    lua_newtable(L);
    lua_pushcfunction(L, LuaObj_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, LuaObj_index);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    p->freeproc = fp;
    p->ptr = ptr;
    p->attached = attached;
    p->procs = R;
    return p;
}

void luaobj_setattach(lua_State *L, int index, int attached) {
    LuaObj *obj;
    obj = (LuaObj*)lua_touserdata(L, index);
    obj->attached = attached;
}

void *luaobj_ptr(lua_State *L, int index) {
    if (lua_type(L, index) != LUA_TUSERDATA) {
        lua_pushstring(L, "luaobj_ptr: not an userdata");
        lua_error(L);
    }
    return ((LuaObj*)lua_touserdata(L, index))->ptr;
}

static
void pushdoc(lua_State *L, xmlDocPtr docptr, int attached) {
    if (!docptr) {
        lua_pushnil(L);
    } else {
        luaobj_new(L, (LuaObj_freeproc)xmlFreeDoc, (void*)docptr, attached,
            xmldoc_lreg);
    }
}

static
void pushnode(lua_State *L, xmlNodePtr nodeptr, int attached) {
    if (!nodeptr) {
        lua_pushnil(L);
    } else {
        luaobj_new(L, (LuaObj_freeproc)xmlFreeNode, (void*)nodeptr, attached,
            xmlnode_lreg);
    }
}

///////////////////////////////////////////////////////////////////////////////

int l_doc_copydoc(lua_State *L) {
    xmlDocPtr doc;
    xmlDocPtr newdoc;
    doc = (xmlDocPtr)luaobj_ptr(L, 1);
    newdoc = xmlCopyDoc(doc, lua_toboolean(L, 2));
    pushdoc(L, newdoc, 0);
    return 1;
}

int l_doc_getrootelement(lua_State *L) {
    xmlDocPtr doc;
    xmlNodePtr node;
    doc = (xmlDocPtr)luaobj_ptr(L, 1);
    node = xmlDocGetRootElement(doc);
    pushnode(L, node, 1);
    return 1;
}

int l_doc_setrootelement(lua_State *L) {
    xmlDocPtr doc;
    xmlNodePtr node;
    doc = (xmlDocPtr)luaobj_ptr(L, 1);
    node = (xmlNodePtr)luaobj_ptr(L, 2);
    xmlDocSetRootElement(doc, node);
    luaobj_setattach(L, 2, 1);
    return 0;
}

int l_doc_dump(lua_State *L) {
    xmlDocPtr doc;
    xmlChar *str = 0;
    int format = 0;
    int size = 0;
    doc = (xmlDocPtr)luaobj_ptr(L, 1);
    if (lua_toboolean(L, 2))
        format = 1;
    xmlDocDumpFormatMemory(doc, &str, &size, format);
    if (str) {
        lua_pushlstring(L, (const char*)str, (size_t)size);
        xmlFree(str);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int l_node_addchild(lua_State *L) {
    xmlNodePtr parent;
    xmlNodePtr cur;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    cur = (xmlNodePtr)luaobj_ptr(L, 2);
    xmlAddChild(parent, cur);
    luaobj_setattach(L, 2, 1);
    return 0;
}

int l_node_childelementcount(lua_State *L) {
    xmlNodePtr parent;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    lua_pushnumber(L, (lua_Number)xmlChildElementCount(parent));
    return 1;
}

int l_node_copynode(lua_State *L) {
    xmlNodePtr cur;
    xmlNodePtr newnode;
    int extended;
    cur = (xmlNodePtr)luaobj_ptr(L, 1);
    extended = (int)luaL_checknumber(L, 2);
    newnode = xmlCopyNode(cur, extended);
    pushnode(L, newnode, 0);
    return 1;
}

int l_node_firstelementchild(lua_State *L) {
    xmlNodePtr parent;
    xmlNodePtr child;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    child = xmlFirstElementChild(parent);
    pushnode(L, child, 1);
    return 1;
}

int l_node_getlastchild(lua_State *L) {
    xmlNodePtr parent;
    xmlNodePtr child;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    child = xmlGetLastChild(parent);
    pushnode(L, child, 1);
    return 1;
}

int l_node_getlineno(lua_State *L) {
    xmlNodePtr parent;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    lua_pushnumber(L, (lua_Number)xmlGetLineNo(parent));
    return 1;
}

int l_node_getnodepath(lua_State *L) {
    xmlNodePtr parent;
    xmlChar *s;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    s = xmlGetNodePath(parent);
    lua_pushstring(L, (const char *)s);
    xmlFree(s);
    return 1;
}

int l_node_getprop(lua_State *L) {
    xmlNodePtr parent;
    const char *pname;
    xmlChar *value;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    pname = luaL_checkstring(L, 2);
    value = xmlGetProp(parent, XS(pname));
    if (!value) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, (const char*)value);
        xmlFree(value);
    }
    return 1;
}

int l_node_hasprop(lua_State *L) {
    xmlNodePtr parent;
    const char *pname;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    pname = luaL_checkstring(L, 2);
    lua_pushboolean(L, xmlHasProp(parent, XS(pname)) ? 1 : 0);
    return 1;
}

int l_node_isblanknode(lua_State *L) {
    lua_pushboolean(L, (int)xmlIsBlankNode((xmlNodePtr)luaobj_ptr(L, 1)));
    return 1;
}

int l_node_lastelementchild(lua_State *L) {
    xmlNodePtr parent;
    xmlNodePtr child;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    child = xmlLastElementChild(parent);
    pushnode(L, child, 1);
    return 1;
}

int l_node_newprop(lua_State *L) {
    xmlNodePtr parent;
    const char *name;
    const char *value;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    name = luaL_checkstring(L, 2);
    value = luaL_checkstring(L, 3);
    xmlNewProp(parent, XS(name), XS(value));
    return 0;
}

int l_node_getcontent(lua_State *L) {
    xmlNodePtr cur;
    xmlChar *content;
    cur = (xmlNodePtr)luaobj_ptr(L, 1);
    content = xmlNodeGetContent(cur);
    if (!content) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, (const char*)content);
        xmlFree(content);
    }
    return 1;
}

int l_node_istext(lua_State *L) {
    lua_pushboolean(L, (int)xmlNodeIsText((xmlNodePtr)luaobj_ptr(L, 1)));
    return 1;
}

int l_node_setcontent(lua_State *L) {
    xmlNodePtr cur;
    const char *content;
    cur = (xmlNodePtr)luaobj_ptr(L, 1);
    content = luaL_checkstring(L, 2);
    xmlNodeSetContent(cur, XS(content));
    return 0;
}

int l_node_setname(lua_State *L) {
    xmlNodePtr cur;
    const char *name;
    cur = (xmlNodePtr)luaobj_ptr(L, 1);
    name = luaL_checkstring(L, 2);
    xmlNodeSetName(cur, XS(name));
    return 0;
}

int l_node_previouselementsibling(lua_State *L) {
    xmlNodePtr parent;
    xmlNodePtr sib;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    sib = xmlPreviousElementSibling(parent);
    pushnode(L, sib, 1);
    return 1;
}

int l_node_removeProp(lua_State *L) {
    xmlNodePtr parent;
    const char *pname;
    xmlAttrPtr attr;
    parent = (xmlNodePtr)luaobj_ptr(L, 1);
    pname = luaL_checkstring(L, 2);
    attr = xmlHasProp(parent, XS(pname));
    if (attr)
        xmlRemoveProp(attr);
    return 0;
}

int l_node_getname(lua_State *L) {
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    lua_pushstring(L, (const char*)node->name);
    return 1;
}

int l_node_getfirstchild(lua_State *L) {
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    pushnode(L, node->children, 1);
    return 1;
}

int l_node_getparent(lua_State *L) {
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    pushnode(L, node->parent, 1);
    return 1;
}

int l_node_nextsibling(lua_State *L) {
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    pushnode(L, node->next, 1);
    return 1;
}

int l_node_prevsibling(lua_State *L) {
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    pushnode(L, node->prev, 1);
    return 1;
}

int l_node_getdoc(lua_State *L) {
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    pushdoc(L, node->doc, 1);
    return 1;
}

int l_node_gettype(lua_State *L) {
    const char *lty[] = {
        0, "element", "attribute", "text", "cdata", "entity-ref",
        "entity", "pi", "comment", "document", "document-type",
        "document-frag", "notation", "html-document", "dtd",
        "element-decl", "attribute-decl", "entity-decl",
        "namespace-decl", "xinclude-start", "xinclude-end",
        "docb-document" };
    xmlNodePtr node;
    node = (xmlNodePtr)luaobj_ptr(L, 1);
    lua_pushstring(L, lty[node->type]);
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int l_newdoc(lua_State *L) {
    xmlDocPtr xmldoc;
    const char *ver;
    
    if (lua_type(L, 1) == LUA_TSTRING)
        ver = lua_tostring(L, -1);
    else
        ver = "1.0";

    xmldoc = xmlNewDoc(XS(ver));
    pushdoc(L, xmldoc, 0);
    return 1;
}

int l_newnode(lua_State *L) {
    xmlNodePtr nodeptr;
    const char *name = luaL_checkstring(L, 1);
    nodeptr = xmlNewNode(0, XS(name));
    pushnode(L, nodeptr, 0);
    return 1;
}

int l_newcomment(lua_State *L) {
    xmlNodePtr nodeptr;
    const char *content;
    content = luaL_checkstring(L, 1);
    nodeptr = xmlNewComment(XS(content));
    pushnode(L, nodeptr, 0);
    return 1;
}

int l_newtext(lua_State *L) {
    xmlNodePtr nodeptr;
    const char *content;
    content = luaL_checkstring(L, 1);
    nodeptr = xmlNewText(XS(content));
    pushnode(L, nodeptr, 0);
    return 1;
}

int l_parsedoc(lua_State *L) {
    xmlDocPtr doc;
    const char *str;
    str = luaL_checkstring(L, 1);
    doc = xmlParseDoc(XS(str));
    pushdoc(L, doc, 0);
    return 1;
}

int luaopen_luaxml2e(lua_State *L) {
    lua_newtable(L);
    luaL_register(L, 0, lreg);
    return 1;
}




