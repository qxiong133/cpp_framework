
online_num=`who | wc -l`
if [ "$online_num" == "0" ] ;then
	echo "work"|sudo -S poweroff
fi

