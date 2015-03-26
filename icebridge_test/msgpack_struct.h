#ifndef MSGPACK_STRUCT_H_
#define MSGPACK_STRUCT_H_

#include <msgpack.hpp>
#include <vector>
#include <map>
#include "msgpack_id.h"

using namespace std;


struct ServerChallengeRequest{
	int  RandomNumber;
	MSGPACK_DEFINE(RandomNumber);
};

struct ServerChallengeResponse{
	int  RandomNumber;
	MSGPACK_DEFINE(RandomNumber);
};

struct HeartBeatRequest{
	int  temp;
	MSGPACK_DEFINE(temp);
};

struct VersionInforResponse{
	string  build_version;
	string  arms_version;
	string  client_version;
	string  update_URL;
	string  skill_version;
	MSGPACK_DEFINE(build_version,arms_version,client_version,update_URL,skill_version);
};

struct ArmsConfigRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct BuildConfigRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct SkillConfigRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct ArmsResponse{
	short  type;
	int  level;
	int  hp;
	int  defense;
	int  attack;
	float  move_speed;
	float  fire_rate;
	int  need_wood;
	int  need_stone;
	int  need_iron;
	int  need_soil;
	int  population;
	int  build_time;
	int  available;
	string  magic_skill;
	string  name;
	int  attack_type;
	float  attack_range;
	float  keep_time;
	int  defense_type;
	MSGPACK_DEFINE(type,level,hp,defense,attack,move_speed,fire_rate,need_wood,need_stone,need_iron,need_soil,population,build_time,available,magic_skill,name,attack_type,attack_range,keep_time,defense_type);
};

struct ArmsListResponse{
	unsigned char  flags;
	vector<ArmsResponse>  arms_list;
	MSGPACK_DEFINE(flags,arms_list);
};

struct BuildResponse{
	string  name;
	short  big_type;
	short  type;
	short  level;
	int  need_wood;
	int  need_stone;
	int  need_iron;
	int  need_soil;
	int  need_diamond;
	int  build_time;
	int  output;
	int  space;
	int  max_hp;
	int  attack;
	int  defense;
	float  fire_rate;
	float  attack_speed;
	string  attack_effect;
	string  build_describe;
	MSGPACK_DEFINE(name,big_type,type,level,need_wood,need_stone,need_iron,need_soil,need_diamond,build_time,output,space,max_hp,attack,defense,fire_rate,attack_speed,attack_effect,build_describe);
};

struct BuildListResponse{
	unsigned char  flags;
	vector<BuildResponse>  build_list;
	MSGPACK_DEFINE(flags,build_list);
};

struct SkillData{
	string  name;
	int  level;
	int  type;
	int  attack_type;
	int  value;
	float  range;
	float  keep_time;
	float  cold_time;
	int  need_mp;
	int  magic_union_level;
	int  need_soil;
	MSGPACK_DEFINE(name,level,type,attack_type,value,range,keep_time,cold_time,need_mp,magic_union_level,need_soil);
};

struct SkillConfigResponse{
	unsigned char  flags;
	vector<SkillData>  skill_config;
	MSGPACK_DEFINE(flags,skill_config);
};

struct RegisterRequest{
	int  race;
	string  name;
	MSGPACK_DEFINE(race,name);
};

struct AccountInfoResponse{
	string  account;
	string  passwd;
	MSGPACK_DEFINE(account,passwd);
};

struct SetNameRequest{
	string  name;
	MSGPACK_DEFINE(name);
};

struct SimpleResponse{
	int  msgid;
	int   sn;
	int   result;
	MSGPACK_DEFINE(msgid,sn,result);
};

struct BindEmailRequest{
	string  Email;
	string  Passwd;
	MSGPACK_DEFINE(Email,Passwd);
};

struct ArmsStruct{
	int  type;
	int  level;
	int  num;
	MSGPACK_DEFINE(type,level,num);
};

struct MyBuildDataResponse{
	string  build_sn;
	int  big_type;
	int  status;
	long  ctime;
	long  time;
	int  build_type;
	int  build_level;
	int  cur_hp;
	int  build_skin;
	int  cur_resource;
	float  x;
	float  y;
	MSGPACK_DEFINE(build_sn,big_type,status,ctime,time,build_type,build_level,cur_hp,build_skin,cur_resource,x,y);
};

