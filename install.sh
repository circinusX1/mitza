#!/bin/bash

#
# apt test
[[ -z $(which apt) ]] && echo "Install runs using 'apt' but there is noo apt on the system!!" && exit 1

#
#root test
[[ $(whoami) != 'root' ]] && echo "run as sudo!" && exit 1
echo "The install will update with apt-get update. continue(y/n)"
read yn
[[ "$yn" == 'n' ]] && exit
#sudo apt-get update
#
# packages required
packs=("libgnutls-openssl27" "libpcrecpp0" "libhtmlcxx3" "mysql-server" "mysql-client" "apache2" "php5" "libapache2-mod-php5" "php5-mysql")
echo "The install will install '${packs[@]}' (if not already installed). Continue...(y/n)"
read yn
[[ "$yn" == 'n' ]] && exit
for pack in ${packs[@]} ; do
    echo "---------------------------------------------------------------"
    echo "checking: $pack"
    is=$(dpkg -l "$pack")
    if [[ -z $is ]];then
        echo "installing: '$pack' && sleep 1
        apt-get install -y $pack
    else
        echo "package: '$pack' already installed"
    fi
done

#
#
service apache2 restart

#
#






