local entity = Item.defaultExport();

if (entity == nil) then
	error("entity is nil");
end

local key, value = Item.getKv("portal");

local portal = "";

if (type(value) ~= "string") then
	if (type(key) ~= "string") then
		error("portal kv key or value must be string");
	else
		portal = key;
	end
else
	portal = value;
end

if (portal:sub(1, 1) == '%') then
	portal = portal:sub(2);
	portal = Item.instanceStr(portal);
end

portal = portal:lower();

local transform = mat4.newRotate(Item.getRotation());
local forward = transform:mulv3(vec3.forward);

local dot = vec3.dot(forward, vec3.up);

if (dot > 0.5) then
	portal = portal:gsub("%%surf%%", "floor");
else
	portal = portal:gsub("%%surf%%", "wall");
end

entity:setInstanceKv("base", portal);
