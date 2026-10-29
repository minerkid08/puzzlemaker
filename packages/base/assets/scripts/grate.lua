require("frame");
require("utils");

local transform = Item.getFloorTransform();
local pos = Item.getPosition();
local size = Item.getSize();

addPanelFrame(nil, size, transform);

local brush = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });
brush:setTexture(Direction.POS_X, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.NEG_X, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.POS_Y, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSNODRAW");
brush:setTexture(Direction.POS_Z, "METAL/METALGRATE018");
brush:setTexture(Direction.NEG_Z, "METAL/METALGRATE018");
brush:transformMat(transform);
local ent = Entity.new("grate", "func_brush", pos, {0, 0, 0});
ent:setKv("solidity", 1);
ent:attachBrush(brush);

brush = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });
brush:setTexture(Direction.POS_X, "TOOLS/TOOLSTRIGGER");
brush:setTexture(Direction.NEG_X, "TOOLS/TOOLSTRIGGER");
brush:setTexture(Direction.POS_Y, "TOOLS/TOOLSTRIGGER");
brush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSTRIGGER");
brush:setTexture(Direction.POS_Z, "TOOLS/TOOLSTRIGGER");
brush:setTexture(Direction.NEG_Z, "TOOLS/TOOLSTRIGGER");
brush:transformMat(transform);
ent = Entity.new("clip", "func_clip_vphysics", pos, {0, 0, 0});
ent:attachBrush(brush);

brush = Brush.new({ 0, 0, -1 / 64 }, { size[1], size[2], 1 / 64 });
brush:setTexture(Direction.POS_X, "TOOLS/TOOLSPLAYERCLIP");
brush:setTexture(Direction.NEG_X, "TOOLS/TOOLSPLAYERCLIP");
brush:setTexture(Direction.POS_Y, "TOOLS/TOOLSPLAYERCLIP");
brush:setTexture(Direction.NEG_Y, "TOOLS/TOOLSPLAYERCLIP");
brush:setTexture(Direction.POS_Z, "TOOLS/TOOLSPLAYERCLIP");
brush:setTexture(Direction.NEG_Z, "TOOLS/TOOLSPLAYERCLIP");
brush:transformMat(transform);
