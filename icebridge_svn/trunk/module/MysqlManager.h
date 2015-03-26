/* 
 * File:   GameDB.h
 * Author: Edwin Xie
 *
 * Created on 2013年10月16日, 上午10:02
 */

#ifndef GAMEDB_H
#define	GAMEDB_H

#include <string>
#include "Poco/Data/SessionPool.h"
#include <boost/shared_ptr.hpp>


class MysqlManager {
	public:
		MysqlManager();
		~MysqlManager();
		/**
		 * 用户注册
		 * @param email
		 * @param psw
		 * @return 成功返回uid, -1 用户名已存在, 0 失败
		 */
		int regUser(const std::string email, const std::string psw);
		/**
		 * 修改密码, server端保存旧密码, 校验旧密码就不需要再次读取数据库
		 * @param uid 
		 * @param oldPws
		 * @param newPws
		 * @return -3 原始密码错误 1 修改成功 0修改失败
		 */
		int changePsw(const unsigned int uid, const std::string oldPsw, const std::string newPsw);
		/**
		 * 验证用户名密码
		 * @param email
		 * @param psw
		 * @return -2 用户名不存在 0 验证失败, 其他 验证成功, 返回uid
		 */
		int verifyUserPsw(const std::string email, const std::string psw);

        bool subDiamond(const unsigned int uid, int diamond_to_sub);

		typedef boost::shared_ptr<Poco::Data::SessionPool> SessionPoolPointer;
	private:
		SessionPoolPointer _pool;

};

#endif	/* GAMEDB_H */


