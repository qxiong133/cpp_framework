
#for i in `seq 1 10000`
#do
#    echo 'player'$i;
#done

for file in `ls users`
do
    echo users/$file
    python test_user_epoll.py users/$file &
done