struct MyAttruibuteResponse{
	int  player_index;
	int  diamond;
	int  race;
	int  avoid_time;
	int  workers;
	int  wood;
	int  stone;
	int  iron;
	int  soil;
	int  record;
	int  level;
	int  worldmap_x;
	int  worldmap_y;
	string  name;
	string  city_hall_uuid;
	map<string,MyBuildDataResponse>  my_build;
	int  backpack_num;
	int  recuite_type;
	int  chance_num;
	map<int,int>  build_num;
	string  combat_uuid;
	int  groupId;
	int  group_position;
	int  magic_union_level;
	MSGPACK_DEFINE(player_index,diamond,race,avoid_time,workers,wood,stone,iron,soil,record,level,worldmap_x,worldmap_y,name,city_hall_uuid,my_build,backpack_num,recuite_type,chance_num,build_num,combat_uuid,groupId,group_position,magic_union_level);
};

struct EnemyAttruibuteResponse{
	int  avoid_time;
	int  race;
	int  workers;
	int  wood;
	int  stone;
	int  iron;
	int  soil;
	int  record;
	int  level;
	int  worldmap_x;
	int  worldmap_y;
	string  name;
	string  city_hall_uuid;
	map<string,MyBuildDataResponse>  my_build;
	MSGPACK_DEFINE(avoid_time,race,workers,wood,stone,iron,soil,record,level,worldmap_x,worldmap_y,name,city_hall_uuid,my_build);
};

struct AttackEnemyRequest{
	string  enemy_name;
	string  hero_uuid;
	MSGPACK_DEFINE(enemy_name,hero_uuid);
};

struct BuildStatus{
	string  build_sn;
	float  x;
	float  y;
	MSGPACK_DEFINE(build_sn,x,y);
};

struct BuildStatusListRequest{
	vector<BuildStatus>  build_status_list;
	MSGPACK_DEFINE(build_status_list);
};

struct BuildCreateRequest{
	int  build_type;
	float  x;
	float  y;
	MSGPACK_DEFINE(build_type,x,y);
};

struct BuildUpgradeRequest{
	string  build_sn;
	MSGPACK_DEFINE(build_sn);
};

struct AssetResponse{
	int  wood;
	int  stone;
	int  iron;
	int  soil;
	int  diamond;
	MSGPACK_DEFINE(wood,stone,iron,soil,diamond);
};

struct DiamondReponse{
	int  diamond;
	MSGPACK_DEFINE(diamond);
};

struct GetMyAssetRequest{
	string  build_sn;
	MSGPACK_DEFINE(build_sn);
};

struct LoginRequest{
	string  account;
	string  Passwd;
	MSGPACK_DEFINE(account,Passwd);
};

struct GetRandNameRequest{
	int  sex;
	MSGPACK_DEFINE(sex);
};

struct RandNameResponse{
	string  rand_name;
	MSGPACK_DEFINE(rand_name);
};

struct EquipResponse{
	int   witch_pannle;
	string  where_uuid;
	int  position;
	string  equip_sn;
	int  part;
	int  quality;
	int  level;
	int  attr1_type;
	int  attr1_value;
	int  attr2_type;
	int  attr2_value;
	int  extral1_type;
	int  extral1_value;
	int  extral2_type;
	int  extral2_value;
	int  diamond_cost;
	MSGPACK_DEFINE(witch_pannle,where_uuid,position,equip_sn,part,quality,level,attr1_type,attr1_value,attr2_type,attr2_value,extral1_type,extral1_value,extral2_type,extral2_value,diamond_cost);
};

struct EquipListResponse{
	vector<EquipResponse>  equip_list;
	MSGPACK_DEFINE(equip_list);
};

struct DressEquipRequest{
	int  which_hero;
	int  backpack_position;
	int  hero_position;
	MSGPACK_DEFINE(which_hero,backpack_position,hero_position);
};

struct UnloadEquipRequest{
	int  which_hero;
	int  position;
	MSGPACK_DEFINE(which_hero,position);
};

struct SaleGoodsRequest{
	int  position;
	int  diamond_cost;
	MSGPACK_DEFINE(position,diamond_cost);
};

