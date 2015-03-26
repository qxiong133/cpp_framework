
/usr/bin/svnserve -d -r /home/svn/project &
/usr/bin/python /usr/local/bin/tracd -d -p 8080 --basic-auth=*,/etc/subversion/dav_svn.passwd,example.com /home/work/trac &
cd /home/work/tools/redis/dev_redis/bin/ && ./redis-server ../config/redis.conf
cd /home/work/tools/mysql/bin/ && ./mysqld_safe --defaults-file=../etc/my.cnf &
/home/xiong/opt/mysql-5.5.9/bin/mysqld_safe --defaults-file=/home/xiong/opt/mysql-5.5.9/etc/my.cnf &
nohup /home/zhongbing/Desktop/redis-server /home/zhongbing/Desktop/redis.conf  &
#supervisord &
