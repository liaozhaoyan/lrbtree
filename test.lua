---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by liaozhaoyan.
--- DateTime: 2024/1/13 1:01 PM
---

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

local r = lrbtree.new(function(t1, t2) return t1[1] - t2[1] end)
local t1 = {1}
local t2 = {2}
local t3 = {3}
local t4 = {4}
local t5 = {5}

r:insert(t1)
r:insert(t3)
r:insert(t5)
r:insert(t2)
r:insert(t4)

r:delete(t3)

print("--- range")
for i, v in ipairs(r:range(t2, t4)) do
    print(i, v, v[1])
end

print("--- walk")
for i, v in r:walk() do
    print(i, v, v[1])
end

assert(r:pop()[1] == 1)
assert(r:pop()[1] == 2)
assert(r:pop()[1] == 4)
assert(r:pop()[1] == 5)
assert(not r:pop())
assert(not r:pop())
