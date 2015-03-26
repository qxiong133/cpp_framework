sudo apt-get install subversion
sudo adduser svn
sudo addgroup subversion
sudo addgroup svn subversion    
cd /home/svn
sudo chown -R root:subversion project
sudo chmod -R go+rws project
sudo mkdir /home/svn/project
sudo svnadmin create /home/svn/project
svn co file:///home/svn/fitness
svnserve -d -r /home/svn
#svn import -m "New import" /home/svn/project file:///home/svn/src/project

sudo apt-get install libapache2-svn
