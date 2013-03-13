
This is luaxml2e version 0.1 rev. 2

Visit <https://github.com/andrepiske/luaxml2e>




Node
====

node:addChild(child)
--------------------
Adds *child* to *node*. The added child will set as not owned.

node:childElementCount()
------------------------
Returns how much nodes that are children of *node* and are
of type 'element'.
The following code does the same:

~~~~.lua
function Node:childElementCount()
    local child = self:getFirstChild()
    local n = 0
    while child ~= nil do
        if child:getType() == 'element' then
            n = n + 1
        end
        child = child:nextSibling()
    end
    return n
end
~~~~

node:copyNode(extended)
-----------------------
Returns a copy of *node*.
The value of *extended* is the same of xmlCopyNode of libxml2


node:firstElementChild()
------------------------
Returns the first child of *node* or nil. The returned child
is owned by its parent.

node:getLastChild()
-------------------
Returns the last child of *node* or nil. The returned child
is owned by its parent.

node:getLineNo()
----------------
Returns the line number where the tag has been found when
a file is parsed by luaxml2e.parseDoc()

node:getNodePath()
------------------
Returns the node path.
The following code will print
"Mary Jane path is /Foo/Bar/Name[2]"
~~~~.lua
XML = require('luaxml2e')
local doc = XML.parseDoc([[
<?xml version="1.0" ?>
<Foo>
    <Bar>
        <Name id="1">John Doe</Name>
        <Name id="2">Mary Jane</Name>
    </Bar>
</Foo>
]])
local bar = doc:getRoot():firstElementChild()
local c = bar:firstElementChild()
while c ~= nil do
    if c:getProp('id') == '2' then
        print('Mary Jane path is ' .. c:getNodePath())
    end
    c = c:nextSibling()
end
~~~~

node:getProp(name)
------------------
Returns the property value of *name* or nil.

**Note**: Currently there is no way to handle more than
one property with the same name.

node:hasProp(name)
------------------
Returns a boolean of whether *node* has a property
named *name*.

node:isBlankNode()
------------------
Returns a boolean of whether *node* is a blank node.

The meaning of blank node is the same as in xmlIsBlankNode of libxml2.
Its documentation says:<br>
"Checks whether this node is an empty or whitespace only (and possibly ignorable) text-node."

node:lastElementChild()
-----------------------
Like node:firstElementChild(), but returns the last child.

node:newProp(name, value)
-------------------------
Adds a property with name *name* and value *value* to *node*.

Note that there if there exists more than one property with the same name, no
checks will be made.

node:setProp() might be more useful.
~~~~.lua
XML = require('luaxml2e')
local doc = XML.newDoc("1.0")
local root = XML.newNode('Foo')
local node = XML.newNode('Bar')
doc:setRoot(root)
root:addChild(node)
node:newProp('first', 'John')
node:newProp('last', 'Doe')
node = XML.newNode('Qux')
root:addChild(node)
node:newProp('foo', 'Bar')
node:newProp('foo', 'Baz')
print(doc:dump(true))
~~~~
The code above will produce the following XML:
~~~~.xml
<?xml version="1.0"?>
<Foo>
  <Bar first="John" last="Doe"/>
  <Qux foo="Bar" foo="Baz"/>
</Foo>
~~~~


node:getContent()
-----------------
Gets the text content of a node. For a more detailed explanation how this
works, see the xmlNodeGetContent of libxml2.

This function returns a string.

Here follows an example that outputs "Hello World.":
~~~~.lua
XML = require('luaxml2e')
local doc = XML.parseDoc([[
<?xml version="1.0" ?>
<Foo>
    <Bar>
        Hello <Baz e="2.71828">World</Baz>.
    </Bar>
</Foo>
]])
print(doc:getRoot():firstElementChild():getContent():match('^%s*(.-)%s*$'))
~~~~


node:isText()
-------------
Same as node:getType()=='text'.

node:setContent(value)
-----------------
Sets the node text content to *value*.

For detailed explanation on how this works, look the
function xmlNodeSetContent of libxml2


node:setName(name)
-------------------
Sets the name of *node* to *name*.

node:removeProp(name)
---------------------
Removes the property with name *name* from *node*.

node:setProp(name, value)
-------------------------
Sets the property with name *name* of *node* to *value*. *value* must not be nil.

node:getPropNames()
-------------------
Returns a table (a list) of properties names declared in *node*.

node:getName()
--------------
Returns the *node* name (its tag name).

node:getFirstChild()
Returns the first *node* child. Note the
first child may be a node of any type.
For the first element child the function node:firstElementChild() may be used.

node:getParent()
-----------------
Returns the parent node of *node* or nil. The returned node is owned.

node:nextSibling()
-------------------
Returns the next sibling node of *node* or nil. The returned node is owned.

node:prevSibling()
-------------------
Returns the previous sibling node of *node* or nil. The returned node is owned.

node:getDoc()
--------------
Returns the document associated to *node* or nil. The returned doc is owned.

node:getType()
---------------
Returns the node type of *node*.
This can be one of "element", "attribute", "text", "cdata", "entity-ref",
"entity", "pi", "comment", "document", "document-type",
"document-frag", "notation", "html-document", "dtd",
"element-decl", "attribute-decl", "entity-decl",
"namespace-decl", "xinclude-start", "xinclude-end" or "docb-document".

Doc
===

doc:copyDoc(recursive)
-----------------------
Returns a copy of *doc*. Further information on how this works may be found
on function xmlCopyDoc of libxml2.

doc:dump([format])
-----------------
Returns the XML string of *doc*.

If *format* is true then the XML string if formatted. This argument
is optional and false by default.

~~~~.lua
XML = require('luaxml2e')
local doc = XML.newDoc()
local node = XML.newNode('Bar')
doc:setRoot(node)
for i=1,5,1 do
    local n = XML.newNode('Foo' .. i)
    node:addChild(n)
    node = n
end
print(doc:dump(true))
print(doc:dump())
~~~~

The code above will produce the following:
~~~~.xml
<?xml version="1.0"?>
<Bar>
  <Foo1>
    <Foo2>
      <Foo3>
        <Foo4>
          <Foo5/>
        </Foo4>
      </Foo3>
    </Foo2>
  </Foo1>
</Bar>

<?xml version="1.0"?>
<Bar><Foo1><Foo2><Foo3><Foo4><Foo5/></Foo4></Foo3></Foo2></Foo1></Bar>
~~~~

doc:setRoot(node)
------------------
Sets *node* the root node of *doc*. This sets *node* as owned.

doc:getRoot()
--------------
Returns the root *node* of *doc* or nil. The result node is owned.

luaxml2e
========

luaxml2e.newDoc([version])
--------------------------
Returns a new XML document. The resulting doc is owned.

The argument *version* is optional and defaults to "1.0".

luaxml2e.newNode(name)
---------------------
Returns a new node with name *name*. The resulting node is owned.

luaxml2e.parseDoc(text)
-----------------------
Parses a XML string and returns a document. The resulting
doc is owned.

*text* must be a string.

To parse a XML file you must first read it to a variable,
this avoids security issues.

This is one way of reading a XML file:
~~~~.lua
XML = require('luaxml2e')
local f = io.open('foo.xml', 'rt')
local doc = XML.parseDoc(f:read('*all'))
f:close()
if doc == nil then
    print('Oh there, error message has been printed to stderr')
else
    ... lots of important stuff here ...
end
~~~~

<span style="font-size: 10pt; font-family: serif;">
luaxml2e version 0.1-2 as of 2012-12-18
<br>
<https://github.com/andrepiske/luaxml2e>
</span>