struct LookMarketGoodsRequest{
	int  page_num;
	MSGPACK_DEFINE(page_num);
};

struct MarketGoodsResponse{
	string  player;
	long  remain_time;
	int  diamond_cost;
	int  goods_type;
	int  position;
	string  equip_sn;
	int  part;
	int  quality;
	int  level;
	int  attr1_type;
	int  attr1_value;
	int  attr2_type;
	int  attr2_value;
	int  extral1_type;
	int  extral1_value;
	int  extral2_type;
	int  extral2_value;
	MSGPACK_DEFINE(player,remain_time,diamond_cost,goods_type,position,equip_sn,part,quality,level,attr1_type,attr1_value,attr2_type,attr2_value,extral1_type,extral1_value,extral2_type,extral2_value);
};

struct MarketListResponse{
	int  max_page_num;
	vector<MarketGoodsResponse>  maket_list;
	MSGPACK_DEFINE(max_page_num,maket_list);
};

struct BuyMarketGoodsRequest{
	string  goods_sn;
	MSGPACK_DEFINE(goods_sn);
};

struct OpenBoxRequest{
	int  box_sn;
	MSGPACK_DEFINE(box_sn);
};

struct HeroBase{
	float  base_leader;
	float  base_strategy;
	float  base_economy;
	float  base_strength;
	float  base_magic_power;
	float  base_hp;
	float  base_mp;
	float  base_attack;
	float  base_defence;
	MSGPACK_DEFINE(base_leader,base_strategy,base_economy,base_strength,base_magic_power,base_hp,base_mp,base_attack,base_defence);
};

struct HeroResponse{
	int  hero_sn;
	string  name;
	string  hero_uuid;
	int  quality;
	int  level;
	float  leader;
	float  strategy;
	float  economy;
	float  strength;
	float  magic_power;
	float  leader_grow;
	float  strategy_grow;
	float  economy_grow;
	float  strength_grow;
	float  magic_power_grow;
	int  cur_hp;
	int  max_hp;
	int  cur_mp;
	int  max_mp;
	int  attack;
	int  defence;
	int  pre_skill;
	int  study_skill;
	int  cur_goods_space;
	int  max_goods_space;
	int  exp;
	int  type;
	int  where;
	HeroBase  base_att;
	vector<ArmsStruct>  arms;
	MSGPACK_DEFINE(hero_sn,name,hero_uuid,quality,level,leader,strategy,economy,strength,magic_power,leader_grow,strategy_grow,economy_grow,strength_grow,magic_power_grow,cur_hp,max_hp,cur_mp,max_mp,attack,defence,pre_skill,study_skill,cur_goods_space,max_goods_space,exp,type,where,base_att,arms);
};

struct HeroListResponse{
	vector<HeroResponse>  hero_list;
	MSGPACK_DEFINE(hero_list);
};

struct RecruitRequest{
	int  random_type;
	MSGPACK_DEFINE(random_type);
};

struct RecruitResponse{
	int  chance_num;
	MSGPACK_DEFINE(chance_num);
};

struct GetRandHeroRequest{
	int  position;
	MSGPACK_DEFINE(position);
};

struct DropHeroRequest{
	int  hero_sn;
	MSGPACK_DEFINE(hero_sn);
};

struct GetMyAssetResponse{
	int  msg_id;
	int  add_wood;
	int  add_stone;
	int  add_iron;
	int  add_soil;
	MSGPACK_DEFINE(msg_id,add_wood,add_stone,add_iron,add_soil);
};

struct CityLimit{
	int  city_level;
	int  build_type;
	int  max_level;
	int  max_num;
	MSGPACK_DEFINE(city_level,build_type,max_level,max_num);
};

struct CityLimitListResponse{
	vector<CityLimit>  city_limit;
	MSGPACK_DEFINE(city_limit);
};

struct EmailSaleResponse{
	string  email_uuid;
	int  diamond;
	long  time;
	string  other_name;
	EquipResponse  equip;
	MSGPACK_DEFINE(email_uuid,diamond,time,other_name,equip);
};

struct EmailSaleListResponse{
	map<string,EmailSaleResponse>  email_sale_list;
	MSGPACK_DEFINE(email_sale_list);
};

struct GetEmailGoodsRequest{
	string  email_uuid;
	MSGPACK_DEFINE(email_uuid);
};

