#!/bin/bash                                                                                                                                                                                                   
set -e
    
## define project parameter
APP_BASE_PATH=/home/work/trac
#VIRTUALENV_PATH=/home/vincent/.virtualenvs/trac12
    
    
    
USER=work
GROUP=work
    
#. $VIRTUALENV_PATH/bin/activate
    
#exec $VIRTUALENV_PATH/bin/tracd --user=$USER --group=$GROUP \
#    --basic-auth="*,$APP_BASE_PATH/.htpasswd,example.com" \
#    -d -p 3050 --pidfile=/var/run/tracd.3050 \

/usr/bin/python /usr/local/bin/tracd -d -p 8080 --basic-auth=*,/etc/subversion/dav_svn.passwd,example.com /home/work/trac


#tracd -d -p 3110 --basic-auth="*,/home/work/trac/.htpasswd,example.com" /home/work/trac
tracd \
    --basic-auth="*,$APP_BASE_PATH/.htpasswd,example.com" \
    -d -p 3050 --pidfile=/var/run/tracd.3050 \
    --protocol=http -s $APP_BASE_PATH 
