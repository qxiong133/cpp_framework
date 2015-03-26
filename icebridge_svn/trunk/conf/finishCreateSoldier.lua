-- ARGV[1] 表示uid; ARGV[2] 表示士兵类别; ARGV[3]表示士兵等级, ARGV[4] 表示士兵所需人口

local isSuccess = "OK"; -- 失败则是"-ERR "
local soldierID;

local hashKey = "user#"..ARGV[1];
local listKey = "army#"..ARGV[1];

--获取人口上限
local popu = redis.call("HGET", hashKey, "abt#popu");
if popu == false then
	popu = 50
end;

--获取当前人口, 如果读取失败, 定当前人口为0
local curPopu = redis.call("HGET", hashKey, "abt#curPopu");
if curPopu == false then
	curPopu = 0
end;

local soldierPopu = ARGV[4];
local soldierField = "army#"..ARGV[2]
local soldier = {};

if (popu - curPopu) < tonumber(soldierPopu) then
	soldierID = -1;
else
	--该类士兵数量+1
	redis.call("HINCRBY", hashKey, soldierField, 1);
	--当前人口增加
	redis.call("HINCRBY", hashKey, "abt#curPopu", soldierPopu);
	--插链表
	--计数器
	local index = 0;
	soldier = redis.call("LRANGE", listKey, 0, -1);
	
	local soldierSize = redis.call("LLEN", listKey);
	--local soldierSize = getn(soldier);	
	for k,v in ipairs(soldier) do  
		if v == "#" then 
			break;
		end
		index = index + 1;
	end 
	--ID
	soldierID = index;
	
	if index == soldierSize then
		--插入list 尾部
		redis.call("RPUSH", listKey, soldierField);
	else
		redis.call("LSET", listKey, index, soldierField);
	end;
end;

return soldierID;