struct EmailNextPageRequst{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct BattleLogResponse{
	short  attack_mode;
	short  battle_damage;
	short  star;
	short  result;
	int  wood;
	int  stone;
	int  iron;
	int  soil;
	string  other_name;
	long  time;
	ArmsStruct  armsList;
	MSGPACK_DEFINE(attack_mode,battle_damage,star,result,wood,stone,iron,soil,other_name,time,armsList);
};

struct FastFinishRequest{
	string  build_sn;
	MSGPACK_DEFINE(build_sn);
};

struct UpgradeArmTypeRequest{
	int  arm_type;
	MSGPACK_DEFINE(arm_type);
};

struct MyArmpsLevelResponse{
	map<int,int>  my_arm_level;
	MSGPACK_DEFINE(my_arm_level);
};

struct AppointHeroRequest{
	string  hero_uuid;
	int  job_title;
	MSGPACK_DEFINE(hero_uuid,job_title);
};

struct CancelHeroRequest{
	string  hero_uuid;
	MSGPACK_DEFINE(hero_uuid);
};

struct SetArmsToHeroRequest{
	string  hero_uuid;
	vector<ArmsStruct>  arms;
	MSGPACK_DEFINE(hero_uuid,arms);
};

struct EnemyStatusResponse{
	int  flags;
	int  dest_x;
	int  dest_y;
	long  time;
	MSGPACK_DEFINE(flags,dest_x,dest_y,time);
};

struct QuikAttackRequest{
	string  enmy_name;
	string  hero_uuid;
	MSGPACK_DEFINE(enmy_name,hero_uuid);
};

struct LogoutRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct WorldMapNearByRequest{
	int  center_area;
	MSGPACK_DEFINE(center_area);
};

struct RandomNewMapRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct RandomMoveCityRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct SettleMoveCityRequest{
	int  x;
	int  y;
	MSGPACK_DEFINE(x,y);
};

struct MyWorldMapCoordResponse{
	int  x;
	int  y;
	MSGPACK_DEFINE(x,y);
};

struct HeroMoveRequest{
	int  x;
	int  y;
	MSGPACK_DEFINE(x,y);
};

struct WorldMapPlayerInfoResponse{
	unsigned char  city_level;
	unsigned char  city_face;
	unsigned char  player_race;
	string  player_name;
	MSGPACK_DEFINE(city_level,city_face,player_race,player_name);
};

struct WorldMapResourceResponse{
	unsigned char  resource_type;
	unsigned char  keeper_type;
	unsigned char  keeper_level;
	unsigned char  keeper_intensity;
	unsigned char  belong_race;
	string  belong_name;
	MSGPACK_DEFINE(resource_type,keeper_type,keeper_level,keeper_intensity,belong_race,belong_name);
};

struct WorldMapMonsterResponse{
	unsigned char  monster_type;
	unsigned char  monster_num;
	unsigned char  monster_intensity;
	MSGPACK_DEFINE(monster_type,monster_num,monster_intensity);
};

struct NearByPlayerInfoResponse{
	int  area;
	map<int,WorldMapPlayerInfoResponse>  player_info;
	MSGPACK_DEFINE(area,player_info);
};

struct NearByResourceResponse{
	int  area;
	map<int,WorldMapResourceResponse>  resource;
	MSGPACK_DEFINE(area,resource);
};

struct NearByMapMonsterResponse{
	int  area;
	map<int,WorldMapMonsterResponse>  monster;
	MSGPACK_DEFINE(area,monster);
};

struct WorlMapNearByResponse{
	int  area;
	map<int,WorldMapPlayerInfoResponse>  player_info;
	map<int,WorldMapResourceResponse>  resource;
	map<int,WorldMapMonsterResponse>  monster;
	MSGPACK_DEFINE(area,player_info,resource,monster);
};

struct NearByAreaResponse{
	vector<WorlMapNearByResponse>  near_area;
	MSGPACK_DEFINE(near_area);
};

struct WorldMapEnterCityRequest{
	int  area;
	MSGPACK_DEFINE(area);
};

struct WorldMapPlunderResourceRequest{
	int  x;
	int  y;
	MSGPACK_DEFINE(x,y);
};

struct LookUpCityRequest{
	string  player_name;
	MSGPACK_DEFINE(player_name);
};

struct LookUpCityResponse{
	int  remain_time;
	MSGPACK_DEFINE(remain_time);
};

struct AbleAttackCityResponse{
	int  x;
	int  y;
	MSGPACK_DEFINE(x,y);
};

struct PVEAttackRequest{
	int  x;
	int  y;
	MSGPACK_DEFINE(x,y);
};

struct PVEAttackResponse{
	int  result;
	MSGPACK_DEFINE(result);
};

struct KillMonsterRequest{
	int  type;
	int  level;
	MSGPACK_DEFINE(type,level);
};

struct ChangeWorlMapStatusResponse{
	int  key;
	int  flags;
	MSGPACK_DEFINE(key,flags);
};

struct PVEWinResponse{
	int  level;
	int  box_num;
	MSGPACK_DEFINE(level,box_num);
};

struct PlayerSimpleInfo{
	int  x;
	int  y;
	int  city_level;
	string  player_name;
	MSGPACK_DEFINE(x,y,city_level,player_name);
};

struct GetSomePlayerRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct SomePlayerResponse{
	vector<PlayerSimpleInfo>  world_player;
	MSGPACK_DEFINE(world_player);
};

struct StudySkillRequest{
	int  skill_type;
	int  skill_level;
	MSGPACK_DEFINE(skill_type,skill_level);
};

struct LeaveWordMapRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct WorldChannelMsgRequest{
	int  my_index;
	string  msg;
	MSGPACK_DEFINE(my_index,msg);
};

struct WorldChannelMsgResponse{
	int  player_index;
	string  name;
	string  msg;
	MSGPACK_DEFINE(player_index,name,msg);
};

struct UnionChannelRequest{
	int  my_index;
	string  msg;
	MSGPACK_DEFINE(my_index,msg);
};

struct UnionChannelResponse{
	int  player_index;
	string  msg;
	MSGPACK_DEFINE(player_index,msg);
};

struct PrivateChannelRequest{
	int  my_index;
	int  player_index;
	string  msg;
	MSGPACK_DEFINE(my_index,player_index,msg);
};

struct PrivateChannelResponse{
	int  player_index;
	string  name;
	string  msg;
	MSGPACK_DEFINE(player_index,name,msg);
};

struct SystemBroadCastResponse{
	string  msg;
	MSGPACK_DEFINE(msg);
};

struct FindFriendRequest{
	int  my_index;
	string  name;
	MSGPACK_DEFINE(my_index,name);
};

struct FindFriendResponse{
	int  flag;
	int  player_index;
	string  name;
	MSGPACK_DEFINE(flag,player_index,name);
};

struct CreateUnionRequest{
	int  flag;
	string  union_name;
	string  describe;
	MSGPACK_DEFINE(flag,union_name,describe);
};

struct MemberResponse{
	int  index;
	int  rank;
	string  name;
	MSGPACK_DEFINE(index,rank,name);
};

struct UnionInfoResponse{
	int  max_member;
	int  cur_member;
	vector<MemberResponse>  member;
	MSGPACK_DEFINE(max_member,cur_member,member);
};

struct LeaveUnionRequest{
	int  sn;
	MSGPACK_DEFINE(sn);
};

struct AddUnionRequest{
	string  union_name;
	MSGPACK_DEFINE(union_name);
};

struct SoldierInfoRequest{
	short   soldier_type;
	short   soldier_level;
	short   soldier_num;
	string  which_camp;
	short   operation;
	MSGPACK_DEFINE(soldier_type,soldier_level,soldier_num,which_camp,operation);
};

struct SoldierCampInfo{
	string  sid;
	string  camp_uuid;
	MSGPACK_DEFINE(sid,camp_uuid);
};

struct ChangeSoldierCampRequest{
	vector<SoldierCampInfo >  soldier_camp_info;
	MSGPACK_DEFINE(soldier_camp_info);
};

struct SoldierProcessResponse{
	short   soldier_type;
	short   soldier_level;
	short   soldier_num;
	string  which_camp;
	short   operation;
	bool  operation_status;
	int  wood;
	int  stone;
	int  iron;
	int  concrete;
	MSGPACK_DEFINE(soldier_type,soldier_level,soldier_num,which_camp,operation,operation_status,wood,stone,iron,concrete);
};

struct TimeIsUpResponse{
	short   soldier_type;
	short   soldier_level;
	short   soldier_num;
	string  which_camp;
	short   operation;
	string  soldier_id;
	MSGPACK_DEFINE(soldier_type,soldier_level,soldier_num,which_camp,operation,soldier_id);
};

struct CampInfo{
	short   soldier_type;
	short   soldier_level;
	short   soldier_num;
	MSGPACK_DEFINE(soldier_type,soldier_level,soldier_num);
};

struct VectorCampInfoResponse{
	vector<CampInfo>  camp_info;
	int  time_remain;
	int  request_time;
	MSGPACK_DEFINE(camp_info,time_remain,request_time);
};

struct CampInfoResponse{
	map<string, VectorCampInfoResponse>  camps_info;
	MSGPACK_DEFINE(camps_info);
};

struct CompleteAtOnceRequest{
	string  camp_id;
	int  diamond_num;
	MSGPACK_DEFINE(camp_id,diamond_num);
};

struct msgStartAttack{
	string  defenderUname;
	short  race;
	vector<string>  sidList;
	short  heroID;
	float  x;
	float  y;
	MSGPACK_DEFINE(defenderUname,race,sidList,heroID,x,y);
};

struct soldier{
	short  category;
	short  level;
	string  sid;
	string  camp_uuid;
	MSGPACK_DEFINE(category,level,sid,camp_uuid);
};

struct CompleteAtOnceResponse{
	int  diamond_num;
	string   which_camp;
	vector<soldier>  soldiers_info;
	MSGPACK_DEFINE(diamond_num,which_camp,soldiers_info);
};

struct soldiersInfoResponse{
	vector<soldier>  soldier_info;
	MSGPACK_DEFINE(soldier_info);
};

struct msgStartAtkToAtkerResponse{
	bool  dfderIsOnline;
	vector<soldier>  dfderSoldiers;
	short  heroID;
	MSGPACK_DEFINE(dfderIsOnline,dfderSoldiers,heroID);
};

struct msgStartAtkToDfderResponse{
	vector<soldier>  atkSoldiers;
	short  heroID;
	float  x;
	float  y;
	MSGPACK_DEFINE(atkSoldiers,heroID,x,y);
};

struct msgHeroCastSkillRequest{
	short  heroID;
	bool  isAtk;
	vector<string>  targets;
	short  skillID;
	float  x;
	float  y;
	MSGPACK_DEFINE(heroID,isAtk,targets,skillID,x,y);
};

struct msgHeroCastSkillResponse{
	short  heroID;
	bool  isAtk;
	vector<string>  targets;
	short  skillID;
	float  x;
	float  y;
	MSGPACK_DEFINE(heroID,isAtk,targets,skillID,x,y);
};

struct msgObjectDieRequest{
	string  objectID;
	bool  isBuilding;
	bool  isTownhall;
	MSGPACK_DEFINE(objectID,isBuilding,isTownhall);
};

struct msgObjectDieResponse{
	string  objectID;
	bool  isBuilding;
	bool  isTownhall;
	MSGPACK_DEFINE(objectID,isBuilding,isTownhall);
};

struct msgHeroMoveRequest{
	short  heroID;
	float  dstX;
	float  dstY;
	MSGPACK_DEFINE(heroID,dstX,dstY);
};

struct msgHeroMoveResponse{
	short  heroID;
	float  dstX;
	float  dstY;
	MSGPACK_DEFINE(heroID,dstX,dstY);
};

struct msgDfderLoginResponse{
	string  dfderUname;
	unsigned int  dfderUid;
	MSGPACK_DEFINE(dfderUname,dfderUid);
};

struct msgBeingAtkResponse{
	string  atkerUname;
	unsigned int  atkerUid;
	MSGPACK_DEFINE(atkerUname,atkerUid);
};

struct battleSoldier{
	string  sid;
	float  x;
	float  y;
	bool  tgtIsBuilding;
	string  targetID;
	short  curHP;
	short  category;
	short  level;
	MSGPACK_DEFINE(sid,x,y,tgtIsBuilding,targetID,curHP,category,level);
};

struct battleBuilding{
	string  bid;
	short  curHP;
	short  curRes;
	int  category;
	int  level;
	float  x;
	float  y;
	MSGPACK_DEFINE(bid,curHP,curRes,category,level,x,y);
};

struct msgCurBattleInfoRequest{
	vector<battleSoldier>  attackers;
	vector<battleSoldier>  defenders;
	vector<battleBuilding>  buildings;
	MSGPACK_DEFINE(attackers,defenders,buildings);
};

struct msgCurBattleInfoResponse{
	vector<battleSoldier>  attackers;
	vector<battleSoldier>  defenders;
	vector<battleBuilding>  buildings;
	MSGPACK_DEFINE(attackers,defenders,buildings);
};

struct stateSoldier{
	string  sid;
	float  x;
	float  y;
	bool  tgtIsBuilding;
	string  targetID;
	short  curHP;
	short  curState;
	MSGPACK_DEFINE(sid,x,y,tgtIsBuilding,targetID,curHP,curState);
};

struct stateBuilding{
	string  bid;
	short  curHP;
	short  curRes;
	MSGPACK_DEFINE(bid,curHP,curRes);
};

struct msgDfderIsreadyRequest{
	bool  isReady;
	MSGPACK_DEFINE(isReady);
};

struct msgDfderIsreadyResponse{
	bool  isReady;
	MSGPACK_DEFINE(isReady);
};

struct msgDfderLogoutResponse{
	bool  logout;
	MSGPACK_DEFINE(logout);
};

struct msgAtkerLogoutResponse{
	bool  logout;
	MSGPACK_DEFINE(logout);
};

struct msgAskCurStateResponse{
	bool  isReport;
	MSGPACK_DEFINE(isReport);
};

struct msgReportCurStateRequest{
	vector<stateSoldier>  selfSoldiers;
	MSGPACK_DEFINE(selfSoldiers);
};

struct msgReportCurStateResponse{
	vector<stateSoldier>  peerSoldiers;
	MSGPACK_DEFINE(peerSoldiers);
};

struct msgReportBattleResultToAtkerResponse{
	unsigned short  starNum;
	int  gettedPoint;
	int  woodLoot;
	int  stoneLoot;
	int  oreLoot;
	int  concreteLoot;
	MSGPACK_DEFINE(starNum,gettedPoint,woodLoot,stoneLoot,oreLoot,concreteLoot);
};

struct msgReportBattleResultToDfderResponse{
	unsigned short  starNum;
	int  gettedPoint;
	int  woodLoot;
	int  stoneLoot;
	int  oreLoot;
	int  concreteLoot;
	MSGPACK_DEFINE(starNum,gettedPoint,woodLoot,stoneLoot,oreLoot,concreteLoot);
};

struct msgSetRallypointRequest{
	float  x;
	float  y;
	string  targetID;
	bool  tgtIsBuilding;
	MSGPACK_DEFINE(x,y,targetID,tgtIsBuilding);
};

struct msgSetRallypointResponse{
	float  x;
	float  y;
	string  targetID;
	bool  tgtIsBuilding;
	MSGPACK_DEFINE(x,y,targetID,tgtIsBuilding);
};

struct msgRequestAckResponse{
	int  msgRequestID;
	MSGPACK_DEFINE(msgRequestID);
};

struct msgAtkerStopBattleRequest{
	bool  isStop;
	MSGPACK_DEFINE(isStop);
};

struct msgAtkerStopBattleToDfderResponse{
	bool  isStop;
	MSGPACK_DEFINE(isStop);
};

struct msgAtkerStopBattleToAtkerResponse{
	bool  isStop;
	MSGPACK_DEFINE(isStop);
};

struct SearchGroupRequest{
	string  group_name;
	MSGPACK_DEFINE(group_name);
};

struct SearchGroupResponse{
	int  gid;
	string  group_name;
	int  group_icon;
	int   group_type;
	int  recent_member_num;
	int  max_num;
	int  min_integral;
	int  total_integral;
	MSGPACK_DEFINE(gid,group_name,group_icon,group_type,recent_member_num,max_num,min_integral,total_integral);
};

struct JoinGroupRequest{
	int  gid;
	MSGPACK_DEFINE(gid);
};

struct JoinGroupResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	MSGPACK_DEFINE(process_status,error_number,error_message);
};

