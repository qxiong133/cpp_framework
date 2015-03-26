local hashkey = "user#"..ARGV[1];

--木头
local wood = redis.call("HGET", hashkey, "abt#wood"); 

--石头
local stone = redis.call("HGET", hashkey, "abt#stone");

--矿石
local ore = redis.call("HGET", hashkey, "abt#ore");

--混凝土
local concrete = redis.call("HGET", hashkey, "abt#concrete");

if wood == false then
	wood = "0"
end

if stone == false then
	stone = "0"
end

if ore == false then
	ore = "0"
end

if concrete == false then
	concrete = "0"
end

return {wood, stone, ore, concrete};
