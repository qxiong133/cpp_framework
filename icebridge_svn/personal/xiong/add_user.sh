

sudo htpasswd -b /etc/subversion/dav_svn.passwd $1 $2
echo "$1 = $2"  >> /home/svn/project/conf/passwd