struct GetJoinRequest{
	string  anything;
	MSGPACK_DEFINE(anything);
};

struct JoinResquest{
	int  uid;
	string  uname;
	int  gid;
	MSGPACK_DEFINE(uid,uname,gid);
};

struct GetJoinRequestResponse{
	vector<JoinResquest >  request_info;
	MSGPACK_DEFINE(request_info);
};

struct AcceptJoinRequest{
	int  gid;
	int  join_uid;
	MSGPACK_DEFINE(gid,join_uid);
};

struct AcceptJoinResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	MSGPACK_DEFINE(process_status,error_number,error_message);
};

struct ExitGroupRequest{
	int  gid;
	MSGPACK_DEFINE(gid);
};

struct ExitGroupResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	MSGPACK_DEFINE(process_status,error_number,error_message);
};

struct CreateGroupResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	int  group_id;
	MSGPACK_DEFINE(process_status,error_number,error_message,group_id);
};

struct CreateGroupRequest{
	string  group_name;
	int  group_icon;
	int   group_type;
	int  min_integral;
	MSGPACK_DEFINE(group_name,group_icon,group_type,min_integral);
};

struct PublicGroupMessageRequest{
	int  gid;
	string  message;
	MSGPACK_DEFINE(gid,message);
};

struct PublicGroupMessageResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	MSGPACK_DEFINE(process_status,error_number,error_message);
};

