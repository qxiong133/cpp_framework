-- ARGV[1]为uid; ARGV[2] = 1 表示增加资源, ARGV[2] = 2 表示减除资源
--ARGV[3,4,5,6]分别为4类资源数量
local hashKey = "user#"..ARGV[1];
local manType = ARGV[2];
local ret;
local isSuccess = true;
--return manType
--print(manType);
if manType == "1" then
	redis.call("HINCRBY", hashKey, "abt#wood", ARGV[3]);
	redis.call("HINCRBY", hashKey, "abt#stone", ARGV[4]);
	redis.call("HINCRBY", hashKey, "abt#ore", ARGV[5]);
	redis.call("HINCRBY", hashKey, "abt#concrete", ARGV[6]);
elseif manType == "2" then
	ret = redis.call("HINCRBY", hashKey, "abt#wood", 0 - ARGV[3]);
	--资源数量不足
	if ret < 0 then
		isSuccess = false;
		--资源复位
		redis.call("HINCRBY", hashKey, "abt#wood", ARGV[3]);
		return isSuccess;
	end;
	redis.call("HINCRBY", hashKey, "abt#stone", 0 - ARGV[4]);
	if ret < 0 then
		isSuccess = false;
		--资源复位
		redis.call("HINCRBY", hashKey, "abt#wood", ARGV[3]);
		redis.call("HINCRBY", hashKey, "abt#stone", ARGV[4]);
		return isSuccess;
	end;
	redis.call("HINCRBY", hashKey, "abt#ore", 0 - ARGV[5]);
	if ret < 0 then
		isSuccess = false;
		--资源复位
		redis.call("HINCRBY", hashKey, "abt#wood", ARGV[3]);
		redis.call("HINCRBY", hashKey, "abt#stone", ARGV[4]);
		redis.call("HINCRBY", hashKey, "abt#ore", ARGV[5]);
		return isSuccess;
	end;
	redis.call("HINCRBY", hashKey, "abt#concrete", 0 - ARGV[6]);
	if ret < 0 then
		isSuccess = false;
		--资源复位
		redis.call("HINCRBY", hashKey, "abt#wood", ARGV[3]);
		redis.call("HINCRBY", hashKey, "abt#stone", ARGV[4]);
		redis.call("HINCRBY", hashKey, "abt#ore", ARGV[5]);
		redis.call("HINCRBY", hashKey, "abt#concrete", ARGV[6]);
		return isSuccess;
	end;
else

end;

return isSuccess
