---@class FizzlerProp
---@field static boolean
---@field model string
---@field openAnim string?
---@field closeAnim string?
---@field skin number?

---@param ent Entity
---@param prop FizzlerProp
---@param startDisabled boolean
---@param mat mat4
local function configProp(ent, prop, startDisabled, mat)
  ent:setKv("model", prop.model);
  if (not prop.static) then
    ent:setKv("holdanimation", true);
		if(prop.skin ~= nil) then
			ent:setKv("skin", prop.skin);
		end
    if (startDisabled) then
      ent:setKv("defaultanimation", prop.closeAnim);
    else
      ent:setKv("defaultanimation", prop.openAnim);
    end
  end
  ent:transformMat(mat);
end

---@param prop FizzlerProp
---@param startDisabled boolean
---@param sourceEntity Entity?
---@param mat mat4
function addFizzlerProps(prop, startDisabled, sourceEntity, mat)
  local tile = Item.getTile();
  local size = Item.getSize();

  local entityName = "prop_dynamic";
  if (prop.static) then
    entityName = "prop_static";
  end
  for i = 1, tile[2] do
    local yPos = i * 2 - 1;
    local leftProp = Entity.new("prop", entityName, { 0, yPos, 0 }, { 0, 0, 0 });
    configProp(leftProp, prop, startDisabled, mat);
    local rightProp = Entity.new("prop", entityName, { size[1], yPos, 0 }, { 0, 180, 0 });
    configProp(rightProp, prop, startDisabled, mat);
  end
  if (sourceEntity) then
    sourceEntity:addOutput("OnUser1", "prop", "setAnimation", prop.closeAnim);
    sourceEntity:addOutput("OnUser2", "prop", "setAnimation", prop.openAnim);
  end
end