struct GetGroupInfoRequest{
	int  gid;
	MSGPACK_DEFINE(gid);
};

struct GroupUserInfo{
	int  race;
	int  uid;
	string  name;
	int  integral;
	int  position;
	MSGPACK_DEFINE(race,uid,name,integral,position);
};

struct GetGroupInfoResponse{
	int  gid;
	string  group_name;
	int  group_icon;
	int   group_type;
	int  recent_member_num;
	int  max_num;
	int  min_integral;
	int  total_integral;
	string  group_message;
	vector<GroupUserInfo >  users_info;
	MSGPACK_DEFINE(gid,group_name,group_icon,group_type,recent_member_num,max_num,min_integral,total_integral,group_message,users_info);
};

struct GroupInfo{
	int  gid;
	string  group_name;
	int  group_icon;
	int   group_type;
	int  recent_member_num;
	int  max_num;
	int  min_integral;
	int  total_integral;
	MSGPACK_DEFINE(gid,group_name,group_icon,group_type,recent_member_num,max_num,min_integral,total_integral);
};

struct GetTop20GroupRequest{
	string  anything;
	MSGPACK_DEFINE(anything);
};

struct GetTop20GroupResponse{
	vector<GroupInfo >  groups_info;
	MSGPACK_DEFINE(groups_info);
};

