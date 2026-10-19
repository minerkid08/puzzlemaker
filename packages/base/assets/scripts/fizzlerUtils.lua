---@class FizzlerProp
---@field static boolean
---@field model string
---@field openAnim string?
---@field closeAnim string?

---@param ent Entity
---@param prop FizzlerProp
---@param startDisabled boolean
---@param pos vec3
---@param rot vec3
local function configProp(ent, prop, startDisabled, pos, rot)
  ent:setKv("model", prop.model);
  if (not prop.static) then
    ent:setKv("holdanimation", true);
    if (startDisabled) then
      ent:setKv("defaultanimation", prop.closeAnim);
    else
      ent:setKv("defaultanimation", prop.openAnim);
    end
  end
  ent:transform({ 0, 0, 0 }, { 90, 0, 0 });
  ent:transform(pos, rot);
end

---@param prop FizzlerProp
---@param startDisabled boolean
---@param sourceEntity Entity?
function addFizzlerProps(prop, startDisabled, sourceEntity)
  local tile = Item.getTile();
  local pos = Item.getPosition();
  local rot = Item.getRotation();
  local size = Item.getSize();

  local entityName = "prop_dynamic";
  if (prop.static) then
    entityName = "prop_static";
  end
  for i = 1, tile[2] do
    local yPos = i * 2 - 1;
    local leftProp = Entity.new("prop", entityName, { 0, yPos, 0 }, { 0, 0, 0 });
    configProp(leftProp, prop, startDisabled, pos, rot);
    local rightProp = Entity.new("prop", entityName, { size[1], yPos, 0 }, { 0, 180, 0 });
    configProp(rightProp, prop, startDisabled, pos, rot);
  end
  if (sourceEntity) then
    sourceEntity:addOutput("OnUser1", "prop", "setAnimation", prop.closeAnim);
    sourceEntity:addOutput("OnUser2", "prop", "setAnimation", prop.openAnim);
  end
end
