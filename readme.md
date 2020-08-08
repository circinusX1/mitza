
# Web Crawler and Indexer for Linux C++, MYSQL


Demo:
https://www.youtube.com/watch?v=9WVLw6nlCak


##########################################
PREQUISITES:


sudo apt-get install libgnutls-openssl27
sudo apt-get install libpcrecpp0
sudo apt-get install libhtmlcxx # or
sudo apt-get install libhtmlcxx3  #or
sudo apt-get install libhtmlcxx*-dev
apt-get install mysql-server mysql-client
apt-get install apache2
apt-get install php5 libapache2-mod-php5
sudo aptitude install libcurl-dev
apt-get install libcurl4-gnutls-dev

/etc/init.d/apache2 restart

DATABASE:

CREATE USER 'muser'@'localhost' IDENTIFIED BY 'mpass';
CREATE USER 'muser'@'127.0.0.1' IDENTIFIED BY 'mpass';
GRANT ALL PRIVILEGES ON *.* TO 'muser'@'localhost'  WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON *.* TO 'muser'@'127.0.0.1'  WITH GRANT OPTION;



#@config.php @ /var/www

define ("DB_HOST","localhost");
define ("DB_USER","muser");
define ("DB_PASSWORD","mpass");
define ("DB_DATABASE","mitza");


#mitza.conf @ mitza/bin
database=mitza     # database name
user=muser         # mysql user
password=mpass     # mysql pass


# usage
./mitza --create      .to create the empty database. If database exists is deleted. Then Ctrl+C to stop
./mitza  --run        .to run.

#sites.txt file format (some versions of curl library will crash on https)

http://address,C,R,D,L,X

C category, refine query on it. numeric value
R reindex 1 or 0, when rerun the mitza on same sites.txt filebuf, default from config
D depth to go in crawling from first page, default fronmc config
L maxim links per site., default 1000 I guess, see the code
X go external of the site 1|0, default 1

The /var-www-mitza/ folder should be copied on your /var/www/mitza/<---here
The tempp folder should be 777



    Status API Training Shop Blog About 

    © 2016 GitHub, Inc. Terms Privacy Security Contact Help 





###  You can check my reverse ssh online service and online key value database at 

[reverse ssh as a service](http://www.mylinuz.com)

[key value database as a service](https://www.meeiot.org)