struct PromotePositionRequest{
	int  promote_uid;
	MSGPACK_DEFINE(promote_uid);
};

struct PromotePositionResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	MSGPACK_DEFINE(process_status,error_number,error_message);
};

struct UpdateGroupRequest{
	string  group_message;
	int  group_icon;
	int   group_type;
	int  min_integral;
	int  group_id;
	MSGPACK_DEFINE(group_message,group_icon,group_type,min_integral,group_id);
};

struct UpdateGroupResponse{
	bool  process_status;
	int  error_number;
	string  error_message;
	MSGPACK_DEFINE(process_status,error_number,error_message);
};

struct SearchUserSimilarRequest{
	string  anything;
	MSGPACK_DEFINE(anything);
};

struct SearchUserSimilarResponse{
	vector<int >  uid_list;
	MSGPACK_DEFINE(uid_list);
};

struct Top50UserInfo{
	int  race;
	int  uid;
	string  name;
	int  integral;
	int  position;
	int  gid;
	string  group_name;
	int  group_icon;
	int  battle_success;
	int  defence_success;
	MSGPACK_DEFINE(race,uid,name,integral,position,gid,group_name,group_icon,battle_success,defence_success);
};

struct GetTop50UserResquest{
	string  anything;
	MSGPACK_DEFINE(anything);
};

struct GetTop50UserResponse{
	vector<Top50UserInfo >  users_info;
	MSGPACK_DEFINE(users_info);
};

#endif//MSGPACK_STRUCT_H_

