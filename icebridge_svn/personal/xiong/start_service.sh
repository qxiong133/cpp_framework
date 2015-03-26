svnserve -d -r /home/svn/project
/usr/bin/python /usr/local/bin/tracd -d -p 8080 --basic-auth=*,/etc/subversion/dav_svn.passwd,example.com /home/work/trac
