require("fizzlerUtils");
require("utils");

local pos = Item.getPosition();
local size = Item.getSize();
local tile = Item.getTile();

local transform = Item.getFloorTransform();

local startDisabled = Item.getKv("startdisabled");

---@type FizzlerProp
local propConfig = {
	static = false,
	model = "models/props/fizzler_dynamic.mdl",
	openAnim = "open",
	closeAnim = "close"
};

---@type Entity
local mainEnt = nil;

for i = 1, tile[2] do
	local postfix = tostring(i);
	local ent = Entity.new("trigger" .. postfix, "trigger_portal_cleanser", pos, { 0, 0, 0 });
	ent:setKv("usescanline", true);
	ent:setKv("spawnflags", 9);
	ent:setKv("solid", 6);
	ent:setKv("startdisabled", startDisabled);
	if (mainEnt == nil) then
		mainEnt = ent;
		mainEnt:markAsIO();
	end

	local yMin = (i - 1) * size[2];
	local yMax = i * size[2];
	local leftBrush = Brush.new({ 0, yMin, -1 / 64 }, { 1, yMax, 1 / 64 });
	local rightBrush = Brush.new({ size[1] - 1, yMin, -1 / 64 }, { size[1], yMax, 1 / 64 });

	local brushes = { leftBrush, rightBrush };
	if (size[1] > 2) then
		local centerBrush = Brush.new({ 1, yMin, -1 / 64 }, { size[1] - 1, yMax, 1 / 64 });
		table.insert(brushes, centerBrush);

		centerBrush:setTexture(Direction.POS_Z, "EFFECTS/FIZZLER_CENTER", { texSize = 1024, fit = true });
		centerBrush:setTexture(Direction.NEG_Z, "EFFECTS/FIZZLER_CENTER", { texSize = 1024, fit = true });
	end

	leftBrush:setTexture(Direction.POS_Z, "EFFECTS/FIZZLER_L", { texSize = 1024, fit = true });
	leftBrush:setTexture(Direction.NEG_Z, "EFFECTS/FIZZLER_R", { texSize = 1024, fit = true });

	rightBrush:setTexture(Direction.POS_Z, "EFFECTS/FIZZLER_R", { texSize = 1024, fit = true });
	rightBrush:setTexture(Direction.NEG_Z, "EFFECTS/FIZZLER_L", { texSize = 1024, fit = true });

	for _, brush in pairs(brushes) do
		brush:setTexture(Direction.POS_X, "TOOLS/TOOLSTRIGGER");
		brush:setTexture(Direction.NEG_X, "TOOLS/TOOLSTRIGGER");
		brush:setTexture(Direction.POS_Y, "TOOLS/TOOLSTRIGGER");
		brush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSTRIGGER");

		brush:transformMat(transform);
		ent:attachBrush(brush);
	end

	mainEnt:addOutput("OnUser1", ent, "Disable");
	mainEnt:addOutput("OnUser2", ent, "Enable");
end

---@cast startDisabled boolean
addFizzlerProps(propConfig, startDisabled, mainEnt, transform);
