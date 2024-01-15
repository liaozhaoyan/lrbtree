# lrbtree
lua wrap for rbtree, refer to [lrbtree](https://github.com/bhhbazinga/lrbtree)

# Installation

The rapidest way to install lrbtree is using the package management tools like luarocks.

```
luarocks install lrbtree
```

# example

```
local lrbtree = require "lrbtree"

local testSize = 1000000

local rbtree = lrbtree.new(function(a, b)
    return a - b
end)

local t1 = os.clock()
for i = 1, testSize do
    rbtree:insert(i)
end
local t2 = os.clock()

for idx, v in rbtree:walk() do
    --	print(idx, v)
end
assert(#rbtree == testSize, "bade rbtree len.")

local list = rbtree:range(1, #rbtree)
for idx, v in ipairs(list) do
    --	print(idx, v)
end
assert(#list == testSize, "bade rbtree range.")

local t3 = os.clock()
for _, v in rbtree:walk() do
    rbtree:delete(v)
end
local t4 = os.clock()
assert(#rbtree == 0, "tree is not empty.")

print("after delete all")
for idx, v in rbtree:walk() do
    print(idx, v)
end

print(string.format("insert %d elements:", testSize), t2 - t1)
print(string.format("delete %d elements:", testSize), t4 - t3)

```

# module

## lrbtree

lrbtree provides the basic API for tree operations.

lrbtree function:

### new

```
local rbtree = lrbtree.new(function(a, b)
    return a - b
end)
```

create a new rbtree data structure.

*  param: compare callback function.
*  return: new rbtree structure.

callback function: to compare value between two member. Both parameters need to be of the same type.

lrbtree member functions, call method rbtree:insert(i).

### insert

Inserts a member into the tree structure

*  param: member.
*  return: success return true, otherwise false.

### len

get the number of members of the whole tree.  

```
local len = rbtree:len()
local len = #rbtree   --Equivalent method
```

### delete

delete member from the rbtree

*  param: member.
*  return: exists then delete return true, otherwise false.

### exists

Check if the member is in the tree structure

*  param: member.
*  return: exists return true, otherwise false.

### range

*  param from: member begin.
*  param to: member end.
*  return: member table.

### walk

```
for _, v in rbtree:walk() do
    rbtree:delete(v)
end
```

* return iterator
* iterator sequeue number of member, lua_number type.
* iterator member.

### first

* return the first value of rbtree

### pop

* return and pop out the first value of rbtree


### last

* return the last value of rbtree