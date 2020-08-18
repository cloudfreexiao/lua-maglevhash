
https://github.com/sunwsh/maglev_hash

```lua
local hash = require "maglevhash.core"
local maglev = hash.maglevhash()
assert(maglev)

local REAL_SERVER_NUMB = 3
local res = maglev:update_service(REAL_SERVER_NUMB, 313)

for i=1, REAL_SERVER_NUMB do
    local res = maglev:add_node(tostring(i), "rs:" .. i)
end
maglev:create_ht()
maglev:swap_entry()
for i=1, REAL_SERVER_NUMB do
    local node = maglev:lookup_node(tostring(i))
    print("node->", node)
end
local nodeid = 5
maglev:add_node(tostring(nodeid), "rs:" .. nodeid)
maglev:create_ht()
maglev:swap_entry()

local node = maglev:lookup_node(tostring(nodeid))
print("node->", node)
`
