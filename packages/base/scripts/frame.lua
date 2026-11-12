---@class FrameCorner
---@field name string
---@field size number

---@class FrameConfig : table<number, string>
---@field corner FrameCorner
---@field zOffset number

---@type FrameConfig
local testchamberFrame = {
	[4] = "models/props_test_chamber/frame_4.mdl",
	[16] = "models/props_test_chamber/frame_16.mdl",
	[20] = "models/props_test_chamber/frame_20.mdl",
	[32] = "models/props_test_chamber/frame_32.mdl",
	[64] = "models/props_test_chamber/frame_64.mdl",
	[128] = "models/props_test_chamber/frame_128.mdl",
	corner = {
		name = "models/props_test_chamber/frame_corner.mdl",
		size = 24
	},
	zOffset = 4
};

---@param config FrameCorner?
---@param size vec2?
---@param transform mat4?
function addPanelFrame(config, size, transform)
	config = config or testchamberFrame;
	size = size or Item.getSize();
	transform = transform or Item.getTransform();

	local width = size[1] * 64 - config.corner.size * 2;
	local height = size[2] * 64 - config.corner.size * 2;

	local sizes = {};
	for k, _ in pairs(config) do
		if (type(k) == "number") then
			table.insert(sizes, k);
		end
	end

	table.sort(sizes, function(a, b) return a > b end);

	local widthUsed = config.corner.size;

	while (true) do
		local addedEnt = false;
		for _, n in ipairs(sizes) do
			if (width >= n) then
				local ent = Entity.new("prop", "prop_static", { widthUsed / 64, 0, config.zOffset / 64 }, { 0, 0, 0 });
				ent:setKv("model", config[n]);
				ent:transformMat(transform);
				ent = Entity.new("prop", "prop_static", { widthUsed / 64, size[2], -config.zOffset / 64 }, { 180, 0, 0 });
				ent:setKv("model", config[n]);
				ent:transformMat(transform);
				widthUsed = widthUsed + n;
				addedEnt = true;
				width = width - n;
				goto continue
			end
		end
		if (not addedEnt) then
			break;
		end
		::continue::
	end
	if (width > 0) then
		print(("frame width finished with %d extra units"):format(width));
	end

	local heightUsed = config.corner.size;

	while (true) do
		local addedEnt = false;
		for _, n in ipairs(sizes) do
			if (height >= n) then
				local ent = Entity.new("prop", "prop_static", { 0, heightUsed / 64, -config.zOffset / 64 }, { 180, 0, -90 });
				ent:setKv("model", config[n]);
				ent:transformMat(transform);
				ent = Entity.new("prop", "prop_static", { size[1], heightUsed / 64, config.zOffset / 64 }, { 0, 0, 90 });
				ent:setKv("model", config[n]);
				ent:transformMat(transform);
				heightUsed = heightUsed + n;
				addedEnt = true;
				height = height - n;
				goto continue
			end
		end
		if (not addedEnt) then
			break;
		end
		::continue::
	end
	if (height > 0) then
		print(("frame height finished with %d extra units"):format(height));
	end

	local e = 1/512;

	local ent = Entity.new("prop", "prop_static", { e, e, config.zOffset / 64 }, { 0, 0, 0 });
	ent:setKv("model", config.corner.name);
	ent:transformMat(transform);
	ent = Entity.new("prop", "prop_static", { e, size[2] - e, config.zOffset / 64 }, { 0, 0, -90 });
	ent:setKv("model", config.corner.name);
	ent:transformMat(transform);
	ent = Entity.new("prop", "prop_static", { size[1] - e, size[2] - e, config.zOffset / 64 }, { 0, 0, 180 });
	ent:setKv("model", config.corner.name);
	ent:transformMat(transform);
	ent = Entity.new("prop", "prop_static", { size[1] - e, e, config.zOffset / 64 }, { 0, 0, 90 });
	ent:setKv("model", config.corner.name);
	ent:transformMat(transform);
end
