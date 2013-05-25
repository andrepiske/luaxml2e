local XML = require('luaxml2e')

local setmetatable = setmetatable
local getmetatable = getmetatable
local print = print
local type = type

module('luaxml2es')

local function wrapobj(obj, mt)
    if obj == nil then
        return nil
    end
    local o = { ['obj']=obj }
    setmetatable(o, mt)
    return o
end

local function mt_index(self, key)
    local t = getmetatable(self)
    local r = t[key]
    if r == nil then
        r = t._props[key](self)
    end
    return r
end

local function property(d, name, proc)
    d._props[name] = proc
end

local doc_mt = { ['_props']={}, ['__index']=mt_index }
local node_mt = { ['_props']={}, ['__index']=mt_index }

local wrapdoc = function(obj) return wrapobj(obj, doc_mt) end
local wrapnode = function(obj) return wrapobj(obj, node_mt) end

function newdoc(version)
    return wrapdoc(XML.newDoc(version))
end

function newnode(name)
    local node = XML.newNode(name)
    return wrapnode(node)
end

function parse(text)
    return wrapdoc(XML.parseDoc(text))
end

function doc_mt:dump(format)
    return self.obj:dump(format)
end

property(doc_mt, 'root', function(self)
    return wrapnode(self.obj:getRoot())
end)

function doc_mt:setroot(node)
    self.obj:setRoot(node~=nil and node.obj or nil)
    return node
end

function doc_mt:__tostring()
    return self.obj:dump()
end

local function children_it(parent, nodetype)
    local fc = parent.obj:getFirstChild()
    while fc ~= nil and nodetype ~= nil and fc:getType() ~= nodetype do
        fc = fc:nextSibling()
    end
    if fc == nil then
        return function() end
    end
    return function(s, v)
        if v == nil then
            return wrapnode(fc)
        end
        v = v.obj
        v = v:nextSibling()
        while v ~= nil and nodetype ~= nil and v:getType() ~= nodetype do
            v = v:nextSibling()
        end
        return wrapnode(v)
    end
end

property(node_mt, 'children', function(self)
    return children_it(self)
end)

property(node_mt, 'elements', function(self)
    return children_it(self, 'element')
end)

function node_mt:getchildren(nodetype)
    return children_it(self, nodetype)
end

function node_mt:find(name)
    local c = self.obj:getFirstChild()
    while c ~= nil do
        if c:getType() == 'element' and c:getName() == name then
            return wrapnode(c)
        end
        c = c:nextSibling()
    end
    return nil
end

property(node_mt, 'next', function(self)
    return wrapnode(self.obj:nextSibling())
end)

property(node_mt, 'prev', function(self)
    return wrapnode(self.obj:prevSibling())
end)

property(node_mt, 'name', function(self)
    return self.obj:getName()
end)

function node_mt:__tostring()
    return self.obj:getContent()
end

function node_mt:__len()
    return self.obj:childElementCount()
end

function node_mt:setcontent(text)
    self.obj:setContent(text)
    return self
end

property(node_mt, 'path', function(self)
    return self.obj:getPath()
end)

property(node_mt, 'type', function(self)
    return self.obj:getType()
end)

function node_mt:setname(name)
    self.obj:setName(name)
end

property(node_mt, 'parent', function(self)
    return wrapnode(self.obj:getParent())
end)

property(node_mt, 'doc', function(self)
    return wrapdoc(self.obj:getDoc())
end)

property(node_mt, 'props', function(self)
    return self.obj:getPropNames()
end)

property(node_mt, 'firstchild', function(self)
    return self.obj:getFirstChild()
end)

property(node_mt, 'content', function(self)
    return self.obj:getContent()
end)

property(node_mt, 'trimmed', function(self)
    return self.obj:getContent():match('^%s*(.-)%s*$')
end)

function node_mt:prop(name, value)
    if value ~= nil then
        self.obj:setProp(name, value)
    else
        value = self.obj:getProp(name)
    end
    return value
end

function node_mt:add(name_or_node) -- already wrapped node I mean
    local wrpnode = name_or_node
    if type(wrpnode) == 'string' then
        wrpnode = wrapnode(XML.newNode(wrpnode)) -- as name
    end
    self.obj:addChild(wrpnode.obj)
    return wrpnode
end


