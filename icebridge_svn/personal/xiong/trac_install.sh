apt-get install trac
apt-get install python-mysqldb

#trac-admin /home/trac/myproject initenv #fill in details here assume project name is myproject
trac-admin /home/work/trac initenv #fill in details here assume project name is myproject
mysql://trac:trac@localhost/trac_db
htdigest -c /home/work/trac/myproject.passwd project.com xiong
trac-admin /home/work/trac/myproject permission add my_username TRAC_ADMIN
chown -R www-data /home/trac/myproject
sudo -u www-data tracd -d -p 3110 --auth=myproject,/home/trac/myproject.passwd,myproject.com /home/trac/myproject

#pstream live_trachosts_com {
#          server  127.0.0.1:3110;
#  }
#  
#  server {
#          listen          80;
#          server_name     myproject.com;
#          location / {
#                rewrite ^/$ /myproject last;
#                  proxy_pass      http://live_trachosts_com;
#          }
#  
#  }
