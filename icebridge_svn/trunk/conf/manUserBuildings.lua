-- ARGV[1] 为uid; ARGV[2] 为建筑id
-- ARGV[3] 为坐标, ARGV[4] 为类别, ARGV[5]为等级, ARGV[6]为hp

local buildingHashKey = "build#"..ARGV[1];
local buildingUUID = ARGV[2];
local buildingInfo;

local buildingJsonStr = redis.call("HGET", buildingHashKey, buildingUUID);
if buildingJsonStr == false then
	buildingInfo = {["location"] =  ARGV[3], ["category"] = tonumber(ARGV[4]), ["class"] = tonumber(ARGV[5]), ["hp"] = tonumber(ARGV[6])};

else

	buildingInfo = cjson.decode(buildingJsonStr);
	buildingInfo["location"] = ARGV[3];
	buildingInfo["category"] = tonumber(ARGV[4]);
	buildingInfo["class"] = tonumber(ARGV[5]);
	buildingInfo["hp"] = tonumber(ARGV[6]);
end;

buildingJsonStr = cjson.encode(buildingInfo);
local ret = redis.call("HSET", buildingHashKey, buildingUUID, buildingJsonStr);
if ret == false then
	return 0
else
	return 1
end;

--return buildingJsonStr;

--local build = {["location"] = "400#800", ["category"] = 1, ["class"] = 2, ["hp"] = 300};
--local str = cjson.encode(build);
--local ret = redis.call("HSET", buildingHashKey, buildingUUID, str);
--return ret;
